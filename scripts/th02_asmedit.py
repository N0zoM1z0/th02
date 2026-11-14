#!/usr/bin/env python3
"""Edit an assembly file while moving procedures into C++ (latin-1, CRLF kept).

    python3 scripts/th02_asmedit.py th02_main.asm <<'EOF'
    remove_proc("mima_update")              # proc + its IDA header comment
    export_proc("mima_193A4", "@mima_193A4$qv")  # rename everywhere + public
    export_data("word_26C68")               # -> _word_26C68 everywhere + public
    rename("old", "new"); replace("exact text", "new text")
    EOF

Run inside .analysis/work/rec98. The script body is Python with `b` as text.
"""
import re
import sys

path = sys.argv[1]
b = open(path, "rb").read().decode("latin-1")
B = r"(?<![\w@$?])"
E = r"(?![\w@$?])"


def rename(old, new):
    global b
    b, n = re.subn(B + re.escape(old) + E, new.replace("\\", "\\\\"), b)
    return n


def export_data(label, new=None):
    global b
    new = new or ("_" + label)
    assert re.search(r"^" + re.escape(label) + r"(?=[ \t])", b, re.M), label
    rename(label, new)
    m = re.search(r"^" + re.escape(new) + r"(?=[ \t])", b, re.M)
    s = m.start()
    b = b[:s] + f"public {new}\r\n" + b[s:]
    return new


def export_proc(name, mangled):
    global b
    assert re.search(r"^" + re.escape(name) + r"\s+proc", b, re.M), name
    rename(name, mangled)
    m = re.search(r"^" + re.escape(mangled) + r"\s+proc", b, re.M)
    b = b[:m.start()] + f"public {mangled}\r\n" + b[m.start():]


def remove_proc(name):
    global b
    m = re.search(r"^" + re.escape(name) + r"\s+proc\b.*$", b, re.M)
    assert m, name
    start = m.start()
    # Include the IDA header comment block and preceding blank lines.
    head = b.rfind("; =============== S U B", 0, start)
    if head != -1 and "endp" not in b[head:start] and "segment" not in b[head:start]:
        start = b.rfind("\n", 0, head) + 1
    e = re.search(r"^" + re.escape(name) + r"\s+endp[^\n]*\n", b[m.end():], re.M)
    assert e, name
    end = m.end() + e.end()
    removed = b[start:end]
    b = b[:start] + b[end:]
    return removed


def remove_between(start_marker, end_marker):
    """Remove text from start_marker (inclusive) to end_marker (exclusive)."""
    global b
    s = b.index(start_marker)
    e = b.index(end_marker, s)
    b = b[:s] + b[e:]


def replace(old, new, count=1):
    global b
    assert b.count(old) == count, (old, b.count(old))
    b = b.replace(old, new)


def insert_after(marker, text):
    global b
    i = b.index(marker) + len(marker)
    b = b[:i] + text + b[i:]


exec(sys.stdin.read())
open(path, "wb").write(b.encode("latin-1"))
