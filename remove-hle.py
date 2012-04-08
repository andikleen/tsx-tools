#!/usr/bin/python
# remove HLE prefixes in a ELF binary.
# WARNING: may corrupt the binary. only run on backup copies
# remove-hle.py [-p] binary ...
# -p enables patching, otherwise just print what would be done

import sys
import os
import re

release = r"(repz|xrelease) "
acq_or_rel = r"(repnz|xacquire|repz|xrelease) "
lock_instr = r"(cmpxchg|xchg|xadd|inc|dec|add|sub|adc|and|btc|btr|bts|neg|not|or|sbb|xor) "
lock = "lock "
match = ("(" + lock + acq_or_rel + lock_instr + ")|" +
         "(" + acq_or_rel + lock + lock_instr + ")|" +
         "(" + release + "mov" + ")|" +
         "(" + acq_or_rel + "xchg" + ")")
#print match
 
class Section:
    def __init__(self, name, foff, vma):
        self.name = name
        self.foff = foff
        self.vaddr = vma

def process(i, pfile):
    sections = {}
    func = None
    section = None
    objdump = os.popen("objdump -dfh " + i + " | expand", 'r', 8192)
    for o in objdump:
        if re.match("^Sections:$", o):
            for o in objdump:
                if re.match(r"\s+\d+\s", o):
                    (idx, name, size, vma, lma, foff, algn) = o.split()
        	    sections[name] = Section(name, int(vma, 16), int(foff, 16))
                    #print "section header %s" % (name,)
                if re.match(r"^$", o): 
                    break
        m = re.match(r"Disassembly of section ([^:]+):", o)
        if m:
            section = sections[m.group(1)]
            print "section %s" % (m.group(1), )
            continue
        if section:
            m = re.search(r"<(.*)>:", o)
            if m:
                func = m.group(1)
                continue
            m = re.match(r"\s+([0-9a-f]+):\s+", o)
            if m:
                vaddr = int(m.group(1), 16)
                global match
                if re.match(match, o[32:]):
                    lockfirst = o[32:36] == "lock"
                    foff = vaddr - section.vaddr + section.foff
                    if lockfirst:
                        foff += 1
                    print "func %s fileaddr %x\n" % (func, foff)
                    print o
                    if pfile:
                        pfile.seek(foff)
                        pfile.write("%c" % (0x90,))

patchmode = False
for i in sys.argv[1:]:
    if i == "-p":
        patchmode = True
        continue
    if patchmode:
        pfile = open(i, "r+")
        process(i, pfile)
        pfile.close()
    else:
        process(i, None)
