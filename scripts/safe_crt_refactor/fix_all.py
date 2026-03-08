import re
import glob

def process_file(path):
    with open(path, "r") as f:
        content = f.read()

    new_content = content

    def replacer_fopen(m):
        spaces = m.group(1)
        last_nl = spaces.rfind('\n')
        indent = spaces[last_nl+1:] if last_nl != -1 else spaces
        prefix = spaces[:last_nl+1] if last_nl != -1 else ""
        
        var = m.group(2).strip()
        arg1 = m.group(3)
        arg3 = m.group(4)
        return f"""{prefix}#if defined(_MSC_VER)
{indent}fopen_s(&{var}, {arg1}, {arg3});
#else
{indent}{var} = fopen({arg1}, {arg3});
#endif"""

    new_content = re.sub(r'(?<!#else\n)(?<!#else\r\n)(?<!#else\n  )(?<!#else\n\t)(\s*)([a-zA-Z0-9_>\.\-]+)\s*=\s*fopen\s*\(([^,]+),\s*([^)]+)\)\s*;', replacer_fopen, new_content)

    def replacer_strcpy(m):
        spaces = m.group(1)
        last_nl = spaces.rfind('\n')
        indent = spaces[last_nl+1:] if last_nl != -1 else spaces
        prefix = spaces[:last_nl+1] if last_nl != -1 else ""
        
        dest = m.group(2).strip()
        src = m.group(3).strip()
        return f"""{prefix}#if defined(_MSC_VER)
{indent}strcpy_s({dest}, sizeof({dest}), {src});
#else
{indent}strcpy({dest}, {src});
#endif"""

    new_content = re.sub(r'(?<!#else\n)(?<!#else\r\n)(?<!#else\n  )(?<!#else\n\t)(\s*)\bstrcpy\s*\(([^,]+),\s*([^)]+)\)\s*;', replacer_strcpy, new_content)

    def replacer_strncpy(m):
        spaces = m.group(1)
        last_nl = spaces.rfind('\n')
        indent = spaces[last_nl+1:] if last_nl != -1 else spaces
        prefix = spaces[:last_nl+1] if last_nl != -1 else ""
        
        dest = m.group(2).strip()
        src = m.group(3).strip()
        size = m.group(4).strip()
        return f"""{prefix}#if defined(_MSC_VER)
{indent}strncpy_s({dest}, {size} + 1, {src}, {size});
#else
{indent}strncpy({dest}, {src}, {size});
#endif"""

    new_content = re.sub(r'(?<!#else\n)(?<!#else\r\n)(?<!#else\n  )(?<!#else\n\t)(\s*)\bstrncpy\s*\(([^,]+),\s*([^,]+),\s*([^)]+)\)\s*;', replacer_strncpy, new_content)

    if new_content != content:
        with open(path, "w") as f:
            f.write(new_content)


import glob
for f in glob.glob('src/**/*.c', recursive=True) + glob.glob('src/**/*.h', recursive=True):
    if '/tests/' not in f.replace('\\', '/') and '/mocks/' not in f.replace('\\', '/'):
        process_file(f)
