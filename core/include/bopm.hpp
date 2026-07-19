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

enum class OptionType
{
    put,
    call
};

struct InputParams
{
    double underlyingPrice;  // Current stock price
    double strike;           // Strike price
    double riskFreeRate;     // Risk-free rate in decimal
    double dividendYield;    // Dividend yield in decimal
    double daysToExpiration; // Time to maturity in days
    double sigma;            // Volatility in decimal
    int steps;               // Number of binomial steps
    OptionType optionType;   // Call or put
};

struct Results
{
    double price;
    double delta;
    double gamma;
    double theta;
};

Results binomialAmericanOption(const InputParams &params);

} // namespace app

#endif
