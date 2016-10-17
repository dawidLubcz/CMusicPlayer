#ifndef LOGGER
#define LOGGER

#define DEBUG

#include <stdio.h>
#include <stdarg.h>

#define PRINT_PREFIX "MP:Default: "

#define PRINT_INF(DESC, ...) PRINT_EX("[INF]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)
#define PRINT_ERR(DESC, ...) PRINT_EX("[ERR]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)
#define PRINT_WRN(DESC, ...) PRINT_EX("[WRN]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)

static inline void PRINT_EX(const char a_acArg[], ...)
{
#ifdef DEBUG
    va_list args;
    va_start(args, a_acArg);
    vprintf(a_acArg, args);
    va_end(args);
#endif
    return;
}

#endif // LOGGER

