#include <gtest/gtest.h>

#include "bopm.hpp"

TEST(BinomialOptionsPricingTest, BinomialTreeParams) {
    app::InputParams inputParams = {12.0f,   // Underlying price
                                    100.0f,  // Strike price
                                    0.05f,   // Risk-free rate in decimal
                                    0.03f,   // Dividend yield in decimal
                                    73.0f,   // Time to maturity in days
                                    0.5f,    // Volatility in decimal
                                    100,     // Steps
                                    app::OptionType::call};

    app::BinomialTreeParams binomialTreeParams(inputParams);

    // dt equals daysToExpiration / 365 / steps
    EXPECT_FLOAT_EQ(binomialTreeParams.dt, 0.002f);
    ASSERT_FLOAT_EQ(binomialTreeParams.up, exp(0.5f * sqrt(0.002f)));
    EXPECT_FLOAT_EQ(binomialTreeParams.down, 1 / binomialTreeParams.up);
    EXPECT_FLOAT_EQ(binomialTreeParams.discountRate, exp(0.05f * 0.002f));
    EXPECT_FLOAT_EQ(binomialTreeParams.riskNeutralProb,
                    (exp((0.05f - 0.03f) * 0.002f) - binomialTreeParams.down) /
                        (binomialTreeParams.up - binomialTreeParams.down));
    inputParams.steps = MAXIMUM_BINOMIAL_STEPS + 1;
    EXPECT_THROW(app::binomialAmericanOption(inputParams),
                 std::invalid_argument);
}

TEST(BinomialOptionsPricingTest, calculation) {
    app::InputParams inputParam = {70.0f,  // Underlying price
                                   80.0f,  // Strike price
                                   0.05f,  // Risk-free rate in decimal
                                   0.0f,   // Dividend yield in decimal
                                   10.0f,  // Time to maturity in days
                                   0.8f,   // Volatility in decimal
                                   999,    // Steps
                                   app::OptionType::call};
    app::Results results = app::binomialAmericanOption(inputParam);

    EXPECT_FLOAT_EQ(results.price, 0.82799953f);

    inputParam.isCall = app::OptionType::put;
    results = app::binomialAmericanOption(inputParam);
    EXPECT_FLOAT_EQ(results.price, 10.738019f);
    EXPECT_FLOAT_EQ(results.delta, -0.827194333f);
    EXPECT_FLOAT_EQ(results.gamma, 0.0281867441f);
    EXPECT_FLOAT_EQ(results.theta, -0.111325406f);
}