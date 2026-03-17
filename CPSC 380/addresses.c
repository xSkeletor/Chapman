#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    uint32_t l_addr;
    int page_no, offset;

    if (argc != 2) {
        printf("Usage: %s <l_addr>\n", argv[0]);
        return 1;
    }

    l_addr = (uint32_t) atoi(argv[1]);

    page_no = l_addr >> 12;
    offset = l_addr & 0x00000FFF;

    printf("The address %u contains:\n", l_addr);
    printf("page number = %d\n", page_no);
    printf("offset = %d\n", offset);

    return 0;
}