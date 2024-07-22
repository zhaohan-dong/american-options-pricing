import {WebSocketServer} from "ws";
import http from "node:http";
import {OptionsParams, OptionsService} from "../services/options";

interface OptionsWssParams extends OptionsParams {
    "requestTime": number
}

export class OptionsWebSocketServer {
    public wsServer: WebSocketServer;
    private wssServerPath: string = "/options";

    constructor(httpServer: http.Server) {
        // Initialize the server
        this.wsServer = new WebSocketServer({ server: httpServer, path: this.wssServerPath })
        this.setupWebSocket(this.wsServer);
    }

    private setupWebSocket(wss: WebSocketServer): void {
        wss.on('connection', (ws) => {
            console.log('WebSocketServer - Client connected');

            ws.on('message', async (message: string) => {
                console.log('WebSocketServer - Received pricing request from client');

                let body: OptionsWssParams | null = null;
                const startTime = performance.now();

                // Parse body, or disconnect
                try {
                    body = JSON.parse(message);
                } catch (error: any) {
                    console.error("WebSocketServer - Error parsing request message:", error)
                    ws.close(1007, `Invalid Payload: ${error.message}`);
                }

                // Perform work if body is valid
                if (body !== null) {
                    // Let the service calculate option price
                    const optionPrice = await this.doWork(body);

                    // Send data back
                    ws.send(JSON.stringify({ "requestTime": body.requestTime, "price": optionPrice}));
                    console.log('WebSocketServer - Pricing data sent to client\n', "Roundtrip Milliseconds:", performance.now() - startTime);
                    body = null;
                }
            });

            ws.on('close', () => {
                console.log('WebSocketServer - Client disconnected');
            });
        });
    }

    private async doWork(body: OptionsWssParams): Promise<number|undefined> {
        try {
            return await OptionsService.calculatePrice({
                price: body.price,
                strike: body.strike,
                riskFreeRate: body.riskFreeRate,
                dividendYield: body.dividendYield,
                daysToExpiration: body.daysToExpiration,
                volatility: body.volatility,
                stepsOfBimodalTree: body.stepsOfBimodalTree,
                isCall: body.isCall
            });
        } catch (error) {
            console.error("Error getting options pricing:", error);
        }
    }
}