import {WebSocketServer} from "ws";
import {OptionsParams, OptionsResult, OptionsService} from "../services/options";

interface OptionsWssParams extends OptionsParams {
    requestTime: number,
    // Deprecated alias for `steps`, kept for older clients (original field name, including the "bimodal" typo for "binomial")
    stepsOfBimodalTree?: number
}

export class OptionsWebSocketServer {
    public wsServer: WebSocketServer;
    public readonly path: string = "/options";

    constructor() {
        // noServer: the HTTP server decides which upgrades reach us (see app.ts),
        // so unauthenticated requests never complete a handshake
        this.wsServer = new WebSocketServer({ noServer: true });
        this.setupWebSocket(this.wsServer);
    }

    private setupWebSocket(wss: WebSocketServer): void {
        wss.on('connection', (ws) => {
            console.log('WebSocketServer - Client connected');

            ws.on('message', async (message: string) => {
                console.log('WebSocketServer - Received pricing request from client');

                let body: OptionsWssParams;
                const startTime = performance.now();  // Use performance.now() because the clock is monotonic

                // Parse body, or disconnect
                try {
                    body = JSON.parse(message);
                } catch (error: any) {
                    console.error("WebSocketServer - Error parsing request message:", error);
                    ws.close(1007, `Invalid Payload: ${error.message}`);
                    return;
                }

                try {
                    const result = await this.doWork(body);
                    ws.send(JSON.stringify({ requestTime: body.requestTime, ...result }));
                    console.log('WebSocketServer - Pricing data sent to client\n', "Roundtrip Milliseconds:", performance.now() - startTime);
                } catch (error: any) {
                    console.error("Error getting options pricing:", error);
                    ws.send(JSON.stringify({ requestTime: body.requestTime, error: error.message ?? 'Pricing failed' }));
                }
            });

            ws.on('close', () => {
                console.log('WebSocketServer - Client disconnected');
            });
        });
    }

    private async doWork(body: OptionsWssParams): Promise<OptionsResult> {
        return await OptionsService.calculatePrice({
            price: body.price,
            strike: body.strike,
            riskFreeRate: body.riskFreeRate,
            dividendYield: body.dividendYield,
            daysToExpiration: body.daysToExpiration,
            volatility: body.volatility,
            steps: body.steps ?? body.stepsOfBimodalTree as number,
            isCall: body.isCall
        });
    }
}
