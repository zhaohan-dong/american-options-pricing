#include <iostream>
#include <unistd.h>
#include "bopm.hpp"

void parseArguments(int argc, char *argv[], OptionParams &params) {
    int opt;
    while ((opt = getopt(argc, argv, "S:K:r:q:T:s:n:c")) != -1) {
        switch (opt) {
            case 'S': params.S = std::stod(optarg); break;
            case 'K': params.K = std::stod(optarg); break;
            case 'r': params.r = std::stod(optarg); break;
            case 'q': params.q = std::stod(optarg); break;
            case 'T': params.T = std::stod(optarg); break;
            case 's': params.sigma = std::stod(optarg); break;
            case 'n': params.steps = std::stoi(optarg); break;
            case 'c': params.isCall = true; break;
            case 'p': params.isCall = false; break;
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-S stock_price] [-K strike_price] [-r risk_free_rate]"
                          << " [-q dividend_yield] [-T maturity] [-s volatility]"
                          << " [-n steps] [-c (call option)] [-p (put option)]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }
}



int main(int argc, char *argv[]) {
    OptionParams params;
    parseArguments(argc, argv, params);

    double option_price = app::binomialAmericanOption(params);

    std::cout << "Hello" << params.sigma << '\n';
    return 0;
}