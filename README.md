# OsPE32PlusKernelEfiLoader
UEFI loader for PE32+ native binaries. Using with EFI Toolkit

Contains code for:
- Opening known target volume;
- Opening Kernel.bin by known location;
- Parsing kernel file for correct MZ\PE32+ headers;
- Loading kernel file into predefined physical address, mapping image sections and yielding control.

Assumes that Kernel.bin implements all security functions.

Actual tasks:
- Add executable image relocation code not to use hard-coded image base;
- Add ELF sructures;
- Add code for load ELF image.
