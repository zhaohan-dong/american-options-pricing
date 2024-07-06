import cookieParser from 'cookie-parser';
import logger from 'morgan';
import * as http from "node:http";
import {Express} from "express";
import express from 'express';
import {OptionsWss} from "./wss-routes/options";
import {authenticate} from "./utils/auth";

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
        // this.app.use('/', indexRouter);
    }
}

export const expressApp = new ExpressApp();

class HttpServer {
    public server?: http.Server;

    constructor() {
        this.startServer();
    }

    private startServer() {
        const port = process.env.PORT || 3000;

        // Create HTTP server and attach the express app
        this.server = http.createServer(expressApp.app);
        this.server.listen(port, () => {
            console.log(`Server started at http://localhost:${port}`);
        });

        this.server.on('upgrade', (req, socket, head) => {
            // Call your authentication middleware before upgrading
            authenticate(req as express.Request, {} as express.Response, (err?: any) => {
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
export const wssServer = new OptionsWss(httpServer.server!)