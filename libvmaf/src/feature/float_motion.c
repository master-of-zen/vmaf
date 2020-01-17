#include <errno.h>
#include <math.h>
#include <string.h>

#include "common/convolution.h"
#include "feature_collector.h"
#include "feature_extractor.h"
#include "motion.h"
#include "motion_tools.h"

#include "picture_copy.h"

typedef struct MotionState {
    size_t float_stride;
    float *ref;
    float *tmp;
    float *blur[3];
} MotionState;

static int init(VmafFeatureExtractor *fex, enum VmafPixelFormat pix_fmt,
                unsigned bpc, unsigned w, unsigned h)
{
    MotionState *s = fex->priv;

    s->float_stride = sizeof(float) * w;
    s->ref = malloc(s->float_stride * h);
    s->tmp = malloc(s->float_stride * h);
    s->blur[0] = malloc(s->float_stride * h);
    s->blur[1] = malloc(s->float_stride * h);
    s->blur[2] = malloc(s->float_stride * h);

    return 0;
}

static int extract(VmafFeatureExtractor *fex,
                   VmafPicture *ref_pic, VmafPicture *dist_pic,
                   unsigned index, VmafFeatureCollector *feature_collector)
{
    MotionState *s = fex->priv;
    int err = 0;

    unsigned prev_i = (index + 2) % 3;
    unsigned this_i = (index + 0) % 3;
    unsigned next_i = (index + 1) % 3;

    picture_copy(s->ref, ref_pic);
    convolution_f32_c_s(FILTER_5_s, 5, s->ref, s->blur[this_i], s->tmp,
                        ref_pic->w[0], ref_pic->h[0],
                        s->float_stride / sizeof(float),
                        s->float_stride / sizeof(float));
    if (index == 0) return 0;

    double score;
    err = compute_motion(s->blur[prev_i], s->blur[this_i],
                         ref_pic->w[0], ref_pic->h[0],
                         s->float_stride, s->float_stride, &score);
    if (err) return err;

    double score2;
    err = compute_motion(s->blur[this_i], s->blur[next_i],
                         ref_pic->w[0], ref_pic->h[0],
                         s->float_stride, s->float_stride, &score2);
    if (err) return err;
    score2 = score2 < score ? score2 : score;

    err = vmaf_feature_collector_append(feature_collector, "float_motion",
                                        score, index);
    if (err) return err;

    err = vmaf_feature_collector_append(feature_collector, "float_motion2",
                                        score2, index - 1);
    if (err) return err;
    return 0;
}

static int close(VmafFeatureExtractor *fex)
{
    MotionState *s = fex->priv;

    if (s->ref) free(s->ref);
    if (s->blur[0]) free(s->blur[0]);
    if (s->blur[1]) free(s->blur[1]);
    if (s->blur[2]) free(s->blur[2]);
    if (s->tmp) free(s->tmp);

    return 0;
}

VmafFeatureExtractor vmaf_fex_float_motion = {
    .name = "float_motion",
    .init = init,
    .extract = extract,
    .close = close,
    .priv_size = sizeof(MotionState),
};
