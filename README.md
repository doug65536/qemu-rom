# qemu-rom
Simple QEMU -bios ROM that initializes PCI and dispi framebuffers
on all attached PCI-VGA

A cross compiler should be used to build this. The "elf" target cross compiler
would work. Anything except system compilers. System compilers are patched
by distributions to force some security related things into the compilation
process, which interfere with programs that need precise control over the 
compilation and linking process.
