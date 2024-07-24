#include "bopm.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <sstream>

namespace app
{

float binomialAmericanOption(const app::OptionParams &params)
{

    // Throw error if the steps exceeds max binomial steps set
    // Because we use two arrays instead of vectors
    if (params.steps > MAXIMUM_BINOMIAL_STEPS - 1)
    {
        std::stringstream ss;
        ss << "Specified number of binomial steps exceeded maximum set at "
              "compile time: "
           << (MAXIMUM_BINOMIAL_STEPS - 1);
        throw std::runtime_error(ss.str());
    }

    // Initialize the parameters
    float dt =
        params.days_to_expiration / DAYS_PER_YEAR / params.steps; // Time Step
    float up = exp(params.sigma * sqrt(dt));                      // Upward move
    float down = 1 / up; // Downward movement
    float risk_neutral_prob = (exp((params.r - params.q) * dt) - down) /
                              (up - down); // Risk-neutral probability
    float disc = exp(-params.r * dt); // Discount rate to multiply PV with and
                                      // get FV (continuous discounting)

    // Initialize arrays (use array pointer because of limited stack size)
    std::array<std::array<float, MAXIMUM_BINOMIAL_STEPS> *,
               MAXIMUM_BINOMIAL_STEPS>
        stockPrices{};
    std::array<std::array<float, MAXIMUM_BINOMIAL_STEPS> *,
               MAXIMUM_BINOMIAL_STEPS>
        optionValues{};
    for (int i = 0; i <= params.steps; ++i)
    {
        stockPrices[i] = new std::array<float, MAXIMUM_BINOMIAL_STEPS>();
        optionValues[i] = new std::array<float, MAXIMUM_BINOMIAL_STEPS>();
    }

    // Construct an array of prices at strike
    (*stockPrices[0])[0] = params.S;
    for (int step = 1; step <= params.steps; ++step)
    {
        // Lowest bound stock price for each step
        (*stockPrices[step])[0] = (*stockPrices[step - 1])[0] * down;

        for (int node = 1; node <= step; ++node)
        {
            (*stockPrices[step])[node] =
                (*stockPrices[step - 1])[node - 1] * up;
        }
    }

    for (int node = 0; node <= params.steps; ++node)
    {
        (*optionValues[params.steps])[node] =
            params.isCall
                ? std::max((*stockPrices[params.steps])[node] - params.K, 0.0f)
                : std::max(params.K - (*stockPrices[params.steps])[node], 0.0f);
    }

    for (int step = params.steps - 1; step >= 0; --step)
    {
        for (int node = 0; node <= step; ++node)
        {
            float stockPrice = (*stockPrices[step])[node];
            float exerciseValue = params.isCall
                                      ? std::max(stockPrice - params.K, 0.0f)
                                      : std::max(params.K - stockPrice, 0.0f);
            float holdValue =
                disc *
                (risk_neutral_prob * (*optionValues[step + 1])[node + 1] +
                 (1 - risk_neutral_prob) * (*optionValues[step + 1])[node]);
            (*optionValues[step])[node] = std::max(exerciseValue, holdValue);
        }
    }
    for (int i = 0; i <= params.steps; ++i)
    {
        delete stockPrices[i];
    }

    return (*optionValues[0])[0];
}

} // namespace app