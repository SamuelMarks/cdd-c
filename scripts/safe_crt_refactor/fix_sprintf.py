import re
import glob

def process_file(path):
    with open(path, "r") as f:
        content = f.read()

    new_content = content

    def replacer_sprintf(m):
        spaces = m.group(1)
        last_nl = spaces.rfind('\n')
        indent = spaces[last_nl+1:] if last_nl != -1 else spaces
        prefix = spaces[:last_nl+1] if last_nl != -1 else ""
        
        dest = m.group(2).strip()
        fmt = m.group(3).strip()
        # To make it simple, we use sizeof(dest) which works for arrays. If it's a pointer, it fails. 
        # But this is C89 so maybe we just write it generically or ignore for now?
        # Let's skip automatic sprintf since it's hard to get the size right without AST.
        pass

