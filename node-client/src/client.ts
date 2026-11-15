import { execFileSync } from "node:child_process";
import { existsSync } from "node:fs";
import { join } from "node:path";

/**
 * Resolves the compiled `cdd-c` binary path from standard build directories.
 *
 * @param repoRoot Repository root directory.
 * @returns Absolute path to the `cdd-c` executable, or `null` if not built.
 */
export function resolveCddBinary(repoRoot: string): string | null {
  const candidates = [
    join(repoRoot, "build", "cdd-c"),
    join(repoRoot, "build", "bin", "cdd-c"),
    join(repoRoot, "build", "src", "cdd-c"),
    join(repoRoot, "build_clang", "cdd-c"),
    join(repoRoot, "build_clang", "bin", "cdd-c"),
    join(repoRoot, "build_gcc", "cdd-c"),
    join(repoRoot, "build_gcc", "bin", "cdd-c"),
  ];
  for (const candidate of candidates) {
    if (existsSync(candidate)) {
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
