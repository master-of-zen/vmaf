#include <string.h>

#include "cli_parse.h"
#include "vidinput.h"

#include "libvmaf/picture.h"
#include "libvmaf/libvmaf.rc.h"

static enum VmafPixelFormat pix_fmt_map(int pf)
{
    switch (pf) {
    case PF_420:
        return VMAF_PIX_FMT_YUV420P;
    case PF_422:
        return VMAF_PIX_FMT_YUV422P;
    case PF_444:
        return VMAF_PIX_FMT_YUV444P;
    default:
        return VMAF_PIX_FMT_UNKNOWN;
    }
}

static int validate_videos(video_input *vid1, video_input *vid2)
{
    int err = 0;

    video_input_info info1, info2;
    video_input_get_info(vid1, &info1);
    video_input_get_info(vid2, &info2);

    if ((info1.frame_w != info2.frame_w) || (info1.frame_h != info2.frame_h)) {
        fprintf(stderr, "dimensions do not match: %dx%d, %dx%d\n",
                info1.frame_w, info1.frame_h, info2.frame_w, info2.frame_h);
        err++;
    }

    if (info1.pixel_fmt != info2.pixel_fmt) {
        fprintf(stderr, "pixel formats do not match: %d, %d\n",
                info1.pixel_fmt, info2.pixel_fmt);
        err++;
    }

    if (!pix_fmt_map(info1.pixel_fmt) || !pix_fmt_map(info2.pixel_fmt)) {
        fprintf(stderr, "unsupported pixel format: %d\n", info1.pixel_fmt);
        err++;
    }

    if (info1.depth != info2.depth) {
        fprintf(stderr, "bitdepths do not match: %d, %d\n",
                info1.depth, info2.depth);
        err++;
    }

    if (info1.depth < 8 || info1.depth > 16) {
        fprintf(stderr, "unsupported bitdepth: %d\n", info1.depth);
        err++;
    }

    //TODO: more validations are possible.

    return err;
}

static int fetch_picture(video_input *vid, VmafPicture *pic)
{
    int ret;
    video_input_ycbcr ycbcr;
    video_input_info info;

    ret = video_input_fetch_frame(vid, ycbcr, NULL);
    if (!ret) return ret;

    video_input_get_info(vid, &info);
    ret = vmaf_picture_alloc(pic, pix_fmt_map(info.pixel_fmt), info.depth,
                             info.frame_w, info.frame_h);
    if (ret) {
        fprintf(stderr, "problem allocating picture.\n");
        return -1;
    }

    //TODO: use a macro
    if (info.depth == 8) {
        for (unsigned i = 0; i < 3; i++) {
            uint8_t *pic_data = pic->data[i];
            uint8_t *ycbcr_data = ycbcr[i].data;
            for (unsigned j = 0; j < pic->h[i]; j++) {
                memcpy(pic_data, ycbcr_data,
                       sizeof(*ycbcr_data) * ycbcr[i].width);
            }
            pic_data += pic->stride[i];
            ycbcr_data += ycbcr[i].stride;
        }
    } else {
        for (unsigned i = 0; i < 3; i++) {
            uint16_t *pic_data = pic->data[i];
            uint16_t *ycbcr_data = (uint16_t*)ycbcr[i].data;
            for (unsigned j = 0; j < pic->h[i]; j++) {
                memcpy(pic_data, ycbcr_data,
                       sizeof(*ycbcr_data) * ycbcr[i].width);
            }
            pic_data += pic->stride[i];
            ycbcr_data += ycbcr[i].stride;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int err = 0;

    CLISettings c;
    cli_parse(argc, argv, &c);

    FILE *file_ref = fopen(c.y4m_path_ref, "r");
    if (!file_ref) {
        fprintf(stderr, "could not open file: %s\n", c.y4m_path_ref);
        return -1;
    }

    FILE *file_dist = fopen(c.y4m_path_dist, "r");
    if (!file_dist) {
        fprintf(stderr, "could not open file: %s\n", c.y4m_path_dist);
        return -1;
    }

    video_input vid_ref;
    err = video_input_open(&vid_ref, file_ref);
    if (err) {
        fprintf(stderr, "problem with y4m: %s\n", c.y4m_path_ref);
        return -1;
    }

    video_input vid_dist;
    err = video_input_open(&vid_dist, file_dist);
    if (err) {
        fprintf(stderr, "problem with y4m: %s\n", c.y4m_path_dist);
        return -1;
    }

    err = validate_videos(&vid_ref, &vid_dist);
    if (err) {
        fprintf(stderr, "videos are incompatible, %d %s.\n",
                err, err == 1 ? "problem" : "problems");
        return -1;
    }

    VmafModel *model[c.model_cnt];
    for (unsigned i = 0; i < c.model_cnt; i++) {
        err = vmaf_model_load_from_path(&model[i], c.model_path[i]);
        if (err) {
            fprintf(stderr, "problem loading model file: %s\n",
                    c.model_path[i]);
            return -1;
        }
    }

    VmafContext *vmaf;
    VmafConfiguration cfg;
    vmaf_default_configuration(&cfg);
    err = vmaf_init(&vmaf, cfg);
    if (err) {
        fprintf(stderr, "problem initializing vmaf context\n");
        return -1;
    }

    for (unsigned i = 0; i < c.feature_cnt; i++) {
        err = vmaf_use_feature(vmaf, c.feature[i]);
        if (err) {
            fprintf(stderr, "feature does not exist: %s\n", c.feature[i]);
            return -1;
        }
    }

    VmafPicture pic_ref, pic_dist;
    fetch_picture(&vid_ref, &pic_ref);
    fetch_picture(&vid_dist, &pic_dist);

    for (unsigned i = 0; i < c.model_cnt; i++)
        vmaf_model_destroy(model[i]);
    video_input_close(&vid_ref);
    video_input_close(&vid_dist);
    vmaf_close(vmaf);
    return err;
}
