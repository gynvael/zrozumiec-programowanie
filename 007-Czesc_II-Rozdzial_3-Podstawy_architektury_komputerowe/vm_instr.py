#!/usr/bin/python
# watch pep8 --show-pep8 --ignore=E111,E114,E241,W391 ./vm.py
from struct import unpack


# Helper functions.
def to_dw(args):
  return unpack("<H", str(args))[0]


def to_dd(args):
  return unpack("<I", str(args))[0]


def VMOV(vm, args):
  vm.reg(args[0]).v = vm.reg(args[1]).v


def VSET(vm, args):
  vm.reg(args[0]).v = to_dd(args[1:1 + 4])


def VLD(vm, args):
  dd = vm.mem.fetch_dword(vm.reg(args[1]).v)
  if dd is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.reg(args[0]).v = dd


def VST(vm, args):
  if not vm.mem.store_dword(vm.reg(args[0]).v,
                            vm.reg(args[1]).v):
    vm.interrupt(vm.INT_MEMORY_ERROR)


def VLDB(vm, args):
  db = vm.mem.fetch_byte(vm.reg(args[1]).v)
  if db is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.reg(args[0]).v = db


def VSTB(vm, args):
  if not vm.mem.store_byte(vm.reg(args[0]).v,
                           vm.reg(args[1]).v & 0xff):
    vm.interrupt(vm.INT_MEMORY_ERROR)


def VADD(vm, args):
  vm.reg(args[0]).v = ((vm.reg(args[0]).v +
                        vm.reg(args[1]).v) & 0xffffffff)


def VSUB(vm, args):
  vm.reg(args[0]).v = ((vm.reg(args[0]).v -
                        vm.reg(args[1]).v) & 0xffffffff)


def VMUL(vm, args):
  vm.reg(args[0]).v = ((vm.reg(args[0]).v *
                        vm.reg(args[1]).v) & 0xffffffff)


def VDIV(vm, args):
  if vm.reg(args[1]).v == 0:
    vm.interrupt(vm.INT_DIVISION_ERROR)
  else:
    vm.reg(args[0]).v = (vm.reg(args[0]).v /
                         vm.reg(args[1]).v)


def VMOD(vm, args):
  if vm.reg(args[1]).v == 0:
    vm.interrupt(vm.INT_DIVISION_ERROR)
  else:
    vm.reg(args[0]).v = (vm.reg(args[0]).v %
                         vm.reg(args[1]).v)


def VOR(vm, args):
  vm.reg(args[0]).v |= vm.reg(args[1]).v


def VAND(vm, args):
  vm.reg(args[0]).v &= vm.reg(args[1]).v


def VXOR(vm, args):
  vm.reg(args[0]).v ^= vm.reg(args[1]).v


def VNOT(vm, args):
  vm.reg(args[0]).v = (vm.reg(args[0]).v ^ 0xffffffff) + 0x1


def VSHL(vm, args):
  vm.reg(args[0]).v = ((vm.reg(args[0]).v <<
                        (vm.reg(args[1]).v & 0x1f)) & 0xffffffff)


def VSHR(vm, args):
  vm.reg(args[0]).v = ((vm.reg(args[0]).v >>
                        (vm.reg(args[1]).v & 0x1f)))


def VCMP(vm, args):
  res = vm.reg(args[0]).v - vm.reg(args[1]).v
  vm.fr &= 0xfffffffc

  if res == 0:
    vm.fr |= vm.FLAG_ZF

  if res < 0:
    vm.fr |= vm.FLAG_CF


def VJZ(vm, args):
  if vm.fr & vm.FLAG_ZF:
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJNZ(vm, args):
  if not (vm.fr & vm.FLAG_ZF):
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJC(vm, args):
  if vm.fr & vm.FLAG_CF:
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJNC(vm, args):
  if not (vm.fr & vm.FLAG_CF):
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJBE(vm, args):
  if (vm.fr & vm.FLAG_CF) or (vm.fr & vm.FLAG_ZF):
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJA(vm, args):
  if not (vm.fr & vm.FLAG_CF) and not (vm.fr & vm.FLAG_ZF):
    vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VPUSH(vm, args):
  vm.sp.v = vm.sp.v - 4
  if vm.mem.store_dword(vm.sp.v, vm.reg(args[0]).v) is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)


