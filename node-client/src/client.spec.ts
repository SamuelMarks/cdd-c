import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import { describe, expect, it } from "vitest";
import { resolveCddBinary, runCddCli } from "./client.js";

describe("cdd-c node-client integration", () => {
  const repoRoot = path.resolve(__dirname, "..", "..");

  it("reports CLI version and help", () => {
    const bin = resolveCddBinary(repoRoot);
    expect(bin).not.toBeNull();
    if (!bin) return;

    const versionOut = runCddCli(bin, ["--version"]);
    expect(versionOut.trim().length).toBeGreaterThan(0);

    const helpOut = runCddCli(bin, ["--help"]);
    expect(helpOut).toContain("to_openapi");
    expect(helpOut).toContain("from_openapi");
  });

  it("generates C SDK from an OpenAPI 3.2.0 specification and round-trips to OpenAPI JSON", () => {
    const bin = resolveCddBinary(repoRoot);
    expect(bin).not.toBeNull();
    if (!bin) return;

    const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), "cdd-c-node-test-"));
    try {
      const specPath = path.join(tmpDir, "openapi.json");
      const sdkOutDir = path.join(tmpDir, "sdk");
      const emittedSpecPath = path.join(tmpDir, "emitted_openapi.json");
      fs.mkdirSync(sdkOutDir, { recursive: true });

      const sampleSpec = {
        openapi: "3.2.0",
        info: {
          title: "Node Client Test API",
          version: "1.0.0",
        },
        paths: {
          "/items": {
            get: {
              operationId: "listItems",
              responses: {
                "200": {
                  description: "OK",
                },
              },
            },
          },
        },
      };
      fs.writeFileSync(specPath, JSON.stringify(sampleSpec, null, 2), "utf-8");

      runCddCli(bin, ["from_openapi", "to_sdk", "-i", specPath, "-o", sdkOutDir]);
      const generatedFiles = fs.readdirSync(sdkOutDir);
      expect(generatedFiles.length).toBeGreaterThan(0);

      runCddCli(bin, ["to_openapi", "-i", sdkOutDir, "-o", emittedSpecPath]);
      expect(fs.existsSync(emittedSpecPath)).toBe(true);

      const roundTripped = JSON.parse(fs.readFileSync(emittedSpecPath, "utf-8"));
      expect(roundTripped.openapi).toBeDefined();
      expect(roundTripped.paths).toBeDefined();
    } finally {
      fs.rmSync(tmpDir, { recursive: true, force: true });
    }
  });
});
