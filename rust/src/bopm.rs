use std::error::Error;
use std::fmt;

/// Maximum number of binomial steps accepted, mirroring the C++ core.
pub const MAXIMUM_BINOMIAL_STEPS: usize = 1000;
/// Greeks are read off the step-1 and step-2 lattice nodes.
pub const MINIMUM_BINOMIAL_STEPS: usize = 3;

const DAYS_PER_YEAR: f64 = 365.0;

// Option type
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum OptionType {
    Call,
    Put,
}

/// User-facing pricing inputs.
#[derive(Copy, Clone, Debug)]
pub struct PricingParams {
    /// Current stock price
    pub underlying_price: f64,
    /// Strike price
    pub strike: f64,
    /// Risk-free rate in decimal
    pub risk_free_rate: f64,
    /// Dividend yield in decimal
    pub dividend_yield: f64,
    /// Time to maturity in days
    pub days_to_expiration: f64,
    /// Volatility in decimal
    pub sigma: f64,
    /// Number of binomial steps
    pub steps: usize,
    /// Option type
    pub option_type: OptionType,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PricingError(pub String);

impl fmt::Display for PricingError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}

impl Error for PricingError {}

#[derive(Debug, Copy, Clone)]
pub struct CalculationResult {
    pub price: f64,
    pub delta: f64,
    pub gamma: f64,
    pub theta: f64,
}

pub struct BinomialTree {
    // Time changed per step (delta time)
    dt: f64,
    // Up factor for each step. The price for the next node after is node.price * up.
    up: f64,
    // Risk neutral probability
    risk_neutral_probability: f64,
    // Per-step discount factor e^(-r * dt)
    discount: f64,
    steps: usize,
    underlying_price: f64,
    option_type: OptionType,
    strike: f64,
}

impl BinomialTree {
    pub fn new(params: PricingParams) -> Result<BinomialTree, PricingError> {
        if params.steps < MINIMUM_BINOMIAL_STEPS {
            return Err(PricingError(format!(
                "Number of binomial steps must be at least {MINIMUM_BINOMIAL_STEPS}"
            )));
        }
        if params.steps > MAXIMUM_BINOMIAL_STEPS {
            return Err(PricingError(format!(
                "Number of binomial steps exceeded maximum of {MAXIMUM_BINOMIAL_STEPS}"
            )));
        }
        if params.underlying_price <= 0.0
            || params.strike <= 0.0
            || params.days_to_expiration <= 0.0
            || params.sigma <= 0.0
        {
            return Err(PricingError(
                "Underlying price, strike, days to expiration and sigma must be positive"
                    .to_string(),
            ));
        }

        let dt = params.days_to_expiration / DAYS_PER_YEAR / params.steps as f64;
        let up = (params.sigma * dt.sqrt()).exp();
        let down = 1.0 / up; // Standard CRR down factor

        // Formula: (e^((r-q)dt) - d) / (u - d)
        let growth = ((params.risk_free_rate - params.dividend_yield) * dt).exp();
        let risk_neutral_probability = (growth - down) / (up - down);
        if !(0.0..=1.0).contains(&risk_neutral_probability) {
            return Err(PricingError(
                "Risk-neutral probability outside [0, 1]; increase steps or check rates and volatility"
                    .to_string(),
            ));
        }

        Ok(BinomialTree {
            dt,
            up,
            risk_neutral_probability,
            discount: (-params.risk_free_rate * dt).exp(),
            steps: params.steps,
            underlying_price: params.underlying_price,
            option_type: params.option_type,
            strike: params.strike,
        })
    }

    // Stock price at (step, node j) is S * up^(2j - step)
    fn stock_at_node(&self, step: usize, j: usize) -> f64 {
        self.underlying_price * self.up.powi(2 * j as i32 - step as i32)
    }

    fn payoff(&self, stock_price: f64) -> f64 {
        match self.option_type {
            OptionType::Call => (stock_price - self.strike).max(0.0),
            OptionType::Put => (self.strike - stock_price).max(0.0),
        }
    }

