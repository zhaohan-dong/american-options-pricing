#include "bopm.hpp"

#include <cmath>
#include <iostream>
#include <sstream>

namespace app
{

BinomialTreeParams::BinomialTreeParams(const InputParams &params)
{
    dt = params.daysToExpiration / DAYS_PER_YEAR / params.steps;
    up = exp(params.sigma * sqrt(dt));
    down = 1 / up;
    riskNeutralProb =
        (exp((params.riskFreeRate - params.dividendYield) * dt) - down) /
        (up - down); // Risk-neutral probability
    discountRate = exp(params.riskFreeRate * dt);
}

void calcStockPricesTillTerminalTime(
    const InputParams &params, const BinomialTreeParams &binomialTreeParams,
    StockAndOptionPriceArray &stockAndOptionPricesArray)
{
    // Construct an array of stock prices from present till terminal nodes
    (*stockAndOptionPricesArray[0])[0].stockPrice = params.underlyingPrice;

    for (int step = 1; step <= params.steps; ++step)
    {
        // Lowest bound stock price for each step
        (*stockAndOptionPricesArray[step])[0].stockPrice =
            (*stockAndOptionPricesArray[step - 1])[0].stockPrice *
            binomialTreeParams.down;

        for (int node = 1; node <= step; ++node)
        {
            (*stockAndOptionPricesArray[step])[node].stockPrice =
                (*stockAndOptionPricesArray[step - 1])[node - 1].stockPrice *
                binomialTreeParams.up;
        }
    }
}

float calcDeltaAtNode(StockAndOptionPriceArray &stockAndOptionPricesArray,
                      int step, int node)
{
    return ((*stockAndOptionPricesArray[step + 1])[node + 1].optionValue -
            (*stockAndOptionPricesArray[step + 1])[node].optionValue) /
           ((*stockAndOptionPricesArray[step + 1])[node + 1].stockPrice -
            (*stockAndOptionPricesArray[step + 1])[node].stockPrice);
}

float calcOptionHoldValue(const float &riskNeutralProb,
                          const float &nextStepUpPrice,
                          const float &nextStepDownPrice,
                          const float &discountRate)
{
    return (riskNeutralProb * nextStepUpPrice +
            (1 - riskNeutralProb) * nextStepDownPrice) /
           discountRate;
}

Results binomialAmericanOption(const app::InputParams &inputParams)
{

    // Throw error if the steps exceeds max binomial steps set
    // Because we use two arrays instead of vectors
    if (inputParams.steps > MAXIMUM_BINOMIAL_STEPS - 1)
    {
        std::stringstream ss;
        ss << "Specified number of binomial steps exceeded maximum set at "
              "compile time: "
           << (MAXIMUM_BINOMIAL_STEPS - 1);
        throw std::runtime_error(ss.str());
    }

    Results results;

    // Initialize the parameters like up/down factors
    BinomialTreeParams binomialTreeParams(inputParams);

    // Initialize arrays (use array pointer because of limited stack
    // size)
    StockAndOptionPriceArray stockAndOptionPricesArray{};
    for (int i = 0; i <= inputParams.steps; ++i)
    {
        stockAndOptionPricesArray[i] = new StockAndOptionPriceStep();
    }

    // Calculate the underlying stock price
    calcStockPricesTillTerminalTime(inputParams, binomialTreeParams,
                                    stockAndOptionPricesArray);

    // Initialize terminal nodes options value
    for (int node = 0; node <= inputParams.steps; ++node)
    {
        (*stockAndOptionPricesArray[inputParams.steps])[node].optionValue =
            inputParams.isCall
                ? std::max((*stockAndOptionPricesArray[inputParams.steps])[node]
                                   .stockPrice -
                               inputParams.strike,
                           0.0f)
                : std::max(
                      inputParams.strike -
                          (*stockAndOptionPricesArray[inputParams.steps])[node]
                              .stockPrice,
                      0.0f);
    }

    // Temp variables to hold option values for comparison on early exercies
    // Can also be stored as part of Node.optionPrice if we want to store these
    // data at each node for future use
    float exerciseValue;
    float holdValue;

    // Repeating because we want to wrap the condition outside of loop
    if (inputParams.isCall == OptionType::call)
    {
        for (int step = inputParams.steps - 1; step >= 0; --step)
        {
            for (int node = 0; node <= step; ++node)
            {
                // Compare if it's better to exercise or hold
                exerciseValue = std::max(
                    (*stockAndOptionPricesArray[step])[node].stockPrice -
                        inputParams.strike,
                    0.0f);

                holdValue = calcOptionHoldValue(
                    binomialTreeParams.riskNeutralProb,
                    (*stockAndOptionPricesArray[step + 1])[node + 1]
                        .optionValue,
                    (*stockAndOptionPricesArray[step + 1])[node].optionValue,
                    binomialTreeParams.discountRate);

                (*stockAndOptionPricesArray[step])[node].optionValue =
                    std::max(exerciseValue, holdValue);
            }
        }
    }
    else
    {
        for (int step = inputParams.steps - 1; step >= 0; --step)
        {
            for (int node = 0; node <= step; ++node)
            {
                // Compare if it's better to exercise or hold
                exerciseValue = std::max(
                    inputParams.strike -
                        (*stockAndOptionPricesArray[step])[node].stockPrice,
                    0.0f);

                holdValue = calcOptionHoldValue(
                    binomialTreeParams.riskNeutralProb,
                    (*stockAndOptionPricesArray[step + 1])[node + 1]
                        .optionValue,
                    (*stockAndOptionPricesArray[step + 1])[node].optionValue,
                    binomialTreeParams.discountRate);

                (*stockAndOptionPricesArray[step])[node].optionValue =
                    std::max(exerciseValue, holdValue);
            }
        }
    }

    results.price = (*stockAndOptionPricesArray[0])[0].optionValue;
    results.delta = calcDeltaAtNode(stockAndOptionPricesArray, 0, 0);
    results.gamma = (calcDeltaAtNode(stockAndOptionPricesArray, 1, 1) -
                     calcDeltaAtNode(stockAndOptionPricesArray, 1, 0)) /
                    ((*stockAndOptionPricesArray[1])[1].stockPrice -
                     (*stockAndOptionPricesArray[1])[0].stockPrice);
    results.theta = ((*stockAndOptionPricesArray[2])[1].optionValue -
                     (*stockAndOptionPricesArray[0])[0].optionValue) /
                    (2 * binomialTreeParams.dt);

    for (int i = 0; i <= inputParams.steps; ++i)
    {
        delete stockAndOptionPricesArray[i];
    }

    return results;
}

} // namespace app