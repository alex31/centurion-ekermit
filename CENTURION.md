# E-Kermit for Centurion

The upstream branch contains Frank da Cruz's E-Kermit 1.8 protocol core.
See [COPYING](COPYING) for the revised three-clause BSD license.

The Centurion branch adapts the source for SDCC and adds a CENTOS binary
receiver on CRT1. It carries Kermit packets in printable lines and writes
to a preallocated executable file assigned to SYS001. The receiver supports
long packets and a 16-bit CRC.
