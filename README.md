# American Options Pricing

[![Build Binaries with CMake](https://github.com/zhaohan-dong/american-options-pricing/actions/workflows/build-binary.yml/badge.svg?branch=main)](https://github.com/zhaohan-dong/american-options-pricing/actions/workflows/build-binary.yml)

This is a repo for American Options pricing. Usually the trading apps will give a Black-Scholes model output, which is only valid for European Options.

The core module contains C++ code for calculating the options pricing.

The service is written in Express.js, websocket and dockerized for easiness of development and use.

TODO: Add CTest to test the exceptions and calculation.

## Core Pricing Module

To build the core module, install CMake version >= 3.10, and run

```shell
cmake core
```

The output binary is in `./core/build/out/BinomialAmericanOption`

Arguments:

- `-S` Stock Price (underlying)
- `-K` Strike
- `-r` Risk-free rate in decimal (not percentage)
- `-q` Dividend yield in decimal (not percentage)
- `-T` Days to expiration (in days, assuming 365 days per year)
- `-s` Volatility of underlying/IV in decimal (not percentage)
- `-n` Number of steps on the binomial tree
- `-c` is call, otherwise put or `-p`

The maximum steps is set to be 10,000. This can be overriden in `core/include/bopm.hpp`

The calculation uses an array of array pointers. This would be faster than representing each node in a binary tree.

For fewer steps, you can perhaps allocate a contiguous 2D-array on stack.

## WebSocket Server

The server is in the `app` directory. It takes a `JWT_SECRET` env variable for authentication and `PORT` for port.

The server is bootstraped on a express.js server, where the http will upgrade to a websocket connection upon authorization.

> The authorization is pending implementation in `src/utils/auth.ts`

Authorization would initiate with POST /auth, and the username and password in the request body.

It will return a ephemeral token for upgrading connection with the WebSocket server. This can be enhanced by adding a counter to invalidate the token after use.

For more info on securing WebSocket Server, see [this article on StackOverflow](https://stackoverflow.com/questions/4361173/http-headers-in-websockets-client-api).

> Currently the token is set for longer expiration time for debugging and development purposes. Check `src/routes/auth.ts`.

The request body format:

```
{
    requestTime: string,  // For logging purposes in future
    price: number,
    strike: number,
    riskFreeRate: number,
    dividendYield: number,
    daysToExpiration: number,
    volatility: number,
    stepsOfBimondalTree: number,
    isCall: boolean
}

```
