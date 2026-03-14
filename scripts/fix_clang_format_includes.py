import os
import re

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return

    # Remove all existing clang-format off/on
    content = re.sub(r'/\*\s*clang-format\s+off\s*\*/\n?', '', content)
    content = re.sub(r'/\*\s*clang-format\s+on\s*\*/\n?', '', content)

    lines = content.split('\n')
    
    first_include = -1
    last_include = -1
    for i, line in enumerate(lines):
        if line.strip().startswith('#include'):
            if first_include == -1:
                first_include = i
            last_include = i

    if first_include == -1:
        # No includes, no need to format wrap
        with open(filepath, 'w', encoding='utf-8', newline='') as f:
            f.write(content)
        return

    # Find start: backtrack to cover any preceding #if / #define that seem related to includes
    start_idx = first_include
    while start_idx > 0:
        prev_line = lines[start_idx-1].strip()
        if prev_line.startswith('#if') or prev_line.startswith('#def') or prev_line == '':
            start_idx -= 1
        else:
            break
            
    # Remove leading empty lines from start_idx
    while start_idx < first_include and lines[start_idx].strip() == '':
        start_idx += 1

    # Find end: fast forward to cover any trailing #endif or #else related to includes
    end_idx = last_include
    while end_idx < len(lines)-1:
        next_line = lines[end_idx+1].strip()
        if next_line.startswith('#endif') or next_line.startswith('#else') or next_line.startswith('#elif') or next_line == '':
            end_idx += 1
        else:
            break
            
    # Remove trailing empty lines from end_idx
    while end_idx > last_include and lines[end_idx].strip() == '':
        end_idx -= 1

    # Now we insert
    lines.insert(end_idx + 1, "/* clang-format on */")
    lines.insert(start_idx, "/* clang-format off */")

    new_content = '\n'.join(lines)
    with open(filepath, 'w', encoding='utf-8', newline='') as f:
        f.write(new_content)

def main():
    for root, _, files in os.walk('src'):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                process_file(os.path.join(root, file))
    for root, _, files in os.walk('include'):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                process_file(os.path.join(root, file))

if __name__ == '__main__':
    main()
