import re
import glob

def process_file(path):
    with open(path, "r") as f:
        content = f.read()

    # Match: var = fopen(arg1, arg2);
    # Ensure it's not a generator string like fprintf(fp, "fopen...")
    
    def replacer(m):
        var = m.group(1).strip()
        arg1 = m.group(2)
        arg3 = m.group(3)
        return f"""#if defined(_MSC_VER)
  fopen_s(&{var}, {arg1}, {arg3});
#else
  {var} = fopen({arg1}, {arg3});
#endif"""

    new_content = re.sub(r'([A-Za-z0-9_>\.\-]+)\s*=\s*fopen\s*\(([^,]+),\s*([^)]+)\)\s*;', replacer, content)

    if new_content != content:
        with open(path, "w") as f:
            f.write(new_content)

for filepath in glob.glob("src/**/*.c", recursive=True):
    if "test" in filepath or "mock" in filepath:
        continue
    process_file(filepath)
