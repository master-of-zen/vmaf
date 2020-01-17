#ifndef __VMAF_MODEL_H__
#define __VMAF_MODEL_H__

typedef struct VmafModel VmafModel;

enum VmafModelFlags {
    VMAF_MODEL_ENABLE_NONE = (1 << 0),
    VMAF_MODEL_ENABLE_CLIP = (1 << 1),
    VMAF_MODEL_ENABLE_TRANSFORM = (1 << 2),
    VMAF_MODEL_ENABLE_CONFIDENCE_INTERVAL = (1 << 3),
};

int vmaf_model_load_from_path(VmafModel **model, const char *path, const char *name, int flag);
void vmaf_model_destroy(VmafModel *model);

#endif /* __VMAF_MODEL_H__ */
