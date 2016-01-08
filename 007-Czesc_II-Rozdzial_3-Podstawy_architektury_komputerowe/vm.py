#!/usr/bin/python
# watch pep8 --show-pep8 --ignore=E111,E114,E241,W391 ./vm.py
import collections
import os
import sys
import threading
from vm_memory import VMMemory
from vm_dev_timer import VMDeviceTimer
from vm_dev_con import VMDeviceConsole
from vm_regs import VMGeneralPurposeRegister
from vm_instr import VM_OPCODES


class VMInstance(object):
  INSTR_HANDLER = 0
  INSTR_LENGTH = 1
  INT_MEMORY_ERROR = 0
  INT_DIVISION_ERROR = 1
  INT_GENERAL_ERROR = 2
  INT_PIT = 8
  INT_CONSOLE = 9
  FLAG_ZF = (1 << 0)
  FLAG_CF = (1 << 1)
  CREG_INT_FIRST = 0x100
  CREG_INT_LAST = 0x10f
  CREG_INT_CONTROL = 0x110
  MASKABLE_INTS = [8, 9]

  def __init__(self):
    self.r = [VMGeneralPurposeRegister() for _ in xrange(16)]
    self.fr = 0
    self.sp = self.r[14]
    self.pc = self.r[15]
    self.mem = VMMemory()
    self.terminated = False
    self.opcodes = VM_OPCODES

    self.sp.v = 0x10000
    self.cr = {}

    # Interrupt registers.
    for creg in xrange(self.CREG_INT_FIRST, self.CREG_INT_LAST + 1):
      self.cr[creg] = 0xffffffff

    self.cr[self.CREG_INT_CONTROL] = 0  # Maskable interrupts disabled.

    self.dev_pit = VMDeviceTimer(self)
    self.dev_console = VMDeviceConsole(self)

    self.io = {
        0x20: self.dev_console,
        0x21: self.dev_console,
        0x22: self.dev_console,
        0x70: self.dev_pit,
        0x71: self.dev_pit
    }

    self.interrupt_queue = []
    self.interrupt_queue_mutex = threading.Lock()

    self.defered_queue = collections.deque()

  def reg(self, r):
    """Given a register ID, returns a reference to the register object. It takes
    only the lower 4 bits of the ID into account ignoring any upper bits.
    """
    return self.r[r & 0xf]

  def crash(self):
    """Terminates the virtual machine on critical error.
    """
    self.terminated = True
    print "The virtual machine entered an erroneous state and is terminating."
    print "Register values at termination:"
    for ri, r in enumerate(vm.r):
      print "  r%u = %x" % (ri, r.v)

  def interrupt(self, i):
    """Add an interrupt to the interrupt queue.
    """
    with self.interrupt_queue_mutex:
      self.interrupt_queue.append(i)

  def _fetch_pending_interrupt(self):
    """Returns a pending interrupt to be processed. If maskable interrupts are
    disabled, returns a non-maskable interrupt (NMI) if available. If no
    interrupts are available for processing, returns None.
    """
    with self.interrupt_queue_mutex:
      if not self.interrupt_queue:
        return None

      # In disable-interrupts state we can process only non-maskable interrupts
      # (faults).
      if self.cr[self.CREG_INT_CONTROL] & 1 == 0:
        # Maskable interrupts disabled. Find a non-maskable one.
        for i, interrupt in enumerate(self.interrupt_queue):
          if interrupt not in self.MASKABLE_INTS:
            return self.interrupt_queue.pop(i)

        # No non-maskable interrupts found.
        return None

      # Return the first interrupt available.
      return self.interrupt_queue.pop()

  def _process_interrupt_queue(self):
    """Processes an interrupt if available.
    """
    i = self._fetch_pending_interrupt()

    if i is None:
      return True

    # Save context.
    tmp_sp = self.sp.v
    for r in [rr.v for rr in self.r] + [self.fr]:
      tmp_sp -= 4
      if self.mem.store_dword(tmp_sp, r) is False:
        # Since there is no way to save state, and therefore no way to
        # recover, crash the machine.
        self.crash()
        return False

    self.sp.v = tmp_sp
    self.pc.v = self.cr[self.CREG_INT_FIRST + (i & 0xf)]

    # Turn off maskable interrupts.
    self.cr[self.CREG_INT_CONTROL] &= 0xfffffffe
    return True

  def load_memory_from_file(self, addr, name):
    """Loads up to 64KB of data from a file into RAM.
    """
    with open(name, "rb") as f:
      # Read at most the size of RAM.
      data = f.read(64 * 1024)
    return self.mem.store_many(addr, data)

  def run_single_step(self):
    # If there is any interrupt on the queue, we need to know about it now.
    if self._process_interrupt_queue() is False:
      # Something failed hard.
      return

    # Check if there is anything in the defered queue. If so, process it now.

    while self.defered_queue:
      action = self.defered_queue.pop()
      action()

    # Normal execution.
    opcode = self.mem.fetch_byte(self.pc.v)
    if opcode is None:
      self.interrupt(self.INT_MEMORY_ERROR)
      return

    if opcode not in self.opcodes:
      self.interrupt(self.INT_GENERAL_ERROR)
      return

    length = self.opcodes[opcode][self.INSTR_LENGTH]
    argument_bytes = self.mem.fetch_many(self.pc.v + 1, length)
    if argument_bytes is None:
      self.interrupt(self.INT_MEMORY_ERROR)
      return

    handler = self.opcodes[opcode][self.INSTR_HANDLER]
    # Uncomment this line to get a dump of executed instructions.
    #print("%.4x: %s\t%s" % (self.pc.v, 
    #                        handler.func_name, 
    #                        str(argument_bytes).encode("hex")))
    self.pc.v += 1 + length
    handler(self, argument_bytes)


  def run(self):
    while not self.terminated:
      self.run_single_step()

    self.dev_console.terminate()
    self.dev_pit.terminate()

    # Simple (though ugly) method to make sure all threads exit.
    os._exit(0)

__all__ = [VMInstance]

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print "usage: vm.py <filename>"
    sys.exit(1)

  vm = VMInstance()
  vm.load_memory_from_file(0, sys.argv[1])
  vm.run()
