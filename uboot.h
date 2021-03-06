#define IH_OS_INVALID       0
#define IH_OS_OPENBSD       1 
#define IH_OS_NETBSD        2
#define IH_OS_FREEBSD       3 
#define IH_OS_4_4BSD        4
#define IH_OS_LINUX         5
#define IH_OS_SVR4          6 
#define IH_OS_ESIX          7 
#define IH_OS_SOLARIS       8 
#define IH_OS_IRIX          9 
#define IH_OS_SCO           10 
#define IH_OS_DELL          11
#define IH_OS_NCR           12 
#define IH_OS_LYNXOS        13 
#define IH_OS_VXWORKS       14
#define IH_OS_PSOS          15 
#define IH_OS_QNX           16 
#define IH_OS_U_BOOT        17
#define IH_OS_RTEMS         18 
#define IH_OS_ARTOS         19 
#define IH_OS_UNITY         20 

#define IH_CPU_INVALID      0
#define IH_CPU_ALPHA        1
#define IH_CPU_ARM          2 
#define IH_CPU_I386         3
#define IH_CPU_IA64         4
#define IH_CPU_MIPS         5
#define IH_CPU_MIPS64       6
#define IH_CPU_PPC          7  
#define IH_CPU_S390         8 
#define IH_CPU_SH           9 
#define IH_CPU_SPARC        10 
#define IH_CPU_SPARC64      11
#define IH_CPU_M68K         12 
#define IH_CPU_NIOS         13 
#define IH_CPU_MICROBLAZE   14 
#define IH_CPU_NIOS2        15
#define IH_CPU_BLACKFIN     16
#define IH_CPU_AVR32        17 

#define IH_TYPE_INVALID 0 
#define IH_TYPE_STANDALONE 1 
#define IH_TYPE_KERNEL 2 
#define IH_TYPE_RAMDISK 3 
#define IH_TYPE_MULTI 4 
#define IH_TYPE_FIRMWARE 5 
#define IH_TYPE_SCRIPT 6 
#define IH_TYPE_FILESYSTEM 7 
#define IH_TYPE_FLATDT 8 
#define IH_TYPE_KERNEL_NOLOAD  14 

#define IH_COMP_NONE 0 
#define IH_COMP_GZIP 1 
#define IH_COMP_BZIP2 2 

#define IH_MAGIC 0x27051956 
#define IH_NMLEN 32 

#ifdef __ASSEMBLER__
// Everything is bigendian 
.macro be_int32 v
.byte (((\v) >> 24) & 0xFF)
.byte (((\v) >> 16) & 0xFF)
.byte (((\v) >> 8) & 0xFF)
.byte ((\v) & 0xFF)
.endm

.macro uboot_header size,base,entry,name
.section .header, "a", @progbits
.global _start
start:
be_int32 IH_MAGIC
be_int32 0  // header crc (qemu ignores it)
be_int32 0  // time
be_int32 \size
be_int32 \base
be_int32 \entry
be_int32 0  // data crc (qemu ignores it)
.byte IH_OS_U_BOOT
.byte IH_CPU_PPC
.byte IH_TYPE_KERNEL
.byte IH_COMP_NONE
name: .asciz "\name"
.space (name + 32) - .,0
.endm

#endif
