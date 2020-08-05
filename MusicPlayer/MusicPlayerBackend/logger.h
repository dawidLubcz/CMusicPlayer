#ifndef LOGGER
#define LOGGER

//#define NO_LOGS

#include <stdio.h>
#include <stdarg.h>

#define PRINT_PREFIX "MP:Default: "

#define PRINT_INF(DESC, ...) PRINT_EX("[INF]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)
#define PRINT_ERR(DESC, ...) PRINT_EX("[ERR]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)
#define PRINT_WRN(DESC, ...) PRINT_EX("[WRN]: " PRINT_PREFIX DESC "\n", ##__VA_ARGS__)
#define PRINT_OUT(DESC, ...) printf("[OUT]: " DESC "\n", ##__VA_ARGS__)
#define PRINT_ENTRY printf("[INF]: " PRINT_PREFIX " %s\n", __func__)
#define PRINT_ENTRY_EX(DESC, ...) printf("[INF]: " PRINT_PREFIX " %s " DESC "\n" , __func__, ##__VA_ARGS__)

static inline void PRINT_EX(const char a_acArg[], ...)
{
    va_list args;
    va_start(args, a_acArg);
#ifndef NO_LOGS
    vprintf(a_acArg, args);
#endif
    va_end(args);
    return;
}

#endif // LOGGER

