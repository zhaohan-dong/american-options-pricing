// Basic correctness tests for the binomial American option pricer.
// Run via CTest: cmake -S core -B build && cmake --build build && ctest --test-dir build

#include "bopm.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace
{

int failures = 0;

void check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
    else
    {
        std::cout << "ok: " << message << '\n';
    }
}

double normCdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

// Black-Scholes European price; with q = 0 an American call is worth the same,
// which gives us an independent closed-form cross-check of the tree.
double blackScholes(bool isCall, double s, double k, double r, double q,
                    double sigma, double years)
{
    const double d1 = (std::log(s / k) + (r - q + 0.5 * sigma * sigma) * years) /
                      (sigma * std::sqrt(years));
    const double d2 = d1 - sigma * std::sqrt(years);
    if (isCall)
    {
        return s * std::exp(-q * years) * normCdf(d1) -
               k * std::exp(-r * years) * normCdf(d2);
    }
    return k * std::exp(-r * years) * normCdf(-d2) -
           s * std::exp(-q * years) * normCdf(-d1);
}

app::InputParams baseParams(app::OptionType type)
{
    app::InputParams params;
    params.underlyingPrice = 100.0;
    params.strike = 100.0;
    params.riskFreeRate = 0.05;
    params.dividendYield = 0.0;
    params.daysToExpiration = 365.0;
    params.sigma = 0.2;
    params.steps = 1000;
    params.optionType = type;
    return params;
}

} // namespace

int main()
{
    // American call without dividends equals the Black-Scholes European call
    const app::Results call =
        app::binomialAmericanOption(baseParams(app::OptionType::call));
    const double bsCall = blackScholes(true, 100.0, 100.0, 0.05, 0.0, 0.2, 1.0);
    check(std::fabs(call.price - bsCall) < 0.01,
          "ATM call matches Black-Scholes closed form");
    check(call.delta > 0.5 && call.delta < 0.7, "call delta in expected range");
    check(call.gamma > 0.0, "call gamma positive");
    check(call.theta < 0.0, "call theta negative");

    // American put carries an early-exercise premium over the European put
    const app::Results put =
        app::binomialAmericanOption(baseParams(app::OptionType::put));
    const double bsPut = blackScholes(false, 100.0, 100.0, 0.05, 0.0, 0.2, 1.0);
    check(put.price > bsPut, "American put above European lower bound");
    check(put.price < bsPut + 1.0, "American put premium is sensible");
    check(std::fabs(put.price - 6.09) < 0.01,
          "ATM put matches reference value 6.09");
    check(put.delta < 0.0 && put.delta > -1.0, "put delta in (-1, 0)");
    check(put.gamma > 0.0, "put gamma positive");

    // Deep ITM American put should be worth at least intrinsic value
    app::InputParams itmPut = baseParams(app::OptionType::put);
    itmPut.underlyingPrice = 60.0;
    const app::Results itm = app::binomialAmericanOption(itmPut);
    check(itm.price >= 40.0, "deep ITM put worth at least intrinsic value");

    // Invalid inputs throw
    bool threw = false;
    app::InputParams bad = baseParams(app::OptionType::call);
    bad.steps = 0;
    try
    {
        app::binomialAmericanOption(bad);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }
    check(threw, "steps below minimum throws");

    threw = false;
    bad = baseParams(app::OptionType::call);
    bad.steps = MAXIMUM_BINOMIAL_STEPS + 1;
    try
    {
        app::binomialAmericanOption(bad);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }
    check(threw, "steps above maximum throws");

    if (failures > 0)
    {
        std::cerr << failures << " test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed\n";
    return EXIT_SUCCESS;
}
