#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cli_parse.h"

#include <libvmaf/libvmaf.rc.h>
#include <libvmaf/model.h>

static const char short_opts[] = "r:d:m:o:x:t:f:i:n:v:";

static const struct option long_opts[] = {
    { "reference",        1, NULL, 'r' },
    { "distorted",        1, NULL, 'd' },
    { "model",            1, NULL, 'm' },
    { "output",           1, NULL, 'o' },
    { "xml",              0, NULL, 'x' },
    { "threads",          1, NULL, 't' },
    { "feature",          1, NULL, 'f' },
    { "import",           1, NULL, 'i' },
    { "no_prediction",    0, NULL, 'n' },
    { "version",          0, NULL, 'v' },
    { NULL,               0, NULL, 0 },
};

static void usage(const char *const app, const char *const reason, ...) {
    if (reason) {
        va_list args;
        va_start(args, reason);
        vfprintf(stderr, reason, args);
        va_end(args);
        fprintf(stderr, "\n\n");
    }
    fprintf(stderr, "Usage: %s [options]\n\n", app);
    fprintf(stderr, "Supported options:\n"
            " --reference/-r $path:      path to reference .y4m\n"
            " --distorted/-d $path:      path to distorted .y4m\n"
            " --model/-m $path:          path to model file\n"
            " --output/-o $path:         path to output file\n"
            " --xml/-x:                  write output file as XML (default)\n"
            " --threads/-t $unsigned:    number of threads to use\n"
            " --feature/-f $string:      additional feature\n"
            " --import/-i $path:         path to precomputed feature log\n"
            " --no_prediction/-n:        no prediction, extract features only\n"
            " --version/-v:              print version and exit\n"
           );
    exit(1);
}

static void error(const char *const app, const char *const optarg,
                  const int option, const char *const shouldbe)
{
    char optname[256];
    int n;

    for (n = 0; long_opts[n].name; n++)
        if (long_opts[n].val == option)
            break;
    assert(long_opts[n].name);
    if (long_opts[n].val < 256) {
        sprintf(optname, "-%c/--%s", long_opts[n].val, long_opts[n].name);
    } else {
        sprintf(optname, "--%s", long_opts[n].name);
    }

    usage(app, "Invalid argument \"%s\" for option %s; should be %s",
          optarg, optname, shouldbe);
}

static unsigned parse_unsigned(const char *const optarg, const int option,
                               const char *const app)
{
    char *end;
    const unsigned res = (unsigned) strtoul(optarg, &end, 0);
    if (*end || end == optarg) error(app, optarg, option, "an integer");
    return res;
}

void cli_parse(const int argc, char *const *const argv,
               CLISettings *const settings)
{
    memset(settings, 0, sizeof(*settings));
    int o;

    while ((o = getopt_long(argc, argv, short_opts, long_opts, NULL)) >= 0) {
        switch (o) {
        case 'r':
            settings->y4m_path_ref = optarg;
            break;
        case 'd':
            settings->y4m_path_dist = optarg;
            break;
        case 'o':
            settings->output_path = optarg;
            break;
        case 'x':
            settings->output_fmt = VMAF_OUTPUT_FORMAT_XML;
            break;
        case 'm':
            if (settings->model_cnt == CLI_SETTINGS_STATIC_ARRAY_LEN) {
                usage(argv[0], "A maximum of %d models is supported\n",
                      CLI_SETTINGS_STATIC_ARRAY_LEN);
            }
            /* parse model-specific flags and assign to settings */
            char *token;
            char delim[] = "=:";
            token = strtok(optarg, delim);
            bool path_set = false;
            bool name_set = false;
            /* default model flag is to disable everything and enable on demand*/
            settings->model_flag[settings->model_cnt] = VMAF_MODEL_ENABLE_NONE;
            while (token != 0) {
                if(!strcmp(token, "path")) {
                    path_set = true;
                    settings->model_path[settings->model_cnt] = strtok(0, delim);
                } else if (!strcmp(token, "name")) {
                    name_set = true;
                    settings->model_name[settings->model_cnt] = strtok(0, delim);
                } else if (!strcmp(token, "enable_clip")) {
                    settings->model_flag[settings->model_cnt] |= VMAF_MODEL_ENABLE_CLIP;
                } else if (!strcmp(token, "enable_transform")) {
                    settings->model_flag[settings->model_cnt] |= VMAF_MODEL_ENABLE_TRANSFORM;
                } else if (!strcmp(token, "enable_ci")) {
                    settings->model_flag[settings->model_cnt] |= VMAF_MODEL_ENABLE_CONFIDENCE_INTERVAL;
                } else {
                    usage(argv[0], "Unknown parameter %s for model.\n", token);
                }
                token = strtok(0, delim);
            }
            /* if model name is not set, create a unique id for this model */
            if (!name_set) {
                size_t buf_size = strlen("custom_vmaf_") + 1 * sizeof(unsigned int) + 1 * sizeof(char);
                char buf[buf_size];
                snprintf(buf, buf_size, "custom_vmaf_%u", settings->model_cnt);
                settings->model_name[settings->model_cnt] = buf;
            }
            /* path always needs to be set for each model specified */
            if (!path_set) {
                usage(argv[0], "For every model, path needs to be set.\n");
            }
            /* increment model count */
            settings->model_cnt++;
            break;
        case 'f':
            if (settings->feature_cnt == CLI_SETTINGS_STATIC_ARRAY_LEN) {
                usage(argv[0], "A maximum of %d features is supported\n",
                      CLI_SETTINGS_STATIC_ARRAY_LEN);
            }
            settings->feature[settings->feature_cnt++] = optarg;
            break;
        case 'i':
            if (settings->import_cnt == CLI_SETTINGS_STATIC_ARRAY_LEN) {
                usage(argv[0], "A maximum of %d imports is supported\n",
                      CLI_SETTINGS_STATIC_ARRAY_LEN);
            }
            settings->import_path[settings->import_cnt++] = optarg;
            break;
        case 't':
            settings->thread_cnt = parse_unsigned(optarg, 't', argv[0]);
            break;
        case 'n':
            settings->no_prediction = true;
            break;
        case 'v':
            fprintf(stderr, "%s\n", vmaf_version());
            exit(0);
        default:
            break;
        }
    }

    if (!settings->y4m_path_ref)
        usage(argv[0], "Reference .y4m (-r/--reference) is required");
    if (!settings->y4m_path_ref)
        usage(argv[0], "Distorted .y4m (-d/--distorted) is required");
    if ((settings->model_cnt == 0) && !settings->no_prediction)
        usage(argv[0], "At least one model file (-m/--model) is required");
}
