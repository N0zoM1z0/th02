#!/usr/bin/env python3
"""Unified diff of target vs built disassembly at one program offset.

    scripts/th02_sbs.py PRODUCT START LEN [BUILT_START]   # hex offsets

Needs capstone (requirements-ci.txt); both sides are rebased to 0.
"""
import sys,capstone,difflib
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lib.pc98 import parse_mz
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
p=sys.argv[1]; s=int(sys.argv[2],16); n=int(sys.argv[3],16)
bs=int(sys.argv[4],16) if len(sys.argv)>4 else s
t=parse_mz(open(f'{R}/.analysis/targets/th02/{p}.exe','rb').read()).program_image
b=parse_mz(open(f'{R}/.analysis/work/rec98/bin/th02/{p}.exe','rb').read()).program_image
md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_16)
def d(img,base):
    return [f"{i.address:04x} {i.mnemonic} {i.op_str}" for i in md.disasm(img[base:base+n],0)]
A=d(t,s); B=d(b,bs)
for l in difflib.unified_diff(A,B,'target','built',n=3,lineterm=''): print(l)
