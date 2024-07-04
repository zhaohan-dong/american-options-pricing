#ifndef BINOMIAL_AMERICAN_OPTION_HPP
#define BINOMIAL_AMERICAN_OPTION_HPP

struct OptionParams {
    double S;  // Current stock price
    double K;  // Strike price
    double r;   // Risk-free rate
    double q;   // Dividend yield
    double T;    // Time to maturity in years
    double sigma; // Volatility
    int steps;   // Number of binomial steps
    bool isCall = true; // Option type (true for call, false for put)
};

namespace app {
    double binomialAmericanOption(const OptionParams& params);
}

#endif