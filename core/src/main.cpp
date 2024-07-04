#include <iostream>
#include <unistd.h>
#include "bopm.hpp"

void parseArguments(int argc, char *argv[], OptionParams &params) {
    int opt;
    bool interactiveMode = false;

    while ((opt = getopt(argc, argv, "S:K:r:q:T:s:n:cpi")) != -1) {
        switch (opt) {
            case 'S': params.S = std::stod(optarg); break;
            case 'K': params.K = std::stod(optarg); break;
            case 'r': params.r = std::stod(optarg); break;
            case 'q': params.q = std::stod(optarg); break;
            case 'T': params.days_to_expiration = std::stod(optarg); break;
            case 's': params.sigma = std::stod(optarg); break;
            case 'n': params.steps = std::stoi(optarg); break;
            case 'c': params.isCall = true; break;
            case 'p': params.isCall = false; break;
            case 'i': interactiveMode = true; break;
            default:
                std::cerr << "Usage: " << argv[0]
                          << " [-S stock_price] [-K strike_price] [-r risk_free_rate]"
                          << " [-q dividend_yield] [-T days_to_expiration] [-s volatility]"
                          << " [-n steps] [-c (call option)] [-p (put option)] [-i (interactive mode)]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    if (interactiveMode) {
        // Prompt for interactive input
        std::cout << "Enter stock price (S): ";
        std::cin >> params.S;

        std::cout << "Enter strike price (K): ";
        std::cin >> params.K;

        std::cout << "Enter risk-free rate (r): ";
        std::cin >> params.r;

        std::cout << "Enter dividend yield (q): ";
        std::cin >> params.q;

        std::cout << "Enter days to expiration (T): ";
        std::cin >> params.days_to_expiration;

        std::cout << "Enter volatility (sigma): ";
        std::cin >> params.sigma;

        std::cout << "Enter number of steps (n): ";
        std::cin >> params.steps;

        std::cout << "Enter option type (1 for call, 0 for put): ";
        int optionType;
        std::cin >> optionType;
        params.isCall = (optionType == 1);
    }
}

int main(int argc, char *argv[]) {
    OptionParams params;

    try {
        parseArguments(argc, argv, params);

        long double option_price = app::binomialAmericanOption(params);

        std::cout << option_price << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    exit(EXIT_SUCCESS);
}