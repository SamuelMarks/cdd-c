import re
import glob

def process_file(path):
    with open(path, "r") as f:
        content = f.read()

    def replacer(m):
        dest = m.group(1).strip()
        src = m.group(2).strip()
        size = m.group(3).strip()
        return f"""#if defined(_MSC_VER)
  strncpy_s({dest}, {size} + 1, {src}, {size});
#else
  strncpy({dest}, {src}, {size});
#endif"""

    new_content = re.sub(r'strncpy\s*\(([^,]+),\s*([^,]+),\s*([^)]+)\)\s*;', replacer, content)

    if new_content != content:
        with open(path, "w") as f:
            f.write(new_content)

for filepath in glob.glob("src/**/*.c", recursive=True):
    if "test" in filepath or "mock" in filepath:
        continue
    process_file(filepath)
