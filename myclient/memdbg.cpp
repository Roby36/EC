
#include "memdbg.h"

int64_t get_sp() {
    int64_t res = 0;
    asm volatile (
        "mov %x[result], sp"
        : [result] "=r" (res)
    );
    printf("Current sp: %llx\n", res);
    return res;
}

int32_t write_byte(int64_t address, int32_t content) {
    int32_t res = 0;
    asm volatile (
        "mov x3, %x[input_content]" "\n"
        "str x3, [%x[input_address]]" "\n"
        "ldr %x[result], [%x[input_address]]"
        : [result] "=r" (res)
        : [input_address] "r" (address), [input_content] "r" (content)
        : "x3"
    );
    return res;
}
