#include <errno.h>
#include <stdlib.h>

#include "feature/feature_collector.h"
#include "libvmaf/libvmaf.rc.h"

struct VmafContext {
    VmafConfiguration cfg;
    VmafFeatureCollector *feature_collector;
};

void vmaf_default_configuration(VmafConfiguration *cfg)
{
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->log_level = 0;
    cfg->n_threads = 1;
}

int vmaf_init(VmafContext **vmaf, VmafConfiguration cfg)
{
    if (!vmaf) return -EINVAL;

    int err = 0;

    VmafContext *const v = *vmaf = malloc(sizeof(*v));
    if (!v) goto fail;
    v->cfg = cfg;
    err = vmaf_feature_collector_init(&(v->feature_collector));
    if (err) goto free_v;

    return 0;

free_v:
    free(v);
fail:
    return -ENOMEM;
}

int vmaf_close(VmafContext *vmaf)
{
    if (!vmaf) return -EINVAL;

    vmaf_feature_collector_destroy(vmaf->feature_collector);
    free(vmaf);

    return 0;
}

int vmaf_use_features_from_model(VmafContext *vmaf, VmafModel model)
{
    return 0;
}

int vmaf_use_feature(VmafContext *vmaf, const char *feature_name)
{
    return 0;
}

int vmaf_read_pictures(VmafContext *vmaf, VmafPicture *ref, VmafPicture *dist)
{
    return 0;
}

int vmaf_score_single(VmafContext *vmaf, VmafModel model, VmafScore *score,
                      unsigned index)
{
    return 0;
}

int vmaf_score_pooled(VmafContext *vmaf, VmafModel model,
                      enum VmafPoolingMethod, VmafScore *score)
{
    return 0;
}
