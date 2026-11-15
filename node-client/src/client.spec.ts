import {
  existsSync,
  mkdirSync,
  mkdtempSync,
  readdirSync,
  readFileSync,
  rmSync,
  writeFileSync,
} from "node:fs";
import { tmpdir } from "node:os";
import { join, resolve } from "node:path";
import { describe, expect, it } from "vitest";
import { resolveCddBinary, runCddCli } from "./client.js";

/**
 * Narrows a resolved binary path or throws an error if missing.
 *
 * @param bin Candidate binary path or `null`.
 * @returns Non-null binary path.
 */
function requireBinary(bin: string | null): string {
  if (bin === null) {
    throw new Error("cdd-c binary not found");
  }
  return bin;
}

describe("cdd-c node-client integration", () => {
  const repoRoot = resolve(__dirname, "..", "..");

  it("resolves candidate binary paths and returns null when absent", () => {
    const tmpRoot = mkdtempSync(join(tmpdir(), "cdd-c-resolve-"));
    try {
      expect(resolveCddBinary(tmpRoot)).toBeNull();
      expect(() => requireBinary(null)).toThrow("cdd-c binary not found");

      const fallbackDir = join(tmpRoot, "build_gcc", "bin");
      const fallbackBin = join(fallbackDir, "cdd-c");
      mkdirSync(fallbackDir, { recursive: true });
      writeFileSync(fallbackBin, "", "utf-8");

      expect(requireBinary(resolveCddBinary(tmpRoot))).toBe(fallbackBin);
    } finally {
      rmSync(tmpRoot, { recursive: true, force: true });
    }
  });

  it("reports CLI version and help", () => {
    const bin = requireBinary(resolveCddBinary(repoRoot));

    const versionOut = runCddCli(bin, ["--version"]);
    expect(versionOut.trim().length).toBeGreaterThan(0);

    const helpOut = runCddCli(bin, ["--help"]);
    expect(helpOut).toContain("to_openapi");
    expect(helpOut).toContain("from_openapi");
  });

  it("generates C SDK from an OpenAPI 3.2.0 specification and round-trips to OpenAPI JSON", () => {
    const bin = requireBinary(resolveCddBinary(repoRoot));

    const tmpDir = mkdtempSync(join(tmpdir(), "cdd-c-node-test-"));
    try {
      const specPath = join(tmpDir, "openapi.json");
      const sdkOutDir = join(tmpDir, "sdk");
      const emittedSpecPath = join(tmpDir, "emitted_openapi.json");
      mkdirSync(sdkOutDir, { recursive: true });

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
      writeFileSync(specPath, JSON.stringify(sampleSpec, null, 2), "utf-8");

      runCddCli(bin, ["from_openapi", "to_sdk", "-i", specPath, "-o", sdkOutDir]);
      const generatedFiles = readdirSync(sdkOutDir);
      expect(generatedFiles.length).toBeGreaterThan(0);

      runCddCli(bin, ["to_openapi", "-i", sdkOutDir, "-o", emittedSpecPath]);
      expect(existsSync(emittedSpecPath)).toBe(true);

      const roundTripped = JSON.parse(readFileSync(emittedSpecPath, "utf-8"));
      expect(roundTripped.openapi).toBeDefined();
      expect(roundTripped.paths).toBeDefined();
    } finally {
      rmSync(tmpDir, { recursive: true, force: true });
    }
  });
});
