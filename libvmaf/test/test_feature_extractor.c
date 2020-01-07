#include <stdint.h>

#include "feature/feature_extractor.h"
#include "test.h"
#include "picture.h"
#include "libvmaf/picture.h"

static char *test_get_feature_extractor_by_name_and_feature_name()
{
    VmafFeatureExtractor *fex1, *fex2 = NULL;
    fex1 = get_feature_extractor_by_name("psnr");
    mu_assert("problem during get_feature_extractor_by_name", fex1);
    fex2 = get_feature_extractor_by_name("psnr");
    mu_assert("problem during get_feature_extractor_by_name", fex2);
    //mu_assert("fex1 and fex2 should not be duplicates", fex1 != fex2);
    fex1 = get_feature_extractor_by_feature_name("");
    mu_assert("problem during get_feature_extractor_by_feature_name", !fex1);

    return NULL;
}

char *run_tests()
{
    mu_run_test(test_get_feature_extractor_by_name_and_feature_name);
    return NULL;
}
