# syntax=docker/dockerfile:1

ARG BUILDPLATFORM

FROM --platform=$BUILDPLATFORM gcc:12.4-bookworm as coreBuilder

ARG BUILDPLATFORM
ARG CORE_VERSION

WORKDIR /app
COPY core ./core

# Conditional fetch and extraction
RUN VERSION=$(cat /app/core/VERSION) && \
    if [ "${BUILDPLATFORM}" = "linux/amd64" ]; then \
    echo "Fetching and extracting tarball for platform: ${BUILDPLATFORM}" && \
    curl -OL "https://github.com/zhaohan-dong/american-options-pricing/releases/download/v${CORE_VERSION}/release_v${CORE_VERSION}.tar.gz" && \
    tar -xzvf release_v${CORE_VERSION}.tar.gz -C /app; \
    else \
    echo "Skipping tarball fetch for platform: ${BUILDPLATFORM}"; \
    fi

# Conditional build step if tarball is not used
RUN if [ "${BUILDPLATFORM}" != "linux/amd64" ]; then \
    echo "Building for platform: ${BUILDPLATFORM}" && \
    apt-get update && apt-get install -y cmake && \
    mkdir -p /app/build && \
    cd /app && \
    cmake -S core -B build -DCMAKE_BUILD_TYPE=Release -DPROJECT_VERSION=${CORE_VERSION} && \
    cd /app/build && make && \
    mv /app/build/out/BinomialAmericanOption /app/BinomialAmericanOption; \
    fi


FROM --platform=$BUILDPLATFORM node:lts-bookworm-slim as appBuilder

WORKDIR /
COPY app /app

WORKDIR /app
RUN npm install && npm run build


# Final build
FROM --platform=$BUILDPLATFORM node:lts-bookworm-slim as final

ARG IMAGE_VERSION
ARG CORE_VERSION

LABEL author="zhaohan_dong@yahoo.com" \
    version=$IMAGE_VERSION \
    core_version=$CORE_VERSION \
    description="Builds the core binary and downloads required releases"

WORKDIR /app
COPY --from=appBuilder /app/build ./dist
COPY --from=appBuilder /app/package.json ./package.json

RUN npm install --omit=dev

# Copy the compiled binary core service
COPY --from=coreBuilder /app/BinomialAmericanOption dist/bin/BinomialAmericanOption

ENTRYPOINT [ "node", "dist/app.js" ]