#ifndef BINOMIAL_AMERICAN_OPTION_HPP
#define BINOMIAL_AMERICAN_OPTION_HPP

#ifndef MAXIMUM_BINOMIAL_STEPS
#define MAXIMUM_BINOMIAL_STEPS 1000
#endif

#ifndef DAYS_PER_YEAR
#define DAYS_PER_YEAR 365
#endif

#include <array>

namespace app
{

struct Node
{
    float stockPrice;
    float optionValue;
};

using StockAndOptionPriceStep = std::array<Node, MAXIMUM_BINOMIAL_STEPS>;
using StockAndOptionPriceArray =
    std::array<StockAndOptionPriceStep *, MAXIMUM_BINOMIAL_STEPS>;

enum OptionType
{
    put,
    call
};

struct InputParams
{
    float underlyingPrice;   // Current stock price
    float strike;            // Strike price
    float riskFreeRate;      // Risk-free rate in decimal
    float dividendYield = 0; // Dividend yield in decimal
    float daysToExpiration;  // Time to maturity in days
    float sigma;             // Volatility in decimal
    int steps; // Number of binomial steps (starting with 0, so total steps + 1)
    OptionType isCall =
        OptionType::call; // Option type (true for call, false for put)
};

class BinomialTreeParams
{
  public:
    float dt;
    float up;
    float down;
    float riskNeutralProb;
    float discountRate;
    BinomialTreeParams(const InputParams &params);
};

void calcStockPricesTillTerminalTime(
    const InputParams &params, const BinomialTreeParams &binomialTreeParams,
    StockAndOptionPriceArray &stockAndOptionPricesArray);

float binomialAmericanOption(const InputParams &params);
} // namespace app

#endif
