# qemu-rom
Simple QEMU -bios ROM that initializes PCI and dispi framebuffers
on all attached PCI-VGA

A cross compiler should be used to build this. The "elf" target cross compiler
would work. Anything except system compilers. System compilers are patched
by distributions to force some security related things into the compilation
process, which interfere with programs that need precise control over the 
compilation and linking process.

## Instructions

    you@yourcomputer:~/somewhere-good
    $ mkdir qemu-rom
    
    you@yourcomputer:~/somewhere-good
    $ cd qemu-rom
    
    you@yourcomputer:~/somewhere-good/qemu-rom
    $ git clone https://github.com/doug65536/qemu-rom.git src
    
    you@yourcomputer:~/somewhere-good/qemu-rom
    $ cd qemu-rom
    
Source now at `~/somewhere-good/qemu-rom/src/`

## If you need toolchain(s)

Get a script you can use to quickly build a cross toolchain
    
    you@yourcomputer:~/somewhere-good/qemu-rom
    git clone https://github.com/travisg/toolchains.git

    you@yourcomputer:~/somewhere-good/qemu-rom
    $ cd toolchains

    you@yourcomputer:~/somewhere-good/qemu-rom/toolchains
    $ 
    
Build an architecture, many architecture names work. The example
command line creates `~/somewhere-good/qemu-rom/tools/aarch64-elf-Linux_x86_64` *or
something like that* (read on below).
    
    you@yourcomputer:~/somewhere-good/qemu-rom/toolchains
    $ mkdir -p $PWD/../tools && ./doit -a aarch64 -f -o $PWD/../tools
    
Notice how vague I was? We just dynamically detect it
    
    you@yourcomputer:~/somewhere-good/qemu-rom/toolchains
    $ cd ..
    
    you@yourcomputer:~/somewhere-good/qemu-rom/
    $ toolchain=$(find tools -maxdepth 1 -name aarch64-\*)
    
    you@yourcomputer:~/somewhere-good/qemu-rom/
    $ toolchain_bin=$toolchain/bin
    
    you@yourcomputer:~/somewhere-good/qemu-rom/
    $ PATH=$toolchain_bin:$PATH
    
## You have a toolchain
    
    you@yourcomputer:~/somewhere-good/qemu-rom/
    $ mkdir build-aarch64
    
    you@yourcomputer:~/somewhere-good/qemu-rom/
    $ cd build-aarch64
    
    you@yourcomputer:~/somewhere-good/qemu-rom/build-aarch64
    $ ../src/configure --host=aarch64-elf
    
    you@yourcomputer:~/somewhere-good/qemu-rom/build-aarch64
    $ make run -j$(nproc)
    
To launch vm with debugging completely disabled (no fighting over port 1234)
    
    you@yourcomputer:~/somewhere-good/qemu-rom/build-aarch64
    $ make run-nogdb -j$(nproc)
    
To launch vm for debugging
    
    you@yourcomputer:~/somewhere-good/qemu-rom/build-aarch64
    $ make debug -j$(nproc)
    
To launch toolchain gdb attached to qemu (presumably the one from `make debug`)
    
    you@yourcomputer:~/somewhere-good/qemu-rom/build-aarch64
    $ make attach
    
## QEMU configuration

You can pass additional arguments to qemu by adding `QEMUEXTRAFLAGS='-flags...'`
to the make command line arguments.

