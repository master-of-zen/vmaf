#ifndef __VMAF_H__
#define __VMAF_H__

#include "libvmaf/model.h"
#include "libvmaf/picture.h"

enum VmafLogLevel {
    VMAF_LOG_LEVEL_NONE,
};

typedef struct {
    enum VmafLogLevel log_level;
    unsigned n_threads;
} VmafConfiguration;

enum VmafPoolingMethod {
    VMAF_POOL_METHOD_UNKNOWN = 0,
    VMAF_POOL_METHOD_MIN,
    VMAF_POOL_METHOD_MEAN,
    VMAF_POOL_METHOD_HARMONIC_MEAN,
};

typedef struct {
    double score;
} VmafScore;

typedef struct VmafContext VmafContext;

void vmaf_default_configuration(VmafConfiguration *cfg);

int vmaf_init(VmafContext **vmaf, VmafConfiguration cfg);

int vmaf_use_features_from_model(VmafContext *vmaf, VmafModel model);

int vmaf_use_feature(VmafContext *vmaf, const char *feature_name);

int vmaf_import_feature_score(VmafContext *vmaf, char *feature_name,
                              double value, unsigned index);

int vmaf_read_pictures(VmafContext *vmaf, VmafPicture *ref, VmafPicture *dist);

int vmaf_score_at_index(VmafContext *vmaf, VmafModel model, VmafScore *score,
                        unsigned index);

int vmaf_score_pooled(VmafContext *vmaf, VmafModel model,
                      enum VmafPoolingMethod pool_method, VmafScore *score);

int vmaf_close(VmafContext *vmaf);

#endif /* __VMAF_H__ */
