import path from "path";
import {default as dotenv} from "dotenv";
dotenv.config({path: path.join(__dirname, '../.env.local')});

import cookieParser from 'cookie-parser';
import logger from 'morgan';
import * as http from "node:http";
import {Express} from "express";
import express from 'express';
import {OptionsWebSocketServer} from "./wss-routes/options";
import {authenticateWss} from "./utils/auth";
import {authRouter} from "./routes/auth";

if (!process.env.JWT_SECRET) {
    console.error('No JWT secret found. Set JWT_SECRET environment variable.');
    process.exit(1);
}

// Setup Express as HTTP Route
class ExpressApp {
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

export const expressApp = new ExpressApp();

// Setup http server with express app
class HttpServer {
    public server?: http.Server;

    constructor() {
        this.startServer();
    }

    private startServer() {
        if (!process.env.PORT) {
            console.log("No port specified, running on default port 8080")
        }
        const port = process.env.PORT || 8080;

        // Create HTTP server and attach the express app
        this.server = http.createServer(expressApp.app);
        this.server.listen(port, () => {
            console.log(`Server started at port ${port}`);
        });

        this.server.on('upgrade', (req, socket, head) => {
            // Call authentication middleware before upgrading
            authenticateWss(req as express.Request, {} as express.Response, (err?: any) => {
                if (err) {
                    socket.write('HTTP/1.1 401 Unauthorized\n\n');
                    socket.destroy();
                    return;
                }
            });
        });
    }
}

export const httpServer = new HttpServer();
export const wssServer = new OptionsWebSocketServer(httpServer.server!)