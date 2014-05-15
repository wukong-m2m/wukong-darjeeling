#!/usr/bin/env python
import collections
import itertools
import sys

import string
import array

def load_memory(filename):
    regs = {}
    M = array.array('H', (0 for _ in xrange(1<<15)))
    with open(filename, 'r') as fd:
        try:
            lineno = 0
            for line in map(string.strip, fd):
                lineno += 1
                line, _, _ = line.partition('#')
                if not line:
                    continue
                if line[-1] == '*':
                    continue
                if line[0] in 'rp':
                    a = line.split()
                    regs.update( zip(a[:-1:2], a[1::2]) )
                    continue
                addr, mem, _ = line.split("  ", 2)
                addr, _ = addr.strip().split(':', 1)
                mem = (''.join(mem.strip().split())).decode('hex')
                assert len(mem) % 2 == 0
                addr = int(addr, 16)
                for i in xrange(0, len(mem), 2):
                    M[addr >> 1] = ord(mem[i]) | ord(mem[i+1])<<8
                    addr += 2
        except:
            print "in line", lineno
            print '%r' % line
            raise
    return M, regs

def main(initaddr):

    M, _ = load_memory(sys.argv[1])

    def fetch_mem(addr):
        return M[addr >> 1]

    addrs = set((initaddr,))
    done = set()
    while addrs:
        naddrs = set()
        for addr in addrs:
            naddrs |= decode(fetch_mem, addr, with_addr=True)

        done |= addrs
        addrs = naddrs - done


bswap = lambda v: (v >> 8) | ((v & 0xFF) << 8)

REGS=list('pc sp sr cq r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15'.split())

def decode(fetch_mem, addr=0x0000, with_addr=False, with_opcodes=True):
    fetched, naddr, final, called = (), None, False, None
    a = fetch_mem(addr)

    if a < 0x2000:
        form = 1
    elif a < 0x4000:
        form = 2
    else:
        form = 0

    if form == 0:
        opcode = (a & 0xF000) >> 12
        sreg   = (a & 0x0F00) >> 8
        adst   = (a & 0x0080) >> 7
        bw     = (a & 0x0040) >> 6
        asrc   = (a & 0x0030) >> 4
        dreg   = (a & 0x000f) >> 0
        r, fetched, final = decode_double(opcode, bw, asrc, sreg, adst, dreg, fetch_mem, addr)
    elif form == 1:
        opcode = (a & 0xFF80) >> 7
        bw     = (a & 0x0040) >> 6
        adst   = (a & 0x0030) >> 4
        dreg   = (a & 0x000f) >> 0
        r, fetched, called, final = decode_single(opcode, bw, adst, dreg, fetch_mem, addr)
    else:
        opcode = (a & 0xE000) >> 13
        c      = (a & 0x1C00) >> 10
        off    = (a & 0x03FF) >> 0
        r, naddr, final = decode_jump(opcode, c, off, addr)

    g = []
    if with_addr:
        g.append('%04x:  ' % addr)
    if with_opcodes:
        g.append('%04x %s %s ' % (
                bswap(a),
                ('%04x' % bswap(fetched[0])) if len(fetched) > 0 else '    ',
                ('%04x' % bswap(fetched[1])) if len(fetched) > 1 else '    ',
                ))
    print ''.join(itertools.chain(g, r))

    jumps = set()
    if not final:
        jumps.add(addr + len(fetched)*2 + 2)
    if naddr is not None:
        jumps.add(naddr)
    if called is not None and called != 0x0010:
        jumps.add(called)
    return jumps

class Instruction(object):
    def __init__(self, opcode, bw, src, dst):
        self.opcode = opcode
        self.bw = bw
        self.tab = '\t'
        self.src = src
        self.comma = ', '
        self.dst = dst

    def compile(self):
        return [self.opcode, self.bw, self.tab, self.src, self.comma, self.dst]

SOPCODES={
    0x20: 'rrc',
    0x21: 'swpb',
    0x22: 'rra',
    0x23: 'sxt',
    0x24: 'push',
    0x25: 'call',
}
def decode_single(opcode, bw, adst, dreg, fetch_mem, addr):
    fetched, called, final = [], None, False
    r = Instruction(SOPCODES[opcode], '.b' if bw else '', '', '%x/%x' % (dreg, adst))
    r.src = r.comma = ''

    if adst == 0:
        r.dst = REGS[dreg]
    elif adst == 2:
        r.dst = '@' + REGS[dreg]
    elif adst == 3 and dreg == 0: # immediate
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        r.dst = "#0x%x" % (b,)
        called = b

    return r.compile(), fetched, called, final

OPCODES={
    0x4: 'mov',
    0x5: 'add',
    0x6: 'addc',
    0x7: 'subc',
    0x8: 'sub',
    0x9: 'cmp',
    0xa: 'dadd',
    0xb: 'bit',
    0xc: 'bic',
    0xd: 'bis',
    0xe: 'xor',
    0xf: 'and',
    }
