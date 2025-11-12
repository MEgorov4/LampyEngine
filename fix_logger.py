#!/usr/bin/env python3
import sys

file_path = 'Engine/Engine/Foundation/Log/Logger.cpp'

encodings = ['utf-8', 'utf-8-sig', 'latin-1', 'cp1251', 'windows-1251']
lines = None
used_encoding = None

for encoding in encodings:
    try:
        with open(file_path, 'r', encoding=encoding) as f:
            lines = f.readlines()
        used_encoding = encoding
        break
    except (UnicodeDecodeError, UnicodeError):
        continue

if lines is None:
    print(f"Could not decode {file_path} with any encoding")
    sys.exit(1)

new_lines = []
for i, line in enumerate(lines):
    if i == 59:  # Line 60 (0-indexed is 59)
        # Remove the Cyrillic comment line
        new_lines.append('\n')
    else:
        new_lines.append(line)

with open(file_path, 'w', encoding='utf-8', newline='') as f:
    f.writelines(new_lines)

print(f"Removed Cyrillic comment from {file_path}")


