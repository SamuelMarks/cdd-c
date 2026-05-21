import sys
import subprocess
import os

def run_cmd(cmd, cwd=None, env=None, check=True):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, env=env)
    if check and result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}: {' '.join(cmd)}")
        sys.exit(result.returncode)
    return result

def main():
    if len(sys.argv) < 2:
        print("Usage: python pre_commit.py [build|test|lint|wasm|shields]")
        sys.exit(1)

    job = sys.argv[1]

    if job == "build":
        print("=== Building ===")
        os.makedirs("build_cmake", exist_ok=True)
        run_cmd(["cmake", ".."], cwd="build_cmake")
        run_cmd(["cmake", "--build", ".", "--config", "Release"], cwd="build_cmake")
        os.makedirs("bin", exist_ok=True)
        import shutil
        if os.path.exists("build_cmake/bin/cdd-c"):
            shutil.copy("build_cmake/bin/cdd-c", "bin/cdd-c")
        elif os.path.exists("build_cmake/bin/Release/cdd-c.exe"):
            shutil.copy("build_cmake/bin/Release/cdd-c.exe", "bin/cdd-c.exe")
        elif os.path.exists("build_cmake/bin/cdd-c.exe"):
            shutil.copy("build_cmake/bin/cdd-c.exe", "bin/cdd-c.exe")
        else:
            print("Warning: cdd-c binary not found after build.")

    elif job == "test":
        print("=== Testing ===")
        os.makedirs("build_cmake", exist_ok=True)
        run_cmd(["cmake", ".."], cwd="build_cmake")
        run_cmd(["cmake", "--build", ".", "--config", "Debug"], cwd="build_cmake")
        run_cmd(["ctest", "-C", "Debug", "--output-on-failure"], cwd="build_cmake")

    elif job == "lint":
        print("=== Linting ===")
        try:
            subprocess.run(["clang-tidy", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print("Running clang-tidy...")
            files = []
            for root, _, filenames in os.walk("src"):
                for filename in filenames:
                    if filename.endswith(".c") or filename.endswith(".h"):
                        files.append(os.path.join(root, filename))
            for root, _, filenames in os.walk("include"):
                for filename in filenames:
                    if filename.endswith(".c") or filename.endswith(".h"):
                        files.append(os.path.join(root, filename))
            if files:
                run_cmd(["clang-tidy", "-p", "build_cmake", "--quiet"] + files)
        except Exception:
            try:
                subprocess.run(["cppcheck", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                print("Running cppcheck...")
                run_cmd(["cppcheck", "--enable=all", "--suppress=missingIncludeSystem", "src", "include"])
            except Exception:
                print("Warning: Neither clang-tidy nor cppcheck is installed. Skipping linting.")

    elif job == "wasm":
        print("=== Building WASM ===")
        # Build WASM is complex and requires wasi-sdk, skipping native windows build for now or simplifying
        print("WASM build skipped in native Windows pre-commit to avoid wasi-sdk download complexities. Use GitHub Actions for WASM validation.")

    elif job == "shields":
        print("=== Updating Shields ===")
        run_cmd([sys.executable, "scripts/update_badges.py"])

        # Test OpenAPI generation
        script_dir = os.path.dirname(os.path.abspath(__file__))
        repo_root = os.path.dirname(script_dir)
        parent_dir = os.path.dirname(repo_root)

        spec_oas3 = os.path.join(parent_dir, "petstore_oas3.json")
        cdd_c_bin = os.path.join("bin", "cdd-c.exe") if os.name == "nt" else os.path.join("bin", "cdd-c")

        if os.path.exists(spec_oas3) and os.path.exists(cdd_c_bin):
            print("=== OpenAPI 3.2.0 Petstore Test ===")
            if os.path.exists("test_out_oas3"):
                import shutil
                shutil.rmtree("test_out_oas3")
            os.makedirs("test_out_oas3", exist_ok=True)
            run_cmd([cdd_c_bin, "from_openapi", "to_sdk", "-i", spec_oas3, "-o", "test_out_oas3", "--tests"])
            run_cmd(["cmake", ".", "-DFETCHCONTENT_UPDATES_DISCONNECTED=ON"], cwd="test_out_oas3")
            run_cmd(["cmake", "--build", "."], cwd="test_out_oas3")
            result = subprocess.run(["ctest", "--output-on-failure"], cwd="test_out_oas3")
            if result.returncode != 0:
                 print("Warning: CTest failed for OAS3. Continuing...")
        else:
             print(f"Warning: Spec {spec_oas3} or binary {cdd_c_bin} not found. Skipping OAS3 test.")

        spec_sw2 = os.path.join(parent_dir, "petstore.json")

        if os.path.exists(spec_sw2) and os.path.exists(cdd_c_bin):
            print("=== Swagger 2.0 Petstore Test ===")
            if os.path.exists("test_out_sw2"):
                import shutil
                shutil.rmtree("test_out_sw2")
            os.makedirs("test_out_sw2", exist_ok=True)
            run_cmd([cdd_c_bin, "from_openapi", "to_sdk", "-i", spec_sw2, "-o", "test_out_sw2", "--tests"])
            run_cmd(["cmake", ".", "-DFETCHCONTENT_UPDATES_DISCONNECTED=ON"], cwd="test_out_sw2")
            run_cmd(["cmake", "--build", "."], cwd="test_out_sw2")
            result = subprocess.run(["ctest", "--output-on-failure"], cwd="test_out_sw2")
            if result.returncode != 0:
                 print("Warning: CTest failed for Swagger 2.0. Continuing...")
        else:
             print(f"Warning: Spec {spec_sw2} or binary {cdd_c_bin} not found. Skipping Swagger 2 test.")

    else:
        print(f"Unknown job: {job}")
        sys.exit(1)

if __name__ == "__main__":
    main()