def VPOP(vm, args):
  res = vm.mem.fetch_dword(vm.sp.v)
  if res is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.reg(args[0]).v = res
  vm.sp.v = vm.sp.v + 4


def VJMP(vm, args):
  vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VJMPR(vm, args):
  vm.pc.v = vm.reg(args[0]).v & 0xffff

def VCALL(vm, args):
  vm.sp.v = vm.sp.v - 4
  if vm.mem.store_dword(vm.sp.v, vm.pc.v) is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.pc.v = (vm.pc.v + to_dw(args[0:2])) & 0xffff


def VCALLR(vm, args):
  vm.sp.v = vm.sp.v - 4
  if vm.mem.store_dword(vm.sp.v, vm.pc.v) is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.pc.v = vm.reg(args[0]).v & 0xffff


def VRET(vm, args):
  res = vm.mem.fetch_dword(vm.sp.v)
  if res is None:
    vm.interrupt(vm.INT_MEMORY_ERROR)
    return
  vm.pc.v = res
  vm.sp.v = vm.sp.v + 4


def VCRL(vm, args):
  v = vm.reg(args[0]).v
  cr_id = to_dw(args[1:1 + 2])
  if cr_id not in vm.cr:
    vm.interrupt(vm.INT_GENERAL_ERROR)
    return

  # Delay setting this register after the interrupts are processed.
  def defered_load():
    vm.cr[cr_id] = v

  vm.defered_queue.append(defered_load)


def VCRS(vm, args):
  cr_id = to_dw(args[1:1 + 2])
  if cr_id not in vm.cr:
    vm.interrupt(vm.INT_GENERAL_ERROR)
    return
  vm.reg(args[0]).v = vm.cr[cr_id]


def VOUTB(vm, args):
  port = args[1]
  if port not in vm.io:
    return
  vm.io[port].handle_inbound(port, vm.reg(args[0]).v & 0xff)


def VINB(vm, args):
  port = args[1]
  if port not in vm.io:
    return
  vm.reg(args[0]).v = vm.io[port].handle_outbound(port) & 0xff


def VIRET(vm, args):
  tmp_sp = vm.sp.v
  for rid in xrange(16, -1, -1):
    v = vm.mem.fetch_dword(tmp_sp)
    if v is None:
      vm.interrupt(vm.INT_GENERAL_ERROR)
      return
    if rid == 16:
      vm.fr = v
    else:
      vm.r[rid].v = v
    tmp_sp += 4


def VCRSH(vm, args):
  vm.crash()


def VOFF(vm, args):
  vm.terminated = True


VM_OPCODES = {
    0x00: (VMOV, 1 + 1),  0x01: (VSET, 1 + 4),
    0x02: (VLD,  1 + 1),  0x03: (VST,  1 + 1),
    0x04: (VLDB, 1 + 1),  0x05: (VSTB, 1 + 1),

    0x10: (VADD, 1 + 1),  0x11: (VSUB, 1 + 1),
    0x12: (VMUL, 1 + 1),  0x13: (VDIV, 1 + 1),
    0x14: (VMOD, 1 + 1),  0x15: (VOR,  1 + 1),
    0x16: (VAND, 1 + 1),  0x17: (VXOR, 1 + 1),
    0x18: (VNOT, 1),      0x19: (VSHL, 1 + 1),
    0x1A: (VSHR, 1 + 1),

    0x20: (VCMP, 1 + 1),  0x21: (VJZ, 2),
    0x22: (VJNZ, 2),      0x23: (VJC, 2),
    0x24: (VJNC, 2),      0x25: (VJBE, 2),
    0x26: (VJA, 2),

    0x30: (VPUSH, 1),     0x31: (VPOP, 1),

    0x40: (VJMP, 2),      0x41: (VJMPR, 1),
    0x42: (VCALL, 2),     0x43: (VCALLR, 1),
    0x44: (VRET, 0),

    0xF0: (VCRL, 1 + 2),  0xF1: (VCRS, 1 + 2),
    0xF2: (VOUTB, 1 + 1), 0xF3: (VINB, 1 + 1),
    0xF4: (VIRET, 0),
    0xFE: (VCRSH, 0),     0xFF: (VOFF, 0)
}
