# American Options Pricing

[![Build Binaries with CMake](https://github.com/zhaohan-dong/american-options-pricing/actions/workflows/build-binary.yml/badge.svg?branch=main)](https://github.com/zhaohan-dong/american-options-pricing/actions/workflows/build-binary.yml)

This is a repo for American Options pricing. Usually the trading apps will give a Black-Scholes model output, which is only valid for European Options.

The core module contains C++ code for calculating the options pricing (a Cox-Ross-Rubinstein binomial tree). A Rust port of the same model lives in `rust/` for use as a library (e.g. with the Vercel Rust Runtime) or as a CLI.

The service is written in Express.js, websocket and dockerized for easiness of development and use.

## Core Pricing Module

To build the core module, install CMake version >= 3.10, and run

```shell
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build
```

The output binary is in `./core/build/out/BinomialAmericanOption`. Run the tests with:

```shell
ctest --test-dir core/build --output-on-failure
```

Arguments:

- `-S` Stock Price (underlying)
- `-K` Strike
- `-r` Risk-free rate in decimal (not percentage)
- `-q` Dividend yield in decimal (not percentage)
- `-T` Days to expiration (in days, assuming 365 days per year)
- `-s` Volatility of underlying/IV in decimal (not percentage)
- `-n` Number of steps on the binomial tree (minimum 3, needed for the greeks)
- `-c` is call, otherwise put or `-p`

Output is JSON: `{"price": ..., "delta": ..., "gamma": ..., "theta": ...}` (theta is per calendar day).

The maximum steps is set to be 1,000. This can be overridden with `-DMAXIMUM_BINOMIAL_STEPS` in `core/include/bopm.hpp`.

The calculation uses a single price array rolled backwards through the tree, so memory usage is linear in the number of steps.

## Rust Crate

The `rust/` directory contains the same model as a library crate plus a CLI binary:

```shell
cd rust
cargo test
cargo run --bin bopm-cli -- -S 100 -K 100 -r 0.05 -q 0 -T 365 -s 0.2 -n 1000 -t put
```

The CLI prints the same JSON shape as the C++ binary. Library use:

```rust
use american_options_pricing::bopm::{BinomialTree, OptionType, PricingParams};

let result = BinomialTree::new(PricingParams { /* ... */ })?.calculate_option_price();
```

## WebSocket Server

The server is in the `app` directory. Environment variables:

- `JWT_SECRET` (required) — secret for signing access tokens
- `AUTH_USERNAME` / `AUTH_PASSWORD` (required) — credentials accepted by `POST /auth`; all logins are rejected if unset
- `PORT` — HTTP port, defaults to 8080
- `PRICING_BINARY_PATH` — path to the pricing binary, defaults to `bin/BinomialAmericanOption` next to the compiled app

The server is bootstrapped on an express.js server. Authorization initiates with `POST /auth` with the username and password in the request body; it returns an ephemeral token (6h expiry) used as the `accessToken` query parameter when upgrading to the WebSocket connection at `/options`. Upgrade requests with a missing or invalid token are rejected before the WebSocket handshake completes.

For more info on securing WebSocket Server, see [this article on StackOverflow](https://stackoverflow.com/questions/4361173/http-headers-in-websockets-client-api).

The request body format:

```jsonc
{
    "requestTime": 1721390000000, // echoed back, for client-side latency tracking
    "price": 100,
    "strike": 100,
    "riskFreeRate": 0.05,
    "dividendYield": 0,
    "daysToExpiration": 365,
    "volatility": 0.2,
    "steps": 1000,               // "stepsOfBimodalTree" is accepted as a deprecated alias
    "isCall": false
}
```

The response is either `{"requestTime": ..., "price": ..., "delta": ..., "gamma": ..., "theta": ...}` or `{"requestTime": ..., "error": "..."}`.

For local development, drop the compiled binary into `app/src/bin/` (git-ignored) or set `PRICING_BINARY_PATH`, then:

```shell
cd app
npm install
npm test    # build + unit tests
npm start
```
