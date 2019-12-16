#include "libvmaf/libvmaf.rc.h"

#include "cli_parse.h"

int main(int argc, char *argv[])
{
    int err = 0;

    CLISettings c;
    cli_parse(argc, argv, &c);

    VmafContext *vmaf;
    VmafConfiguration cfg;

    vmaf_default_configuration(&cfg);
    err = vmaf_init(&vmaf, cfg);
    if (err) return err;
    return vmaf_close(vmaf);
}
