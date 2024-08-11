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
}