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

NEW_RE = re.compile(r'\bnew\s+(\w+)\s*\{')

def convert_new_text(text):
    # new Type{ ... }  ->  alloc(Type{ ... })  over the WHOLE text, matching braces, so
    # literals spanning lines and nested literals convert too. Skips strings, chars and
    # // comments while matching.
    out = []
    i = 0
    while True:
        m = NEW_RE.search(text, i)
        if not m:
            out.append(text[i:])
            return ''.join(out)
        # the `new` must be code, not inside a comment or string on its line
        line_start = text.rfind('\n', 0, m.start()) + 1
        code, _, _ = split_comment(text[line_start:m.start()] + 'x')
        if not code.endswith('x') or code.count('"') % 2 == 1:
            out.append(text[i:m.end()])
            i = m.end()
            continue
        j = m.end()            # just past the opening `{`
        depth = 1
        while j < len(text) and depth > 0:
            c = text[j]
            if c in '"\'':    # skip a string/char literal
                q = c; j += 1
                while j < len(text) and text[j] != q:
                    j += 2 if text[j] == '\\' else 1
            elif c == '/' and text[j:j+2] == '//':
                j = text.find('\n', j)
                if j < 0: j = len(text)
                continue
            elif c == '{': depth += 1
            elif c == '}': depth -= 1
            j += 1
        if depth > 0:          # unbalanced: leave it for a human
            out.append(text[i:m.end()])
            i = m.end()
            continue
        inner = convert_new_text(text[m.end():j-1])   # nested `new`s
        out.append(text[i:m.start()] + 'alloc(' + m.group(1) + '{' + inner + '})')
        i = j

def mask_literals(code):
    # Same length as `code`, with the INSIDE of every string/char literal replaced by `_`,
    # so patterns only ever match real code (e.g. a C `for (...)` inside a string that
    # codegen emits must be left alone).
    out = list(code)
    i = 0
    while i < len(code):
        q = code[i]
        if q in '"\'':
            j = i + 1
            while j < len(code) and code[j] != q:
                j += 2 if code[j] == '\\' else 1
            for k in range(i + 1, min(j, len(code))):
                out[k] = '_'
            i = j + 1
        else:
            i += 1
    return ''.join(out)

FOR_RE = re.compile(r'for\s*\(\s*(?:int|int64_t)\s+(\w+)\s*=\s*(.+?);\s*\1\s*(<=?)\s*(.+?);\s*\1\s*\+\+\s*\)')

def convert_for(line):
    # for (int i = A; i < B; i++)   ->  for i in A..B       (the common counting idiom)
    # for (int i = A; i <= B; i++)  ->  for i in A..(B) + 1 (`..` binds loosest)
    m = FOR_RE.search(mask_literals(line))
    if m:
        m = FOR_RE.match(line, m.start())   # same span, real text for the groups
        hi = m.group(4) if m.group(3) == '<' else f'({m.group(4)}) + 1'
        return line[:m.start()] + f'for {m.group(1)} in {m.group(2)}..{hi}' + line[m.end():]
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
    (r'\bnew\s+\w+\s*\{', 'new T{...} the translator could not convert - use alloc(T{...})'),
]

def flags_for(line):
    code, _, _ = split_comment(line)
    code = mask_literals(code)
    out = []
    for pat, msg in FLAGS:
        if re.search(pat, code):
            out.append(msg)
    return out

def translate(text):
    out = []
    text = convert_new_text(text)
    # file header: `// src/lexer.dmm - ..., written in D--` names the Strata file instead
    text = re.sub(r'^// src/(\w+)\.(?:dmm|hmm) - ', r'// selfhost/\1.strata - ', text, count=1, flags=re.M)
    text = text.replace('written in D-- (', 'written in Strata (translated from the D-- bootstrap seed; ', 1)
    text = text.replace(', written in D--', ', written in Strata (translated from the D-- bootstrap seed)', 1)
    # D-- `int main(str[] args) {...}` -> a plain function, called from Strata's top-level
    # code (Strata's entry point) with args() and its result as the exit code.
    main_re = re.compile(r'^int\s+main\s*\(\s*str\s*\[\s*\]\s*(\w+)\s*\)', re.M)
    if main_re.search(text):
        text = main_re.sub(r'int dmm_main(str[] \1)', text, count=1)
        text = text.rstrip('\n') + ('\n\n// entry point (translated from D--\'s `int main(str[] args)`)\n'
                                    'import <stdlib.h>\nexit(dmm_main(args()));\n')
    for raw in text.split('\n'):
        line = convert_include(raw)
        if not line.lstrip().startswith('import'):
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
