#!/usr/bin/python
# generate table of TXN_ASSERT() locations and assign unique ids for each
# by patching the binary
# all object files using TXN_ASSERT must be processed on the same command line!
# then the resulting .c file should be compiled and linked in

import sys
import os
import re

global identifier
identifier = 0

class Section:
    def __init__(self, name, foff, vma):
        self.name = name
        self.foff = foff
        self.vaddr = vma

def addr2line(fn, addr):
    a2l = os.popen("addr2line -e %s %#x" % (fn, addr))
    res = a2l.readline()
    a2l.close()
    return res.rstrip()

def process(i, pfile):
    global identifier
    sections = {}
    func = None
    section = None
    objdump = os.popen("objdump -dfh " + i + " | expand", 'r', 8192)
    for o in objdump:
        if re.match("^Sections:$", o):
            for o in objdump:
                if re.match(r"\s+\d+\s", o):
                    (idx, name, size, vma, lma, foff, algn) = o.split()
                    if name != ".txn_abort":
                        continue
        	    sections[name] = Section(name, int(foff, 16), int(vma, 16))
                    #print >>sys.stderr,"section header %s" % (name,)
                if re.match(r"^$", o): 
                    break
        m = re.match(r"Disassembly of section ([^:]+):", o)
        if m: 
            if m.group(1) == ".txn_abort":
                section = sections[m.group(1)]
            else:
                section = None
            continue
        if section:
            m = re.match(r"\s+([0-9a-f]+):\s+", o)
            if m:
                vaddr = int(m.group(1), 16)
                if re.match("\s*xabort", o[32:]):
                    foff = vaddr - section.vaddr + section.foff
                    if pfile:
                        if identifier > 254:
                            # XXX fix assert code to print multiple
                            print >>sys.stderr, "error: too many txn_asserts"
                        pfile.seek(foff + 2)
                        pfile.write("%c" % (identifier & 0xff, ))
                m = re.match("\s*jmpq\s+([0-9a-f]+)", o[32:])
                if m:
                    pos = addr2line(i, int(m.group(1), 16))
                    print " [%d] = \"%s\"," % (identifier, pos)
                    identifier += 1
                     

print "/* Auto generated. Do not edit */"
print "char *txn_assert_table[] = {"
for i in sys.argv[1:]:
    pfile = open(i, "r+")
    process(i, pfile)
    pfile.close()
print "};"
print
print "int txn_assert_table_size = sizeof(txn_assert_table) / sizeof(char *);"

