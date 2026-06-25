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
        elif os.path.exists("build_cmake/Release/cdd-c.exe"):
            shutil.copy("build_cmake/Release/cdd-c.exe", "bin/cdd-c.exe")
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
            subprocess.run(["cppcheck", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print("Running cppcheck...")
            run_cmd(["cppcheck", "--enable=warning,performance,portability", "--suppress=missingIncludeSystem", "--suppress=unknownMacro", "--suppress=unusedFunction", "src", "include"])
        except Exception:
            print("Warning: cppcheck is not installed. Skipping cppcheck.")

    elif job == "valgrind":
        if sys.platform.startswith("linux"):
            print("=== Valgrind ===")
            os.makedirs("build_cmake", exist_ok=True)
            run_cmd(["cmake", ".."], cwd="build_cmake")
            run_cmd(["cmake", "--build", ".", "--config", "Debug"], cwd="build_cmake")

            cdd_c_bin = os.path.join("build_cmake", "bin", "cdd-c")
            if not os.path.exists(cdd_c_bin):
                print("Warning: cdd-c binary not found for valgrind.")
            else:
                try:
                    subprocess.run(["valgrind", "--version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    print("Running valgrind...")
                    run_cmd(["valgrind", "--leak-check=full", "--error-exitcode=1", cdd_c_bin, "--version"])
                except Exception:
                    print("Warning: valgrind is not installed or failed. Skipping valgrind.")
        else:
            print("=== Valgrind (Skipped on non-Linux) ===")

    elif job == "wasm":
        print("=== Building WASM ===")
        # Build WASM is complex and requires wasi-sdk, skipping native windows build for now or simplifying
        print("WASM build skipped in native Windows pre-commit to avoid wasi-sdk download complexities. Use GitHub Actions for WASM validation.")

    elif job == "shields":
        print("=== Updating Shields ===")
        run_cmd([sys.executable, "scripts/update_badges.py"])

    elif job == "integration":
        import shutil
        has_docker = shutil.which("docker") is not None
        has_java = shutil.which("java") is not None

        jvm_started = False
        if has_java:
            print("=== Trying JVM Petstore Servers ===")
            import urllib.request
            import urllib.error
            import subprocess
            import time

            os.makedirs(".cache", exist_ok=True)
            oas3_jar = ".cache/openapi-petstore.jar"
            sw2_war = ".cache/swagger-petstore.war"
            jetty_jar = ".cache/jetty-runner.jar"

            def download_if_missing(url, dest):
                if not os.path.exists(dest):
                    print(f"Downloading {dest}...")
                    try:
                        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
                        with urllib.request.urlopen(req) as response, open(dest, 'wb') as out_file:
                            shutil.copyfileobj(response, out_file)
                    except Exception as e:
                        print(f"Failed to download {dest}: {e}")
                        return False
                return True

            oas3_ok = download_if_missing("https://jitpack.io/com/github/OpenAPITools/openapi-petstore/master-SNAPSHOT/openapi-petstore-master-SNAPSHOT.jar", oas3_jar)
            sw2_ok = download_if_missing("https://jitpack.io/com/github/swagger-api/swagger-petstore/master-SNAPSHOT/swagger-petstore-master-SNAPSHOT.war", sw2_war)
            jetty_ok = download_if_missing("https://repo1.maven.org/maven2/org/eclipse/jetty/jetty-runner/9.4.53.v20231009/jetty-runner-9.4.53.v20231009.jar", jetty_jar)

            if oas3_ok and sw2_ok and jetty_ok:
                print("Starting JVM petstores in background...")
                sw2_proc = subprocess.Popen(["java", "-jar", jetty_jar, "--port", "8092", sw2_war], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                oas3_proc = subprocess.Popen(["java", "--add-opens", "java.base/java.lang=ALL-UNNAMED", "-jar", oas3_jar, "--server.port=8093"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

                print("Waiting for JVM Petstore containers to start...")
                for _ in range(30):
                    try:
                        urllib.request.urlopen("http://localhost:8092/api/swagger.json")
                        urllib.request.urlopen("http://localhost:8093/api/v3/openapi.json")
                        jvm_started = True
                        break
                    except Exception:
                        time.sleep(2)

                if not jvm_started:
                    print("Warning: JVM Petstore servers failed to start or become ready in time. Falling back...")
                    sw2_proc.kill()
                    oas3_proc.kill()
            else:
                print("Could not download required JVM jars. Falling back...")

        if not jvm_started and has_docker:
            # Docker setup for petstore servers
            print("=== Starting Petstore Docker Containers ===")
            # We use swaggerapi/petstore for both, configuring the base path to match the respective specs
            run_cmd(["docker", "rm", "-f", "petstore_sw2_test", "petstore_oas3_test"], check=False)
            run_cmd(["docker", "run", "-d", "--name", "petstore_sw2_test", "-e", "SWAGGER_HOST=http://localhost:8092", "-e", "SWAGGER_BASE_PATH=/v2", "-p", "8092:8080", "swaggerapi/petstore"], check=False)
            run_cmd(["docker", "run", "-d", "--name", "petstore_oas3_test", "-e", "SWAGGER_HOST=http://localhost:8093", "-e", "SWAGGER_BASE_PATH=/api/v3", "-p", "8093:8080", "swaggerapi/petstore"], check=False)

            import time
            import urllib.request
            import urllib.error

            print("Waiting for Petstore containers to start...")
            for _ in range(30):
                try:
                    urllib.request.urlopen("http://localhost:8092/v2/swagger.json")
                    urllib.request.urlopen("http://localhost:8093/api/v3/openapi.json")
                    break
                except Exception:
                    time.sleep(2)
            else:
                print("Warning: Petstore containers did not become ready in time.")

        # Test OpenAPI generation
        script_dir = os.path.dirname(os.path.abspath(__file__))
        repo_root = os.path.dirname(script_dir)

        spec_oas3 = os.path.join(repo_root, "petstore_oas3.json")
        spec_sw2 = os.path.join(repo_root, "petstore.json")

        if jvm_started:
            import urllib.request
            try:
                print("Fetching Swagger 2.0 spec from JVM server...")
                urllib.request.urlretrieve("http://localhost:8092/api/swagger.json", spec_sw2)
            except Exception as e:
                print(f"Warning: Failed to fetch Swagger 2.0 spec: {e}")

            try:
                print("Fetching OAS3 spec from JVM server...")
                urllib.request.urlretrieve("http://localhost:8093/api/v3/openapi.json", spec_oas3)
            except Exception as e:
                print(f"Warning: Failed to fetch OAS3 spec: {e}")
        elif has_docker:
            import urllib.request
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
            run_cmd(["ctest", "--output-on-failure", "-C", "Debug"], cwd="test_out_oas3", env=env_oas3, check=True)
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
            run_cmd(["ctest", "--output-on-failure", "-C", "Debug"], cwd="test_out_sw2", env=env_sw2, check=True)
        else:
             print(f"Warning: Spec {spec_sw2} or binary {cdd_c_bin} not found. Skipping Swagger 2 test.")

        if jvm_started:
            print("=== Tearing down JVM Petstore Servers ===")
            sw2_proc.kill()
            oas3_proc.kill()

        if not jvm_started and has_docker:
            print("=== Tearing down Petstore Docker Containers ===")
            run_cmd(["docker", "rm", "-f", "petstore_sw2_test", "petstore_oas3_test"], check=False)

    else:
        print(f"Unknown job: {job}")
        sys.exit(1)

if __name__ == "__main__":
    main()
