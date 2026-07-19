#include "bopm.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace app
{

namespace
{

double payoff(double stockPrice, double strike, OptionType optionType)
{
    return optionType == OptionType::call
               ? std::max(stockPrice - strike, 0.0)
               : std::max(strike - stockPrice, 0.0);
}

void validate(const InputParams &params)
{
    if (params.steps < 3)
    {
        // Greeks are read off the step-1 and step-2 lattice nodes
        throw std::invalid_argument(
            "Number of binomial steps must be at least 3");
    }
    if (params.steps > MAXIMUM_BINOMIAL_STEPS)
    {
        std::stringstream ss;
        ss << "Specified number of binomial steps exceeded maximum set at "
              "compile time: "
           << MAXIMUM_BINOMIAL_STEPS;
        throw std::invalid_argument(ss.str());
    }
    if (params.underlyingPrice <= 0.0 || params.strike <= 0.0 ||
        params.daysToExpiration <= 0.0 || params.sigma <= 0.0)
    {
        throw std::invalid_argument("Underlying price, strike, days to "
                                    "expiration and sigma must be positive");
    }
}

} // namespace

Results binomialAmericanOption(const InputParams &params)
{
    validate(params);

    const int n = params.steps;
    const double dt = params.daysToExpiration / DAYS_PER_YEAR / n;
    const double up = std::exp(params.sigma * std::sqrt(dt));
    const double down = 1.0 / up;
    const double growth =
        std::exp((params.riskFreeRate - params.dividendYield) * dt);
    const double riskNeutralProb = (growth - down) / (up - down);
    const double discount = std::exp(-params.riskFreeRate * dt);

    if (riskNeutralProb < 0.0 || riskNeutralProb > 1.0)
    {
        throw std::invalid_argument(
            "Risk-neutral probability outside [0, 1]; increase steps or check "
            "rates and volatility");
    }

    // Stock price at (step, node j) is S * up^(2j - step), so a single
    // (n + 1)-length array rolled backwards is all the state we need.
    std::vector<double> optionValues(n + 1);
    for (int j = 0; j <= n; ++j)
    {
        optionValues[j] = payoff(params.underlyingPrice * std::pow(up, 2 * j - n),
                                 params.strike, params.optionType);
    }

    // Option values at steps 1 and 2, captured for the greeks
    double step2Values[3] = {0.0, 0.0, 0.0};
    double step1Values[2] = {0.0, 0.0};

    for (int step = n - 1; step >= 0; --step)
    {
        for (int j = 0; j <= step; ++j)
        {
            const double stockAtNode =
                params.underlyingPrice * std::pow(up, 2 * j - step);
            const double exerciseValue =
                payoff(stockAtNode, params.strike, params.optionType);
            const double holdValue =
                discount * (riskNeutralProb * optionValues[j + 1] +
                            (1.0 - riskNeutralProb) * optionValues[j]);
            optionValues[j] = std::max(exerciseValue, holdValue);
        }

        if (step == 2)
        {
            std::copy(optionValues.begin(), optionValues.begin() + 3,
                      step2Values);
        }
        else if (step == 1)
        {
            std::copy(optionValues.begin(), optionValues.begin() + 2,
                      step1Values);
        }
    }

    const double s1Up = params.underlyingPrice * up;
    const double s1Down = params.underlyingPrice * down;
    const double s2UpUp = s1Up * up;
    const double s2Mid = params.underlyingPrice; // up * down == 1 (CRR)
    const double s2DownDown = s1Down * down;

    const double deltaUp = (step2Values[2] - step2Values[1]) / (s2UpUp - s2Mid);
    const double deltaDown =
        (step2Values[1] - step2Values[0]) / (s2Mid - s2DownDown);

    Results results;
    results.price = optionValues[0];
    results.delta = (step1Values[1] - step1Values[0]) / (s1Up - s1Down);
    results.gamma = (deltaUp - deltaDown) / (0.5 * (s2UpUp - s2DownDown));
    results.theta =
        (step2Values[1] - optionValues[0]) / (2.0 * dt) / DAYS_PER_YEAR;

    return results;
}

} // namespace app
