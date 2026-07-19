import path from "path";
import {default as dotenv} from "dotenv";
dotenv.config({path: path.join(__dirname, '../.env.local')});

import cookieParser from 'cookie-parser';
import logger from 'morgan';
import * as http from "node:http";
import {Express} from "express";
import express from 'express';
import {OptionsWebSocketServer} from "./wss-routes/options";
import {authenticateWssRequest} from "./utils/auth";
import {authRouter} from "./routes/auth";

if (!process.env.JWT_SECRET) {
    console.error('No JWT secret found. Set JWT_SECRET environment variable.');
    process.exit(1);
}

// Setup Express as HTTP Route
export class ExpressApp {
    public app: Express;

    constructor() {
        this.app = express();
        this.setupExpressMiddleware();
        this.setupRoutes();
    }

    private setupExpressMiddleware(): void {
        this.app.use(logger('dev'));
        this.app.use(express.json());
        this.app.use(express.urlencoded({ extended: false }));
        this.app.use(cookieParser());
    }

    private setupRoutes(): void {
        this.app.use(authRouter);
    }
}

// Setup http server with express app; websocket upgrades are only completed
// after authentication succeeds (ws runs in noServer mode)
export class HttpServer {
    public server: http.Server;
    public wssServer: OptionsWebSocketServer;

    constructor(expressApp: ExpressApp) {
        this.server = http.createServer(expressApp.app);
        this.wssServer = new OptionsWebSocketServer();
        this.setupUpgradeHandling();
    }

    public listen(): void {
        if (!process.env.PORT) {
            console.log("No port specified, running on default port 8080")
        }
        const port = process.env.PORT || 8080;

        this.server.listen(port, () => {
            console.log(`Server started at port ${port}`);
        });
    }

    private setupUpgradeHandling(): void {
        this.server.on('upgrade', (req, socket, head) => {
            const pathname = new URL(req.url ?? '', 'http://localhost').pathname;
            if (pathname !== this.wssServer.path) {
                socket.write('HTTP/1.1 404 Not Found\r\n\r\n');
                socket.destroy();
                return;
            }

            authenticateWssRequest(req, (err) => {
                if (err) {
                    socket.write('HTTP/1.1 401 Unauthorized\r\n\r\n');
                    socket.destroy();
                    return;
                }
                this.wssServer.wsServer.handleUpgrade(req, socket, head, (ws) => {
                    this.wssServer.wsServer.emit('connection', ws, req);
                });
            });
        });
    }
}

export function start(): HttpServer {
    const httpServer = new HttpServer(new ExpressApp());
    httpServer.listen();
    return httpServer;
}

// Only start listening when run directly, so tests can import without side effects
if (require.main === module) {
    start();
}