    pub fn calculate_option_price(&self) -> CalculationResult {
        let n = self.steps;
        let down = 1.0 / self.up;

        // 1. Initialize terminal payoff at Step N
        let mut option_values: Vec<f64> = (0..=n)
            .map(|j| self.payoff(self.stock_at_node(n, j)))
            .collect();

        // Variables to "capture" values for Greeks
        let mut step2_values = [0.0; 3];
        let mut step1_values = [0.0; 2];

        // 2. Iterate backwards
        for step in (0..n).rev() {
            for j in 0..=step {
                let exercise_value = self.payoff(self.stock_at_node(step, j));

                // Binomial expectation + discounting
                let hold_value = self.discount
                    * (self.risk_neutral_probability * option_values[j + 1]
                        + (1.0 - self.risk_neutral_probability) * option_values[j]);

                option_values[j] = exercise_value.max(hold_value);
            }

            // Capture data for Greeks as we pass the relevant steps
            if step == 2 {
                step2_values.copy_from_slice(&option_values[0..3]);
            } else if step == 1 {
                step1_values.copy_from_slice(&option_values[0..2]);
            }
        }

        // 3. Stock prices for steps 1 and 2 (Greek denominators)
        let s1_up = self.underlying_price * self.up;
        let s1_down = self.underlying_price * down;

        let s2_up_up = s1_up * self.up;
        let s2_mid = self.underlying_price; // up * down == 1 (CRR)
        let s2_down_down = s1_down * down;

        // 4. Calculate Greeks
        let delta_up = (step2_values[2] - step2_values[1]) / (s2_up_up - s2_mid);
        let delta_down = (step2_values[1] - step2_values[0]) / (s2_mid - s2_down_down);

        let delta = (step1_values[1] - step1_values[0]) / (s1_up - s1_down);
        let gamma = (delta_up - delta_down) / (0.5 * (s2_up_up - s2_down_down));
        let theta = (step2_values[1] - option_values[0]) / (2.0 * self.dt) / DAYS_PER_YEAR;

        CalculationResult {
            price: option_values[0],
            delta,
            gamma,
            theta,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn base_params(option_type: OptionType) -> PricingParams {
        PricingParams {
            underlying_price: 100.0,
            strike: 100.0,
            risk_free_rate: 0.05,
            dividend_yield: 0.0,
            days_to_expiration: 365.0,
            sigma: 0.2,
            steps: 1000,
            option_type,
        }
    }

    #[test]
    fn atm_call_matches_black_scholes() {
        // With no dividends an American call equals the European call;
        // Black-Scholes closed form for these inputs is 10.4506.
        let result = BinomialTree::new(base_params(OptionType::Call))
            .unwrap()
            .calculate_option_price();
        assert!((result.price - 10.4506).abs() < 0.01, "price = {}", result.price);
        assert!(result.delta > 0.5 && result.delta < 0.7);
        assert!(result.gamma > 0.0);
        assert!(result.theta < 0.0);
    }

    #[test]
    fn atm_put_matches_reference_value() {
        // Matches the C++ core and the commonly cited value for this benchmark.
        let result = BinomialTree::new(base_params(OptionType::Put))
            .unwrap()
            .calculate_option_price();
        assert!((result.price - 6.0896).abs() < 0.01, "price = {}", result.price);
        assert!(result.delta < 0.0 && result.delta > -1.0);
        assert!(result.gamma > 0.0);
    }

    #[test]
    fn deep_itm_put_worth_at_least_intrinsic() {
        let mut params = base_params(OptionType::Put);
        params.underlying_price = 60.0;
        let result = BinomialTree::new(params).unwrap().calculate_option_price();
        assert!(result.price >= 40.0);
    }

    #[test]
    fn small_trees_still_produce_greeks() {
        let mut params = base_params(OptionType::Put);
        params.steps = MINIMUM_BINOMIAL_STEPS;
        let result = BinomialTree::new(params).unwrap().calculate_option_price();
        assert!(result.price > 0.0);
        assert!(result.delta.is_finite() && result.delta != 0.0);
        assert!(result.gamma.is_finite());
        assert!(result.theta.is_finite());
    }

    #[test]
    fn invalid_steps_rejected() {
        let mut params = base_params(OptionType::Call);
        params.steps = 0;
        assert!(BinomialTree::new(params).is_err());
        params.steps = 2;
        assert!(BinomialTree::new(params).is_err());
        params.steps = MAXIMUM_BINOMIAL_STEPS + 1;
        assert!(BinomialTree::new(params).is_err());
    }

    #[test]
    fn non_positive_inputs_rejected() {
        let mut params = base_params(OptionType::Call);
        params.sigma = 0.0;
        assert!(BinomialTree::new(params).is_err());

        let mut params = base_params(OptionType::Call);
        params.days_to_expiration = -1.0;
        assert!(BinomialTree::new(params).is_err());
    }
}
