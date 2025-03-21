#include <stdio.h>
#include "synap.h"

#define format(x) printf("define(`" #x "', `0x%08x')\n", (unsigned int) x)

int main() {

    printf("# generated by synap/kernel/src/generate_selinux_ioctl_defines.c\n");

    format(SYNAP_SET_NETWORK_INPUT);
    format(SYNAP_SET_NETWORK_OUTPUT);
    format(SYNAP_CREATE_IO_BUFFER_FROM_DMABUF);
    format(SYNAP_CREATE_SECURE_IO_BUFFER_FROM_DMABUF);
    format(SYNAP_CREATE_IO_BUFFER_FROM_MEM_ID);
    format(SYNAP_DESTROY_IO_BUFFER);
    format(SYNAP_ATTACH_IO_BUFFER);
    format(SYNAP_DETACH_IO_BUFFER);
    format(SYNAP_RUN_NETWORK);
    format(SYNAP_CREATE_NETWORK);
    format(SYNAP_DESTROY_NETWORK);
    format(SYNAP_LOCK_HARDWARE);
    format(SYNAP_UNLOCK_HARDWARE);
    format(SYNAP_QUERY_HARDWARE_LOCK);
    format(SYNAP_CREATE_IO_BUFFER);

}
