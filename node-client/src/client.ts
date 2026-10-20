import { execFileSync } from "node:child_process";
import * as fs from "node:fs";
import * as path from "node:path";

/**
 * Resolves the compiled `cdd-c` binary path from standard build directories.
 *
 * @param repoRoot Repository root directory.
 * @returns Absolute path to the `cdd-c` executable, or `null` if not built.
 */
export function resolveCddBinary(repoRoot: string): string | null {
  const candidates = [
    path.join(repoRoot, "build", "cdd-c"),
    path.join(repoRoot, "build", "bin", "cdd-c"),
    path.join(repoRoot, "build", "src", "cdd-c"),
    path.join(repoRoot, "build_clang", "cdd-c"),
    path.join(repoRoot, "build_clang", "bin", "cdd-c"),
    path.join(repoRoot, "build_gcc", "cdd-c"),
    path.join(repoRoot, "build_gcc", "bin", "cdd-c"),
  ];
  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }
  return null;
}

/**
 * Invokes the `cdd-c` CLI binary with the provided arguments.
 *
 * @param binaryPath Path to `cdd-c` executable.
 * @param args CLI arguments array.
 * @returns Standard output from the command.
 */
export function runCddCli(binaryPath: string, args: string[]): string {
  return execFileSync(binaryPath, args, {
    encoding: "utf-8",
    stdio: ["ignore", "pipe", "pipe"],
  });
}
