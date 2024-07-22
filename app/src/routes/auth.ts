import express from "express";
import jwt, {Secret} from "jsonwebtoken";
import {authenticateUser} from "../utils/auth";
import cors from "cors";

export const authRouter = express.Router();
const SECRET = process.env.JWT_SECRET as Secret;

authRouter.use(cors());

authRouter.post("/auth", async (req, res) => {
    const { username, password } = req.body;

    if (authenticateUser(username, password)) {
        // User is authenticated, generate JWT token
        const accessToken = jwt.sign({ username }, SECRET, { expiresIn: '6h' });

        // Return the token to the client
        res.json({ accessToken });
    } else {
        res.status(401).json({ message: 'Invalid credentials' });
    }
})
