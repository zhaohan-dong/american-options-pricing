import {WebSocketServer} from "ws";
import http from "node:http";
import {OptionsService} from "../services/options";

export class OptionsWss {
    public wss?: WebSocketServer;

    constructor(httpServer: http.Server) {
        this.wss = new WebSocketServer({ server: httpServer, path: "/options" })
        this.setupWebSocket(this.wss);
    }

    private setupWebSocket(wss: WebSocketServer): void {
        wss.on('connection', (ws) => {
            console.log('WebSocket client connected');

            // Perform work here
            ws.on('message', async (message: string) => {
                console.log('Received message:', message);
                const optionPriceMessage = await this.doWork(message);
                ws.send(`${optionPriceMessage}`);
            });

            ws.on('close', () => {
                console.log('WebSocket client disconnected');
            });
        });
    }

    private async doWork(message: string) {
        const body = JSON.parse(message);
        try {
            const optionPrice = await OptionsService.calculatePrice({
                price: body.price,
                strike: body.strike,
                riskFreeRate: body.riskFreeRate,
                dividendYield: body.dividendYield,
                daysToExpiration: body.daysToExpiration,
                volatility: body.volatility,
                stepsOfBimodalTree: body.stepsOfBimodalTree,
                isCall: body.isCall
            })
            return optionPrice;
        } catch (error) {
            console.error("Error getting options pricing:", error);
        }
    }
}