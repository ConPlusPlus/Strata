#!/usr/bin/env python3
# dmm2strata.py - a first-cut D-- -> Strata source translator.
#
# The Strata compiler is bootstrapped in D--; the plan is to self-host it in Strata.
# D-- and Strata are ~90% the same C-family language, so most of the port is mechanical.
# This script does the safe, unambiguous transforms and FLAGS lines that need a human
# (features Strata doesn't have yet, or constructs too ambiguous to convert blindly), so
# the expensive part - hand-porting thousands of lines - is avoided.
#
#   python tools/dmm2strata.py compiler/src/lexer.dmm > out/lexer.strata
#
# It is intentionally conservative: when unsure, it leaves the code alone and prints a
# "// TODO(port):" marker so you can fix those spots by hand.

import re, sys, os

# --- transforms that are safe and unambiguous --------------------------------

def strip_line_terminator(line):
    # D-- ends statements/fields with `;`; Strata uses newlines. Drop a trailing `;`
    # (but not inside a string/char literal - handled crudely by ignoring lines whose
    # `;` is inside quotes).
    code, _, comment = split_comment(line)
    if code.rstrip().endswith(';') and code.count('"') % 2 == 0 and code.count("'") % 2 == 0:
        code = code.rstrip()[:-1]
    return code + comment

def split_comment(line):
    # split off a trailing // comment that isn't inside a string (crude but ok for our source)
    in_s = False; in_c = False
    i = 0
    while i < len(line) - 1:
        ch = line[i]
        if ch == '"' and not in_c: in_s = not in_s
        elif ch == "'" and not in_s: in_c = not in_c
        elif ch == '/' and line[i+1] == '/' and not in_s and not in_c:
            return line[:i], '', line[i:]
        i += 1
    return line, '', ''

def convert_include(line):
    # #include "foo.dmm" / "foo.hmm"  ->  import foo   (Strata module)
    # #include <foo.h>               ->  import <foo.h> (C header, kept)
    # A trailing // comment is kept, moved to its own line above (the driver's module
    # import must be alone on its line).
    code, _, comment = split_comment(line)
    lead = comment.strip() + '\n' if comment.strip() else ''
    m = re.match(r'\s*#include\s+"([\w./]+)\.(?:dmm|hmm|dm)"\s*$', code)
    if m:
        mod = os.path.splitext(os.path.basename(m.group(1)))[0]
        return f'{lead}import {mod}'
    m = re.match(r'(\s*)#include\s+(<[\w./]+>)\s*$', code)
    if m:
        return f'{m.group(1)}import {m.group(2)}' + (('   ' + comment) if comment else '')
    return line

def convert_new(line):
    # new Type{ ... }  ->  alloc(Type{ ... })   (flat braces only)
    return re.sub(r'\bnew\s+(\w+)\s*\{([^{}]*)\}', r'alloc(\1{\2})', line)

def convert_for(line):
    # for (int i = A; i < B; i++)  ->  for i in A..B     (the common counting idiom)
    m = re.search(r'for\s*\(\s*(?:int|int64_t)\s+(\w+)\s*=\s*(.+?);\s*\1\s*<\s*(.+?);\s*\1\s*\+\+\s*\)', line)
    if m:
        return line[:m.start()] + f'for {m.group(1)} in {m.group(2)}..{m.group(3)}' + line[m.end():]
    return line

def convert_words(line):
    code, _, comment = split_comment(line)
    code = re.sub(r'\bauto\b', 'var', code)
    code = re.sub(r'\bstr\b', 'string', code)   # \b keeps substr/int_to_str/cstr safe
    code = re.sub(r'(\w\*?)\[\]', r'\1[dynamic]', code)   # D-- T[] grows == Strata T[dynamic]
    return code + comment

# --- things Strata can't do yet / too risky to auto-convert: flag them --------

FLAGS = [
    (r'\bfn\s*\(',         'fn(...) function-pointer type - not in Strata'),
    (r'\bglobal\b',        'global - not in Strata'),
    (r'\bextern\b',        'extern - use import/link in Strata'),
    (r'for\s*\(',          'C-style for that was not the counting idiom - convert to for-in/while by hand'),
    (r'\bnew\s+\w+\s*\{[^}]*\{', 'nested new{...} - convert alloc() by hand'),
]

def flags_for(line):
    code, _, _ = split_comment(line)
    out = []
    for pat, msg in FLAGS:
        if re.search(pat, code):
            out.append(msg)
    return out

def translate(text):
    out = []
    for raw in text.split('\n'):
        line = convert_include(raw)
        if not line.lstrip().startswith('import'):
            line = convert_new(line)
            line = convert_for(line)
            line = convert_words(line)
            line = strip_line_terminator(line)
        notes = flags_for(line)   # flag what the transforms could NOT handle
        if notes:
            out.append('// TODO(port): ' + '; '.join(notes))
        out.append(line)
    return '\n'.join(out)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.stderr.write('usage: dmm2strata.py <file.dmm|.hmm>\n'); sys.exit(2)
    sys.stdout.reconfigure(encoding='utf-8', newline='\n')
    sys.stdout.write(translate(open(sys.argv[1], encoding='utf-8').read()))
