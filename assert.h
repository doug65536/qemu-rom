#pragma once

void __assert_failed(char const *expr, char const *file, int line);

#define assert(e) while(!(e)) __assert_failed(#e, __FILE__, __LINE__)

