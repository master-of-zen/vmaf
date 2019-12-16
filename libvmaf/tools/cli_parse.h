#ifndef __VMAF_CLI_PARSE_H__
#define __VMAF_CLI_PARSE_H__

#include "libvmaf/libvmaf.rc.h"

typedef struct {
    char *y4m_path_ref, *y4m_path_dist;
    char *model_path;
    enum VmafLogLevel log_level;
    unsigned n_threads;
} CLISettings;

void cli_parse(const int argc, char *const *const argv,
               CLISettings *const settings);

#endif /* __VMAF_CLI_PARSE_H__ */
