//#include <stddef.h>
//#include "dispi.h"

//extern char ___image_end[];
//extern char ___image_vma[];
//extern char ___image_lma[];
//extern char ___bss_st[];
//extern char ___bss_en[];

//extern "C"
//int main();

//extern "C"
//__attribute__((__optimize__("O2"), __section__(".text.entry")))
//void entry()
//{
//    unsigned *src = (unsigned*)___image_lma;
//    unsigned *dst = (unsigned*)___image_vma;
//    size_t sz = (unsigned*)___image_end - dst;
//    while (sz--)
//        *dst++ = *src++;
    
//    dst = (unsigned*)___bss_st;
//    sz = (unsigned*)___bss_en - dst;
//    while (sz--)
//        dst = 0;
    
//    main();
//}
