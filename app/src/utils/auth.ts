import crypto from "node:crypto";
import http from "node:http";
import {jwtVerify} from "jose";

const SECRET = new TextEncoder().encode(process.env.JWT_SECRET);

/* https://stackoverflow.com/questions/4361173/http-headers-in-websockets-client-api Method 1 the query string
 * Didn't use the header method because that messes up the websocket server setup
 */
export function authenticateWssRequest(req: http.IncomingMessage, callback: (err: Error | null) => void): void {
    const accessToken = new URL(req.url ?? '', 'http://localhost').searchParams.get('accessToken');

    if (!accessToken) {
        console.error("Access token is missing");
        return callback(new Error('Unauthorized'));
    }

    jwtVerify(accessToken, SECRET)
        .then(() => callback(null))
        .catch(() => {
            console.error("Access token unauthorized");
            callback(new Error('Unauthorized'));
        });
}

// Hash before comparing so timingSafeEqual gets equal-length buffers and the
// comparison leaks nothing about the expected value's length or content
function timingSafeStringEqual(a: string, b: string): boolean {
    const hashA = crypto.createHash('sha256').update(a).digest();
    const hashB = crypto.createHash('sha256').update(b).digest();
    return crypto.timingSafeEqual(hashA, hashB);
}

export function authenticateUser(username: unknown, password: unknown): boolean {
    const expectedUsername = process.env.AUTH_USERNAME;
    const expectedPassword = process.env.AUTH_PASSWORD;

    if (!expectedUsername || !expectedPassword) {
        console.error('AUTH_USERNAME/AUTH_PASSWORD not configured; rejecting all logins');
        return false;
    }
    if (typeof username !== 'string' || typeof password !== 'string') {
        return false;
    }

    // Evaluate both comparisons so a bad username costs the same as a bad password
    const usernameMatches = timingSafeStringEqual(username, expectedUsername);
    const passwordMatches = timingSafeStringEqual(password, expectedPassword);
    return usernameMatches && passwordMatches;
}
