#ifndef BINOMIAL_AMERICAN_OPTION_HPP
#define BINOMIAL_AMERICAN_OPTION_HPP

struct OptionParams {
    double S;  // Current stock price
    double K;  // Strike price
    double r;   // Risk-free rate in decimal
    double q = 0;   // Dividend yield in decimal
    double days_to_expiration;    // Time to maturity in days
    double sigma; // Volatility in decimal
    int steps;   // Number of binomial steps
    bool isCall = true; // Option type (true for call, false for put)
};

namespace app {
    double binomialAmericanOption(const OptionParams& params);
}

#endif