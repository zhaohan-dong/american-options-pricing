import util from 'util';
import path from "path";
import {WebSocketServer} from "ws";
const exec = util.promisify(require('child_process').exec);

interface OptionsParams {
    price: number,
    strike: number,
    riskFreeRate: number,
    dividendYield: number,
    daysToExpiration: number,
    volatility: number,
    stepsOfBimodalTree: number,
    isCall: boolean
}

export class OptionsService {
    constructor() {}

    public static async calculatePrice(args: OptionsParams): Promise<number|undefined> {
        if (isNaN(args.price) || isNaN(args.strike) || isNaN(args.riskFreeRate) ||
            isNaN(args.dividendYield) || isNaN(args.daysToExpiration) ||
            isNaN(args.volatility) || isNaN(args.stepsOfBimodalTree)) {
            throw new Error('Options Pricing - Invalid input: all arguments besides isCall must be numbers');
        }

        const {stdout, stderr} = await exec(
            `${path.join(__dirname, '../bin/BinomialAmericanOption')} -S ${args.price} -K ${args.strike} -r ${args.riskFreeRate} -q ${args.dividendYield} -T ${args.daysToExpiration} -s ${args.volatility} -n ${args.stepsOfBimodalTree} ${args.isCall ? '-c' : '-p'}`
        );

        if (stderr) {
            console.error('Options Pricing - Error:', stderr);
        } else {
            console.log('Options Pricing - Output:', stdout);
        }

        const outputValue = parseFloat(stdout);
        if (isNaN(outputValue)) {
            throw new Error(`Output is not a valid number: ${stdout}`);
        }
        return outputValue;
    }
}