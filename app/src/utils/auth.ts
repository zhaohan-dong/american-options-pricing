import { Request, Response, NextFunction } from "express";
import jwt, {Secret} from "jsonwebtoken";
import {parse} from "node:url";

const SECRET: Secret = process.env.JWT_SECRET as Secret;

/* https://stackoverflow.com/questions/4361173/http-headers-in-websockets-client-api Method 1 the query string
 * Didn't use the header method because that messes up the websocket server setup
 */
export function authenticateWss(req: Request, res: Response, next: NextFunction) {
    const accessToken = parse(req.url, true).query['accessToken'] as string;

    if (!accessToken) {
        console.error("Access token is missing");
        return next(new Error('Unauthorized'));
    }

    jwt.verify(accessToken, SECRET, (err, decoded) => {
        if (err) {
            console.error("Access token unauthorized");
            return next(new Error('Unauthorized'));
        } else {
            next(); // Continue processing
        }
    });
}

export function authenticateUser(username: string, password: string): boolean {
    return true
}