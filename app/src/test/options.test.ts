import assert from "node:assert/strict";
import {test, before, after} from "node:test";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import {OptionsService, OptionsParams} from "../services/options";

// Stub binary that echoes fixed JSON, so these tests exercise the service's
// validation and parsing without needing the compiled pricing core
const stubResult = {price: 6.0896, delta: -0.4111, gamma: 0.023, theta: -0.0061};
let stubDir: string;

const validParams: OptionsParams = {
    price: 100,
    strike: 100,
    riskFreeRate: 0.05,
    dividendYield: 0,
    daysToExpiration: 365,
    volatility: 0.2,
    steps: 1000,
    isCall: false
};

before(() => {
    stubDir = fs.mkdtempSync(path.join(os.tmpdir(), 'options-test-'));
    const stubPath = path.join(stubDir, 'stub-pricer');
    fs.writeFileSync(stubPath, `#!/bin/sh\necho '${JSON.stringify(stubResult)}'\n`, {mode: 0o755});
    process.env.PRICING_BINARY_PATH = stubPath;
});

after(() => {
    delete process.env.PRICING_BINARY_PATH;
    fs.rmSync(stubDir, {recursive: true, force: true});
});

test('parses the JSON emitted by the pricing binary', async () => {
    const result = await OptionsService.calculatePrice(validParams);
    assert.deepEqual(result, stubResult);
});

test('rejects non-numeric inputs', async () => {
    await assert.rejects(
        OptionsService.calculatePrice({...validParams, price: '100; rm -rf /' as unknown as number}),
        /must be a finite number/
    );
    await assert.rejects(
        OptionsService.calculatePrice({...validParams, volatility: NaN}),
        /must be a finite number/
    );
    await assert.rejects(
        OptionsService.calculatePrice({...validParams, strike: Infinity}),
        /must be a finite number/
    );
});

test('rejects out-of-range or non-integer steps', async () => {
    await assert.rejects(OptionsService.calculatePrice({...validParams, steps: 0}), /steps/);
    await assert.rejects(OptionsService.calculatePrice({...validParams, steps: 1001}), /steps/);
    await assert.rejects(OptionsService.calculatePrice({...validParams, steps: 10.5}), /steps/);
});

test('rejects non-boolean isCall', async () => {
    await assert.rejects(
        OptionsService.calculatePrice({...validParams, isCall: '-c' as unknown as boolean}),
        /isCall/
    );
});

test('rejects a binary response without a valid price', async () => {
    const badStub = path.join(stubDir, 'bad-pricer');
    fs.writeFileSync(badStub, `#!/bin/sh\necho 'not json'\n`, {mode: 0o755});
    process.env.PRICING_BINARY_PATH = badStub;
    try {
        await assert.rejects(OptionsService.calculatePrice(validParams), /not valid JSON/);
    } finally {
        process.env.PRICING_BINARY_PATH = path.join(stubDir, 'stub-pricer');
    }
});
