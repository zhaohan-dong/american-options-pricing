import express from "express";
import jwt, {Secret} from "jsonwebtoken";
import {authenticateUser} from "../utils/auth";

export const authRouter = express.Router();
const SECRET = process.env.JWT_SECRET as Secret;

authRouter.post("/", async (req, res) => {
    const { username, password } = req.body;

    if (authenticateUser(username, password)) {
        // User is authenticated, generate JWT token
        const token = jwt.sign({ username }, SECRET, { expiresIn: '1h' });

        // Return the token to the client
        res.json({ token });
    } else {
        res.status(401).json({ message: 'Invalid credentials' });
    }
})
