#include <iostream>
#include <unistd.h>
#include "bopm.hpp"

void parseArguments(int argc, char *argv[], app::InputParams &params)
{
    int opt;
    bool interactiveMode = false;
    bool has_S = false, has_K = false, has_r = false, has_T = false,
         has_s = false, has_n = false;

    while ((opt = getopt(argc, argv, "S:K:r:q:T:s:n:cpi")) != -1)
    {
        switch (opt)
        {
        case 'S':
            params.underlyingPrice = std::stof(optarg);
            has_S = true;
            break;
        case 'K':
            params.strike = std::stof(optarg);
            has_K = true;
            break;
        case 'r':
            params.riskFreeRate = std::stof(optarg);
            has_r = true;
            break;
        case 'q':
            params.dividendYield = std::stof(optarg);
            break;
        case 'T':
            params.daysToExpiration = std::stof(optarg);
            has_T = true;
            break;
        case 's':
            params.sigma = std::stof(optarg);
            has_s = true;
            break;
        case 'n':
            params.steps = std::stoi(optarg);
            has_n = true;
            break;
        case 'c':
            params.isCall = app::OptionType::call;
            break;
        case 'p':
            params.isCall = app::OptionType::put;
            break;
        case 'i':
            interactiveMode = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0]
                      << " [-i (interactive mode)] [-S stock_price] [-K "
                         "strike_price] [-r risk_free_rate]"
                      << " [-q dividend_yield] [-T days_to_expiration] [-s "
                         "volatility]"
                      << " [-n steps] [-c (call option)] [-p (put option)]\n";
            exit(EXIT_FAILURE);
        }
    }

    // Check if all required arguments are present
    if ((!has_S || !has_K || !has_r || !has_T || !has_s || !has_n) &&
        !interactiveMode)
    {
        std::cerr << "Error: Missing required arguments." << '\n';
        std::cerr
            << "Usage: " << argv[0]
            << " [-S stock_price] [-K strike_price] [-r risk_free_rate]"
            << " [-q dividend_yield] [-T days_to_expiration] [-s volatility]"
            << " [-n steps] [-c (call option)] [-p (put option)]" << '\n';
        std::cerr << "Or use interactive mode with -i\n";
        exit(EXIT_FAILURE);
    }

    if (interactiveMode)
    {
        // Prompt for interactive input
        std::cout << "Enter stock price (S): ";
        std::cin >> params.underlyingPrice;

        std::cout << "Enter strike price (K): ";
        std::cin >> params.strike;

        std::cout << "Enter risk-free rate (r): ";
        std::cin >> params.riskFreeRate;

        std::cout << "Enter dividend yield (q): ";
        std::cin >> params.dividendYield;

        std::cout << "Enter days to expiration (T): ";
        std::cin >> params.daysToExpiration;

        std::cout << "Enter volatility (sigma): ";
        std::cin >> params.sigma;

        std::cout << "Enter number of steps (n): ";
        std::cin >> params.steps;

        std::cout << "Enter option type (1 for call, 0 for put): ";
        int optionType;
        std::cin >> optionType;
        params.isCall = static_cast<app::OptionType>(
            (optionType) == static_cast<int>(app::OptionType::call));
    }
}

int main(int argc, char *argv[])
{
    app::InputParams params;

    try
    {
        parseArguments(argc, argv, params);

        float option_price = app::binomialAmericanOption(params);

        std::cout << option_price << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    exit(EXIT_SUCCESS);
}