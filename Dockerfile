# syntax=docker/dockerfile:1

# Build the Rust pricing core as a fully static musl binary. Runs natively on
# the build host and cross-compiles to the target arch, so no QEMU-emulated
# compiles and no libc coupling with the runtime image.
FROM --platform=$BUILDPLATFORM rust:1-bookworm AS core-builder

ARG TARGETARCH

WORKDIR /app
COPY rust .

RUN apt-get update \
    && apt-get install -y --no-install-recommends musl-tools gcc-aarch64-linux-gnu gcc-x86-64-linux-gnu \
    && rm -rf /var/lib/apt/lists/*

ENV CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER=x86_64-linux-gnu-gcc \
    CARGO_TARGET_AARCH64_UNKNOWN_LINUX_MUSL_LINKER=aarch64-linux-gnu-gcc

RUN case "$TARGETARCH" in \
    amd64) TRIPLE=x86_64-unknown-linux-musl ;; \
    arm64) TRIPLE=aarch64-unknown-linux-musl ;; \
    *) echo "Unsupported TARGETARCH: $TARGETARCH" && exit 1 ;; \
    esac \
    && rustup target add "$TRIPLE" \
    && cargo build --release --bin bopm-cli --target "$TRIPLE" \
    && cp "target/$TRIPLE/release/bopm-cli" /app/bopm-cli


# Compile the TypeScript app. The output is platform-independent, so run this
# stage natively on the build host for speed.
FROM --platform=$BUILDPLATFORM node:lts-bookworm-slim AS app-builder

WORKDIR /app
COPY app/package.json app/package-lock.json ./
RUN npm ci

COPY app .
RUN npm run build


# Final image
FROM node:lts-bookworm-slim AS final

ARG IMAGE_VERSION
ARG CORE_VERSION

LABEL version=$IMAGE_VERSION \
    core_version=$CORE_VERSION \
    description="American options pricing service (Express app with Rust binomial pricing core)"

WORKDIR /app
COPY --from=app-builder /app/package.json /app/package-lock.json ./
RUN npm ci --omit=dev

COPY --from=app-builder /app/build ./dist
COPY --from=core-builder /app/bopm-cli dist/bin/bopm-cli

ENV PORT=8080 \
    PRICING_BINARY_PATH=/app/dist/bin/bopm-cli
EXPOSE 8080
USER node

ENTRYPOINT [ "node", "dist/app.js" ]
