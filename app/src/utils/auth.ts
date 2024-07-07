import { Request, Response, NextFunction } from "express";
import jwt, {Secret} from "jsonwebtoken";

const SECRET: Secret = process.env.JWT_SECRET as Secret;

export function authenticateWss(req: Request, res: Response, next: NextFunction) {

    const token = req.headers.authorization;

    if (!token || !token.startsWith("Bearer ")) {
        res.status(401).send('Unauthorized');
        return next(new Error('Unauthorized'));
    }

    const jwtToken = token.split(' ')[1];

    jwt.verify(jwtToken, SECRET!, (err, decoded) => {
        if (err) {
            // res.status(401).send('Unauthorized'); // Invalid token
            return next(new Error('Unauthorized'));
        } else {
            // Token is valid, decoded contains the payload
            next(); // Continue processing
        }
    });
}

export function authenticateUser(username: string, password: string): boolean {
    return username === 'example_user' && password === 'password'
}