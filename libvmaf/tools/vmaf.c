#include "libvmaf/libvmaf.rc.h"

int main(int argc, char *argv[])
{
    int err = 0;
    VmafContext *vmaf;
    VmafConfiguration cfg;

    vmaf_default_configuration(&cfg);
    err = vmaf_init(&vmaf, cfg);
    if (err) return err;
    return vmaf_close(vmaf);
}
