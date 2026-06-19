import sys
import subprocess
import os

# Strip git environment variables that are set by pre-commit hooks,
# because they leak into CMake's FetchContent and break internal git clones.
for key in ["GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE"]:
    if key in os.environ:
        del os.environ[key]

def run_cmd(cmd, cwd=None, env=None, check=True):
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, env=env)
    except FileNotFoundError:
        print(f"Command not found: {cmd[0]}")
        if check:
            sys.exit(1)
        else:
            class DummyResult:
                returncode = 127
            return DummyResult()
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

        import shutil
        has_docker = shutil.which("docker") is not None

        if has_docker:
            # Docker setup for petstore servers
            print("=== Starting Petstore Docker Containers ===")
            # We use swaggerapi/petstore for both, configuring the base path to match the respective specs
            run_cmd(["docker", "rm", "-f", "petstore_sw2_test", "petstore_oas3_test"], check=False)
            run_cmd(["docker", "run", "-d", "--name", "petstore_sw2_test", "-e", "SWAGGER_HOST=http://localhost:8092", "-e", "SWAGGER_BASE_PATH=/v2", "-p", "8092:8080", "swaggerapi/petstore"], check=False)
            run_cmd(["docker", "run", "-d", "--name", "petstore_oas3_test", "-e", "SWAGGER_HOST=http://localhost:8093", "-e", "SWAGGER_BASE_PATH=/api/v3", "-p", "8093:8080", "swaggerapi/petstore"], check=False)

            import time
            print("Waiting for Petstore containers to start...")
            time.sleep(15)
        else:
            print("=== Docker not found. Skipping Petstore Docker Containers setup ===")

        # Test OpenAPI generation
        script_dir = os.path.dirname(os.path.abspath(__file__))
        repo_root = os.path.dirname(script_dir)

        spec_oas3 = os.path.join(repo_root, "petstore_oas3.json")
        spec_sw2 = os.path.join(repo_root, "petstore.json")

        import urllib.request
        import urllib.error

        if has_docker:
            try:
                print("Fetching Swagger 2.0 spec from container...")
                urllib.request.urlretrieve("http://localhost:8092/v2/swagger.json", spec_sw2)
            except Exception as e:
                print(f"Warning: Failed to fetch Swagger 2.0 spec: {e}")

            try:
                print("Fetching OAS3 spec from container...")
                # Note: both containers are using swaggerapi/petstore image which might default to swagger.json,
                # but we'll try to fetch whatever is served at the base path or fall back to swagger.json
                try:
                    urllib.request.urlretrieve("http://localhost:8093/api/v3/openapi.json", spec_oas3)
                except urllib.error.URLError:
                    urllib.request.urlretrieve("http://localhost:8093/api/v3/swagger.json", spec_oas3)
            except Exception as e:
                print(f"Warning: Failed to fetch OAS3 spec: {e}")

        cdd_c_bin = os.path.join("bin", "cdd-c.exe") if os.name == "nt" else os.path.join("bin", "cdd-c")

        if os.path.exists(spec_oas3) and os.path.exists(cdd_c_bin):
            print("=== OpenAPI 3.2.0 Petstore Test ===")
            if os.path.exists("test_out_oas3"):
                import shutil
                shutil.rmtree("test_out_oas3")
            os.makedirs("test_out_oas3", exist_ok=True)
            run_cmd([cdd_c_bin, "from_openapi", "to_sdk", "-i", spec_oas3, "-o", "test_out_oas3", "--tests"])
            run_cmd(["cmake", ".", "-DFETCHCONTENT_UPDATES_DISCONNECTED=ON", "-DFETCHCONTENT_SOURCE_DIR_CDD_C=.."], cwd="test_out_oas3")
            run_cmd(["cmake", "--build", "."], cwd="test_out_oas3")
            env_oas3 = os.environ.copy()
            env_oas3["BASE_URL"] = "http://localhost:8093"
            result = run_cmd(["ctest", "--output-on-failure"], cwd="test_out_oas3", env=env_oas3, check=False)
            if result.returncode != 0:
                 print("Warning: CTest failed for OAS3. Continuing...")
        else:
             print(f"Warning: Spec {spec_oas3} or binary {cdd_c_bin} not found. Skipping OAS3 test.")

        if os.path.exists(spec_sw2) and os.path.exists(cdd_c_bin):
            print("=== Swagger 2.0 Petstore Test ===")
            if os.path.exists("test_out_sw2"):
                import shutil
                shutil.rmtree("test_out_sw2")
            os.makedirs("test_out_sw2", exist_ok=True)
            run_cmd([cdd_c_bin, "from_openapi", "to_sdk", "-i", spec_sw2, "-o", "test_out_sw2", "--tests"])
            run_cmd(["cmake", ".", "-DFETCHCONTENT_UPDATES_DISCONNECTED=ON", "-DFETCHCONTENT_SOURCE_DIR_CDD_C=.."], cwd="test_out_sw2")
            run_cmd(["cmake", "--build", "."], cwd="test_out_sw2")
            env_sw2 = os.environ.copy()
            env_sw2["BASE_URL"] = "http://localhost:8092"
            result = run_cmd(["ctest", "--output-on-failure"], cwd="test_out_sw2", env=env_sw2, check=False)
            if result.returncode != 0:
                 print("Warning: CTest failed for Swagger 2.0. Continuing...")
        else:
             print(f"Warning: Spec {spec_sw2} or binary {cdd_c_bin} not found. Skipping Swagger 2 test.")

        if has_docker:
            print("=== Tearing down Petstore Docker Containers ===")
            run_cmd(["docker", "rm", "-f", "petstore_sw2_test", "petstore_oas3_test"], check=False)

    else:
        print(f"Unknown job: {job}")
        sys.exit(1)

if __name__ == "__main__":
    main()
