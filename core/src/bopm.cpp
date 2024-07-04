#include "bopm.hpp"

#include <vector>
#include <cmath>
#include <iostream>

namespace app {
    long double binomialAmericanOption(const OptionParams& params) {
        long double dt = params.days_to_expiration / 252 / params.steps; // Time Step
        long double up = exp(params.sigma * sqrt(dt)); // Upward move
        long double down = 1 / up; // Downward movement
        long double risk_neutral_prob = (exp((params.r - params.q) * dt) - down) / (up - down); // Risk-neutral probability
        long double disc = exp(-params.r * dt); // Discount rate

        std::vector<std::vector<long double>> stockPrices(params.steps + 1, std::vector<long double>(params.steps + 1));
        std::vector<std::vector<long double>> optionValues(params.steps + 1, std::vector<long double>(params.steps + 1));

        // Construct a vector of prices at strike
        stockPrices[0][0] = params.S;
        for (int step = 1; step <= params.steps; ++step) {
            stockPrices[step][0] = stockPrices[step - 1][0] * down; // Lowest bound stock price for each step
            for (int node = 1; node <= step; ++node) {
                stockPrices[step][node] = stockPrices[step-1][node-1] * up;
            }
        }

        for (int node = 0; node <= params.steps; ++node) {
            if (params.isCall) {
                optionValues[params.steps][node] = std::max(stockPrices[params.steps][node] - params.K, 0.0L);
            } else {
                optionValues[params.steps][node] = std::max(params.K - stockPrices[params.steps][node], 0.0L);
            }
        }

        for (int step = params.steps - 1; step >=0 ; --step) {
            for (int node = 0; node <= step; ++node) {
                if (params.isCall) {
                    optionValues[step][node] = std::max(stockPrices[step][node] - params.K,
                                                  disc * (risk_neutral_prob * optionValues[step + 1][node] +
                                                          (1 - risk_neutral_prob) * optionValues[step + 1][node + 1]));
                } else {
                    optionValues[step][node] = std::max(params.K - stockPrices[step][node],
                                                  disc * (risk_neutral_prob * optionValues[step + 1][node] +
                                                          (1 - risk_neutral_prob) * optionValues[step + 1][node + 1]));
                }
            }
        }
        return optionValues[0][0];
    }
}