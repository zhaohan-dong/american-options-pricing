#include "bopm.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <format>

namespace app {
    double binomialAmericanOption(const app::OptionParams& params) {

        // Throw error if the steps exceeds max binomial steps set
        // Because we use two arrays instead of vectors        
        if (params.steps > MAXIMUM_BINOMIAL_STEPS - 1) {
            throw std::runtime_error(std::format("Specified number of binomial steps exceeded maximum set at compile time: {}", MAXIMUM_BINOMIAL_STEPS - 1));
        }

        // Initialize the parameters
        double dt = params.days_to_expiration / 252 / params.steps; // Time Step
        double up = exp(params.sigma * sqrt(dt)); // Upward move
        double down = 1 / up; // Downward movement
        double risk_neutral_prob = (exp((params.r - params.q) * dt) - down) / (up - down); // Risk-neutral probability
        double disc = exp(-params.r * dt); // Discount rate

        // Initialize arrays (use array pointer because of limited stack size)
        std::array<std::array<double, MAXIMUM_BINOMIAL_STEPS>*, MAXIMUM_BINOMIAL_STEPS> stockPrices{};
        std::array<std::array<double, MAXIMUM_BINOMIAL_STEPS>*, MAXIMUM_BINOMIAL_STEPS> optionValues{};
        for (int i = 0; i <= params.steps; ++i) {
            stockPrices[i] = new std::array<double, MAXIMUM_BINOMIAL_STEPS>();
            optionValues[i] = new std::array<double, MAXIMUM_BINOMIAL_STEPS>();
        }

        // Construct an array of prices at strike
        (*stockPrices[0])[0] = params.S;
        for (int step = 1; step <= params.steps; ++step) {
            (*stockPrices[step])[0] = (*stockPrices[step - 1])[0] * down; // Lowest bound stock price for each step
            for (int node = 1; node <= step; ++node) {
                (*stockPrices[step])[node] = (*stockPrices[step-1])[node-1] * up;
            }
        }

        for (int node = 0; node <= params.steps; ++node) {
            if (params.isCall) {
                (*optionValues[params.steps])[node] = std::max((*stockPrices[params.steps])[node] - params.K, 0.0);
            } else {
                (*optionValues[params.steps])[node] = std::max(params.K - (*stockPrices[params.steps])[node], 0.0);
            }
        }

        for (int step = params.steps - 1; step >=0 ; --step) {
            for (int node = 0; node <= step; ++node) {
                if (params.isCall) {
                    (*optionValues[step])[node] = std::max((*stockPrices[step])[node] - params.K,
                                                  disc * (risk_neutral_prob * (*optionValues[step + 1])[node] +
                                                          (1 - risk_neutral_prob) * (*optionValues[step + 1])[node + 1]));
                } else {
                    (*optionValues[step])[node] = std::max(params.K - (*stockPrices[step])[node],
                                                  disc * (risk_neutral_prob * (*optionValues[step + 1])[node] +
                                                          (1 - risk_neutral_prob) * (*optionValues[step + 1])[node + 1]));
                }
            }
        }

        for (int i = 0; i <= params.steps; ++i) {
            delete stockPrices[i];
        }

        return (*optionValues[0])[0];
    }
}