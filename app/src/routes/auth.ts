import express from "express";
import {SignJWT} from "jose";
import {authenticateUser} from "../utils/auth";
import cors from "cors";

export const authRouter = express.Router();
const SECRET = new TextEncoder().encode(process.env.JWT_SECRET);

authRouter.use(cors());

authRouter.post("/auth", async (req, res) => {
    const { username, password } = req.body ?? {};

    if (authenticateUser(username, password)) {
        // User is authenticated, generate JWT token
        const accessToken = await new SignJWT({ username })
            .setProtectedHeader({ alg: 'HS256' })
            .setIssuedAt()
            .setExpirationTime('6h')
            .sign(SECRET);

        // Return the token to the client
        res.json({ accessToken });
    } else {
        res.status(401).json({ message: 'Invalid credentials' });
    }
})
