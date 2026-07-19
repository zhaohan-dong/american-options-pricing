use american_options_pricing::bopm::{BinomialTree, OptionType, PricingParams};
use clap::{Parser, ValueEnum};

#[derive(Copy, Clone, ValueEnum, Debug)]
enum CliOptionType {
    Call,
    Put,
}

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct InputArgs {
    /// Current stock price
    #[arg(short = 'S', long)]
    underlying_price: f64,
    /// Strike price
    #[arg(short = 'K', long)]
    strike: f64,
    /// Risk-free rate in decimal
    #[arg(short = 'r', long)]
    risk_free_rate: f64,
    /// Dividend yield in decimal
    #[arg(short = 'q', long, default_value_t = 0.0)]
    dividend_yield: f64,
    /// Time to maturity in days
    #[arg(short = 'T', long)]
    days_to_expiration: f64,
    /// Volatility in decimal
    #[arg(short = 's', long)]
    sigma: f64,
    /// Number of binomial steps
    #[arg(short = 'n', long)]
    steps: usize,
    /// Option type, "call" or "put"
    #[arg(short = 't', long, value_enum)]
    option_type: Option<CliOptionType>,
    /// Call option (shorthand for -t call)
    #[arg(short = 'c', long, conflicts_with_all = ["put", "option_type"])]
    call: bool,
    /// Put option (shorthand for -t put)
    #[arg(short = 'p', long, conflicts_with_all = ["call", "option_type"])]
    put: bool,
}

impl InputArgs {
    fn resolved_option_type(&self) -> Result<CliOptionType, String> {
        if self.call {
            Ok(CliOptionType::Call)
        } else if self.put {
            Ok(CliOptionType::Put)
        } else {
            self.option_type
                .ok_or_else(|| "specify option type with -t call|put, -c, or -p".to_string())
        }
    }
}

impl InputArgs {
    fn into_params(self, option_type: CliOptionType) -> PricingParams {
        let args = self;
        PricingParams {
            underlying_price: args.underlying_price,
            strike: args.strike,
            risk_free_rate: args.risk_free_rate,
            dividend_yield: args.dividend_yield,
            days_to_expiration: args.days_to_expiration,
            sigma: args.sigma,
            steps: args.steps,
            option_type: match option_type {
                CliOptionType::Call => OptionType::Call,
                CliOptionType::Put => OptionType::Put,
            },
        }
    }
}

fn main() {
    let args = InputArgs::parse();
    let option_type = match args.resolved_option_type() {
        Ok(option_type) => option_type,
        Err(error) => {
            eprintln!("Error: {error}");
            std::process::exit(1);
        }
    };

    match BinomialTree::new(args.into_params(option_type)) {
        Ok(tree) => {
            let result = tree.calculate_option_price();
            // Same JSON shape as the C++ core binary
            println!(
                "{{\"price\": {}, \"delta\": {}, \"gamma\": {}, \"theta\": {}}}",
                result.price, result.delta, result.gamma, result.theta
            );
        }
        Err(error) => {
            eprintln!("Error: {error}");
            std::process::exit(1);
        }
    }
}
