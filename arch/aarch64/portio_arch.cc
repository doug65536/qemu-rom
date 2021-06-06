#include "portio_arch.h"

// 000000003eff0000-000000003effffff;
void volatile *port_io_window = (void volatile *)0x03eff0000;
