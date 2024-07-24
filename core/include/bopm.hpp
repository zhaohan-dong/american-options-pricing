#ifndef BINOMIAL_AMERICAN_OPTION_HPP
#define BINOMIAL_AMERICAN_OPTION_HPP

#ifndef MAXIMUM_BINOMIAL_STEPS
#define MAXIMUM_BINOMIAL_STEPS 1000
#endif

#ifndef DAYS_PER_YEAR
#define DAYS_PER_YEAR 365
#endif

namespace app
{
struct OptionParams
{
    float S;                  // Current stock price
    float K;                  // Strike price
    float r;                  // Risk-free rate in decimal
    float q = 0;              // Dividend yield in decimal
    float days_to_expiration; // Time to maturity in days
    float sigma;              // Volatility in decimal
    int steps; // Number of binomial steps (starting with 0, so total steps + 1)
    bool isCall = true; // Option type (true for call, false for put)
};
float binomialAmericanOption(const OptionParams &params);
} // namespace app

#endif
