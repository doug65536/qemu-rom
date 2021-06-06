#include <stddef.h>
#include <stdint.h>
#include "arch/pci.h"
#include "assert.h"
#include "debug.h"
#include "dispi.h"


int main()
{
    //*(int*)0xf00ff00f = 42;
    pci_init();
    
    bool dispi_ok = dispi_init();
    
    if (!dispi_ok)
        return 0;
    
    size_t display_count = dispi_display_count();
    
    for (size_t i = 0; i < display_count; ++i) {
        int width = 1024;
        int height = 768;

        dispi_set_mode(i, width, height, 32);

        dispi_fill_screen(i);
    }
    
    return 0;
}