def decode_double(opcode, bw, asrc, sreg, adst, dreg, fetch_mem, addr):
    fetched, final = [], False
    r = Instruction(OPCODES[opcode],
                    '.b ' if bw else ' ',
                    '%x/%x' % (sreg, asrc),
                    '%x/%x' % (dreg, adst))

    if sreg == 2 and asrc == 2:
        r.src = '#0x4'
    elif sreg == 2 and asrc == 3:
        r.src = '#0x8'
    elif r.opcode == 'cmp' and sreg == 3 and asrc == 0:
        r.opcode = 'tst'
        r.src = r.comma = ''
    elif r.opcode == 'mov' and sreg == 3 and asrc == 0:
        r.opcode = 'clr'
        r.src = r.comma = ''
    elif sreg == 3 and asrc == 0:
        r.src = '#0x0'
    elif r.opcode == 'add' and sreg == 3 and asrc == 1:
        r.opcode = 'inc'
        r.src = r.comma = ''
    elif r.opcode == 'add' and sreg == 3 and asrc == 2:
        r.opcode = 'incd'
        r.src = r.comma = ''
    elif r.opcode == 'sub' and sreg == 3 and asrc == 1:
        r.opcode = 'dec'
    elif r.opcode == 'sub' and sreg == 3 and asrc == 2:
        r.opcode = 'decd'
        r.src = r.comma = ''
    elif sreg ==3 and asrc == 1:
        r.src = '#0x1'
    elif sreg == 3 and asrc == 2:
        r.src = '#0x2'
    elif sreg == 3 and asrc == 3:
        r.src = '#-0x1'
    elif asrc == 0:
        r.src = REGS[sreg]
    elif asrc == 1 and sreg == 2: # absolute
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        r.src = "&0x%04x" % (b,)
    elif asrc == 1: # relative
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        r.src = "0x%04x(%s)" % (b, REGS[sreg],)
    elif asrc == 2:
        r.src = '@%s' % (REGS[sreg],)
    elif asrc == 3 and sreg == 0: # immediate
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        r.src = "#0x%x" % (b,)
    elif asrc == 3 and sreg == 1:
        r.opcode = 'pop'
        r.src = r.comma = ''
    elif asrc == 3:
        r.src = '@%s+' % (REGS[sreg],)

    if adst == 0:
        r.dst = REGS[dreg]
    elif adst == 1 and dreg == 2: # absolute
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        r.dst = "&0x%04x" % (b,)
    elif adst == 1: # relative
        addr += 2; b = fetch_mem(addr); fetched.append( b )
        s = ''
        if b & 0x8000:
            s = '-'
            b = 0xFFFF - b + 1
        r.dst = "%s0x%x(%s)" % (s, b, REGS[dreg],)

    if asrc == 3 and sreg == 1 and adst == 0 and dreg == 0:
        r.opcode = 'ret'
        r.src = r.comma = r.dst = ''
        final = True
    elif r.opcode == 'mov' and adst == 0 and dreg == 0:
        r.opcode = 'br'
        r.comma = r.dst = ''
        final = True
    return r.compile(), fetched, final

JMPOPCODES = 'jnz jeq jnc jc jn jge jl jmp'.split()
def decode_jump(opcode, c, off, addr):
    assert opcode == 1
    s = '+'
    if off & 0x0200:
        s = '-'
        off = 0x3FF - off
    else:
        off += 0 # from next instruction
    delta = off * 2
    if s == '-':
        n = addr + delta * -1
    else:
        n = addr + delta +2
    final = (c == 7)
    return ('%s\t$%s0x%x\t\t; ->%04x' % (
            JMPOPCODES[c], s, delta, n), n, final)

def X(hexstring):
    s = ''.join(hexstring.split()).decode('hex')
    M = array.array('H')
    M.fromstring(s)
    return lambda addr: M[addr >> 1]



def tdecode(hexstring):
    decode(X(hexstring), with_opcodes=False)



def test():
    '''
    >>> tdecode('1a53')
    inc  r10
    >>> tdecode('1a53')
    inc	r10
    >>> tdecode('0d93')
    tst	r13
    >>> tdecode('013c')
    jmp $+0x4
    >>> tdecode('0d3c')
    jmp $+0x1c
    >>> tdecode('df3b')
    jl $-0x40
    >>> tdecode('0843')
    clr	r8
    >>> tdecode('2a53')
    incd	r10
    >>> tdecode('3041')
    ret
    >>> tdecode('3f40adde')
    mov         #0xdead, r15
    >>> tdecode('82451111')
    mov	r5, &0x1111
    >>> tdecode('d1a1 cf43 1111')
    dadd.b	0x43cf(sp), 0x1111(sp)
    >>> tdecode('0012')
    push pc
    >>> tdecode('0e10')
    rrc r14
    >>> tdecode('8c10')
    swpb r12
    >>> tdecode('8c11')
    sxt r12
    >>> tdecode('0c12')
    push r12
    >>> tdecode('4c12')
    push.b r12
    >>> tdecode('8c12')
    call r12
    >>> tdecode('004c')
    br r12
    >>> tdecode('204c')
    br @r12
    >>> tdecode('ac12')
    call @r12
    >>> tdecode('b0121000')
    call #0x10
    >>> tdecode('30401000')
    br #0x10
    >>> tdecode('2f83')
    decd r15
    >>> tdecode('bf4f feef')
    mov @r15+, -0x1002(r15)
    >>> tdecode('0e11')
    rra r14
    >>> tdecode('3090 0080')
    cmp #0x8000, pc
    '''
    pass

if __name__ == "__main__":
    if len(sys.argv)  < 2:
        import doctest
        doctest.testmod(optionflags=doctest.NORMALIZE_WHITESPACE)
    else:
        # 0x4400
        # 0x44be
        #0x44d8
        #main(0x44e2)
        #main(0x44ec)
        #main(0x44fe)
        #main(0x4508)
        main(int(sys.argv[2], 16))
