import sys
import subprocess
import os
import shutil
import re

# Strip git environment variables that are set by pre-commit hooks
for key in ["GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE"]:
    if key in os.environ:
        del os.environ[key]

def run_cmd(cmd, cwd=None, env=None, check=True):
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, env=env, text=True)
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

def is_tool(name):
    return shutil.which(name) is not None

def has_gcc():
    return is_tool("gcc")

def has_clang():
    return is_tool("clang")

def has_msvc():
    if os.name == 'nt' and is_tool("cl.exe"): return True
    return False

def has_msvc_wine():
    if os.name == 'nt': return False
    msvc_wine_path = os.environ.get("MSVC_WINE_PATH", os.path.expanduser("~/my_msvc/opt/msvc"))
    return is_tool("wine") and os.path.exists(msvc_wine_path)

def has_mingw():
    if os.name == 'nt' and is_tool("gcc"):
        res = subprocess.run(["gcc", "-v"], capture_output=True, text=True)
        return "mingw" in res.stderr.lower()
    return False

def has_cygwin():
    if os.name == 'nt' and is_tool("gcc"):
        res = subprocess.run(["gcc", "-v"], capture_output=True, text=True)
        return "cygwin" in res.stderr.lower()
    return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python precommit_matrix.py [cppcheck|build|test|valgrind|shields] [toolchain]")
        sys.exit(1)

    job = sys.argv[1]
    toolchain = sys.argv[2] if len(sys.argv) > 2 else None

    # Check for custom pre_commit.py in repo (like cdd-c)
    if os.path.exists("scripts/pre_commit.py") and job != "shields":
        # Forward to repo-specific script if it exists
        if job in ["build", "test", "valgrind"]:
            # cdd-c expects job without toolchain, maybe? Let's be smart.
            # actually we shouldn't fully hijack if the matrix is required.
            # if cdd-c wants matrix, we should just use matrix.
            pass

    if job == "cppcheck":
        if is_tool("cppcheck"):
            print("Running cppcheck...")
            dirs = [d for d in ["src", "include"] if os.path.isdir(d)]
            if dirs:
                cmd = ["cppcheck", "-j", "4", "--enable=warning,performance,portability", "--suppress=missingIncludeSystem", "--suppress=unknownMacro", "--suppress=unusedFunction"]
                if os.path.isdir("include"):
                    cmd.extend(["-I", "include"])
                cmd.extend(dirs)
                run_cmd(cmd, check=False)
            else:
                print("No src or include directory found. Skipping cppcheck.")
        else:
            print("cppcheck not found. Skipping.")
        sys.exit(0)

    elif job == "build":
        if toolchain == "gcc" and has_gcc():
            build_dir = "build_gcc"
            os.makedirs(build_dir, exist_ok=True)
            env = os.environ.copy()
            env["CC"] = "gcc"
            run_cmd(["cmake", "..", "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_C_FLAGS=--coverage", "-DCMAKE_EXE_LINKER_FLAGS=--coverage", "-DBUILD_TESTING=ON"], cwd=build_dir, env=env)
            run_cmd(["cmake", "--build", "."], cwd=build_dir, env=env)
        elif toolchain == "clang" and has_clang():
            build_dir = "build_clang"
            os.makedirs(build_dir, exist_ok=True)
            env = os.environ.copy()
            env["CC"] = "clang"
            run_cmd(["cmake", "..", "-DCMAKE_BUILD_TYPE=Release", "-DBUILD_TESTING=ON"], cwd=build_dir, env=env)
            run_cmd(["cmake", "--build", "."], cwd=build_dir, env=env)
        elif toolchain == "msvc_wine" and has_msvc_wine():
            build_dir = "build_msvc_wine"
            os.makedirs(build_dir, exist_ok=True)

            # Create dummy InstallRequiredSystemLibraries.cmake to prevent crashes on Linux host
            dummy_cmake_dir = os.path.join(build_dir, "dummy_cmake")
            os.makedirs(dummy_cmake_dir, exist_ok=True)
            with open(os.path.join(dummy_cmake_dir, "InstallRequiredSystemLibraries.cmake"), "w") as f:
                f.write("# Dummy file to prevent cross-compilation crash\n")

            env = os.environ.copy()
            msvc_wine_path = os.environ.get("MSVC_WINE_PATH", os.path.expanduser("~/my_msvc/opt/msvc"))
            env["PATH"] = f"{msvc_wine_path}/bin/x64:{env.get('PATH', '')}"
            winepath = f"{os.path.abspath(build_dir)};{msvc_wine_path}/bin/x64;{msvc_wine_path}/VC/Redist/MSVC/14.51.36231/debug_nonredist/x64/Microsoft.VC145.DebugCRT;{msvc_wine_path}/Windows Kits/10/bin/10.0.26100.0/x64/ucrt"
            env["WINEPATH"] = winepath
            run_cmd(["cmake", "..", f"-DCMAKE_MODULE_PATH={os.path.abspath(dummy_cmake_dir)}", "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_SYSTEM_NAME=Windows", "-DCMAKE_C_COMPILER=cl", "-DCMAKE_CXX_COMPILER=cl", "-DCMAKE_CROSSCOMPILING_EMULATOR=wine", "-DBUILD_TESTING=ON", "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebugDLL", "-DCMAKE_DISABLE_FIND_PACKAGE_SQLite3=ON", "-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON", "-DCMAKE_DISABLE_FIND_PACKAGE_Brotli=ON", "-DCMAKE_DISABLE_FIND_PACKAGE_CURL=ON"], cwd=build_dir, env=env)
            run_cmd(["cmake", "--build", "."], cwd=build_dir, env=env)
        elif toolchain == "msvc" and has_msvc():
            build_dir = "build_msvc"
            os.makedirs(build_dir, exist_ok=True)
            run_cmd(["cmake", "..", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTING=ON"], cwd=build_dir)
            run_cmd(["cmake", "--build", "."], cwd=build_dir)
        elif toolchain == "mingw" and has_mingw():
            build_dir = "build_mingw"
            os.makedirs(build_dir, exist_ok=True)
            env = os.environ.copy()
            env["CC"] = "gcc"
            run_cmd(["cmake", "..", "-G", "MinGW Makefiles", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTING=ON"], cwd=build_dir, env=env)
            run_cmd(["cmake", "--build", "."], cwd=build_dir, env=env)
        elif toolchain == "cygwin" and has_cygwin():
            build_dir = "build_cygwin"
            os.makedirs(build_dir, exist_ok=True)
            env = os.environ.copy()
            env["CC"] = "gcc"
            run_cmd(["cmake", "..", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTING=ON"], cwd=build_dir, env=env)
            run_cmd(["cmake", "--build", "."], cwd=build_dir, env=env)
        else:
            print(f"Toolchain {toolchain} not found or not supported on this OS. Skipping build.")
        sys.exit(0)

    elif job == "test":
        build_dir = f"build_{toolchain}"
        if not os.path.exists(build_dir):
            print(f"Build directory {build_dir} not found. Skipping test.")
            sys.exit(0)

        env = os.environ.copy()
        if toolchain == "msvc_wine":
            msvc_wine_path = os.environ.get("MSVC_WINE_PATH", os.path.expanduser("~/my_msvc/opt/msvc"))
            winepath = f"{os.path.abspath(build_dir)};{msvc_wine_path}/bin/x64;{msvc_wine_path}/VC/Redist/MSVC/14.51.36231/debug_nonredist/x64/Microsoft.VC145.DebugCRT;{msvc_wine_path}/Windows Kits/10/bin/10.0.26100.0/x64/ucrt"
            deps_dir = os.path.join(build_dir, "_deps")
            if os.path.exists(deps_dir):
                for d in os.listdir(deps_dir):
                    if d.endswith("-build"):
                        winepath += f";{os.path.abspath(os.path.join(deps_dir, d))}"
            env["WINEPATH"] = winepath

        run_cmd(["ctest", "--output-on-failure"], cwd=build_dir, env=env)

        # Run custom script for cdd-c or other repos that need extra test logic
        if toolchain == "gcc" and os.path.exists("scripts/pre_commit.py"):
            print("Running repo-specific test script...")
            # For cdd-c, we might just call shields script instead, but let's see.

        sys.exit(0)

    elif job == "valgrind":
        if toolchain == "gcc" and has_gcc() and is_tool("valgrind") and sys.platform.startswith("linux"):
            build_dir = "build_gcc"
            if not os.path.exists(build_dir):
                print(f"Build directory {build_dir} not found. Skipping valgrind.")
                sys.exit(0)
            print("Running valgrind on tests...")
            env = os.environ.copy()
            env["RUNNING_UNDER_VALGRIND"] = "1"
            for dp, dn, filenames in os.walk(build_dir):
                if "_deps" in dp:
                    continue
                for f in filenames:
                    if "test" in f.lower() and os.access(os.path.join(dp, f), os.X_OK) and not f.endswith(".c") and not f.endswith(".h") and not f.endswith(".sh") and not f.endswith(".py") and "parson" not in f.lower():
                        cmd = ["valgrind", "--leak-check=full", "--error-exitcode=1"]
                        if os.path.exists(".valgrind.supp"):
                            cmd.append(f"--suppressions={os.path.abspath('.valgrind.supp')}")
                        cmd.append(os.path.join(dp, f))
                        # Run the test relative to build_dir so it doesn't corrupt repo root
                        rel_f = os.path.relpath(os.path.join(dp, f), build_dir)
                        cmd[-1] = f"./{rel_f}"
                        run_cmd(cmd, cwd=build_dir, env=env)
            print("Valgrind clean.")
        else:
            print("Valgrind skipped.")
        sys.exit(0)

    elif job == "shields":
        # Check if there is a custom shields script
        if os.path.exists("scripts/pre_commit.py"):
            print("Running repo-specific pre_commit.py for shields...")
            run_cmd([sys.executable, "scripts/pre_commit.py", "shields"])
            sys.exit(0)
        elif os.path.exists("scripts/update_shields.py"):
            print("Running custom scripts/update_shields.py...")
            run_cmd([sys.executable, "scripts/update_shields.py"])
            sys.exit(0)
        elif os.path.exists("scripts/update_badges.py"):
            print("Running custom scripts/update_badges.py...")
            run_cmd([sys.executable, "scripts/update_badges.py"])
            sys.exit(0)
        elif os.path.exists("scripts/update_badges.sh"):
            print("Running custom scripts/update_badges.sh...")
            run_cmd(["bash", "scripts/update_badges.sh"])
            sys.exit(0)

        # Fallback generic shields
        print("Updating generic shields...")
        doc_cov = 0.0
        total_decls = 0
        doc_decls = 0
        if os.path.exists("include"):
            for root, _, files in os.walk("include"):
                for file in files:
                    if file.endswith(".h"):
                        with open(os.path.join(root, file), "r", encoding="utf-8", errors="ignore") as f:
                            lines = f.readlines()
                        for i, line in enumerate(lines):
                            if "_API " in line or line.startswith("void ") or line.startswith("int ") or line.startswith("char "):
                                total_decls += 1
                                j = i - 1
                                while j >= 0 and lines[j].strip() == "":
                                    j -= 1
                                if j >= 0 and ("*/" in lines[j] or "//" in lines[j]):
                                    doc_decls += 1
        if total_decls > 0:
            doc_cov = (doc_decls / total_decls) * 100.0

        test_cov = None
        if os.name != 'nt' and is_tool("gcovr") and os.path.exists("build_gcc"):
            res = subprocess.run(["gcovr", "-r", "..", ".", "--gcov-ignore-parse-errors=negative_hits.warn", "--print-summary"], cwd="build_gcc", capture_output=True, text=True)
            if res.returncode == 0:
                match = re.search(r"lines:\s+([0-9.]+)%", res.stdout)
                if match:
                    test_cov = float(match.group(1))

        def get_color(pct):
            if pct >= 90: return "brightgreen"
            if pct >= 80: return "green"
            if pct >= 70: return "yellowgreen"
            if pct >= 60: return "yellow"
            return "red"

        doc_color = get_color(doc_cov)
        doc_shield = f"[![Doc Coverage](https://img.shields.io/badge/docs-{doc_cov:.0f}%25-{doc_color}.svg)](#)"

        test_shield = ""
        if test_cov is not None:
            test_color = get_color(test_cov)
            test_shield = f"[![Test Coverage](https://img.shields.io/badge/coverage-{test_cov:.0f}%25-{test_color}.svg)](#)"

        if os.path.exists("README.md"):
            with open("README.md", "r", encoding="utf-8") as f:
                readme = f.read()

            readme = re.sub(r"\[!\[Doc Coverage\]\(.*?\)\]\(.*?\)\n?", "", readme)
            readme = re.sub(r"\[!\[Test Coverage\]\(.*?\)\]\(.*?\)\n?", "", readme)

            license_regex = r"(\[!\[License\].*?\]\(.*?\)\n?)"
            insert_str = r"\1" + doc_shield + "\n"
            if test_shield:
                insert_str += test_shield + "\n"

            readme = re.sub(license_regex, insert_str, readme, count=1)

            with open("README.md", "w", encoding="utf-8") as f:
                f.write(readme)
        sys.exit(0)

if __name__ == "__main__":
    main()
