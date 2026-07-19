import util from 'util';
import path from "path";
import {execFile} from "child_process";

const execFileAsync = util.promisify(execFile);

// Bounds mirror the pricing core (steps >= 3 needed for greeks, max set at compile time)
const MIN_STEPS = 3;
const MAX_STEPS = 1000;

export interface OptionsParams {
    price: number,
    strike: number,
    riskFreeRate: number,
    dividendYield: number,
    daysToExpiration: number,
    volatility: number,
    steps: number,
    isCall: boolean
}

export interface OptionsResult {
    price: number,
    delta: number,
    gamma: number,
    theta: number
}

function assertFiniteNumber(value: unknown, name: string): asserts value is number {
    if (typeof value !== 'number' || !Number.isFinite(value)) {
        throw new Error(`Options Pricing - Invalid input: ${name} must be a finite number`);
    }
}

export class OptionsService {
    constructor() {}

    private static binaryPath(): string {
        return process.env.PRICING_BINARY_PATH || path.join(__dirname, '../bin/BinomialAmericanOption');
    }

    public static async calculatePrice(args: OptionsParams): Promise<OptionsResult> {
        assertFiniteNumber(args.price, 'price');
        assertFiniteNumber(args.strike, 'strike');
        assertFiniteNumber(args.riskFreeRate, 'riskFreeRate');
        assertFiniteNumber(args.dividendYield, 'dividendYield');
        assertFiniteNumber(args.daysToExpiration, 'daysToExpiration');
        assertFiniteNumber(args.volatility, 'volatility');
        if (!Number.isInteger(args.steps) || args.steps < MIN_STEPS || args.steps > MAX_STEPS) {
            throw new Error(`Options Pricing - Invalid input: steps must be an integer between ${MIN_STEPS} and ${MAX_STEPS}`);
        }
        if (typeof args.isCall !== 'boolean') {
            throw new Error('Options Pricing - Invalid input: isCall must be a boolean');
        }

        const optionsCalculationStartTime = performance.now();

        // Execute the compiled binary; execFile with an argument array so no
        // user-supplied value ever reaches a shell
        const {stdout} = await execFileAsync(this.binaryPath(), [
            '-S', String(args.price),
            '-K', String(args.strike),
            '-r', String(args.riskFreeRate),
            '-q', String(args.dividendYield),
            '-T', String(args.daysToExpiration),
            '-s', String(args.volatility),
            '-n', String(args.steps),
            args.isCall ? '-c' : '-p'
        ]);

        console.log('Options Pricing - Calculation Milliseconds:', performance.now() - optionsCalculationStartTime);

        let result: OptionsResult;
        try {
            result = JSON.parse(stdout);
        } catch {
            throw new Error(`Options Pricing - Output is not valid JSON: ${stdout}`);
        }
        if (typeof result.price !== 'number' || !Number.isFinite(result.price)) {
            throw new Error(`Options Pricing - Output has no valid price: ${stdout}`);
        }
        return result;
    }
}
