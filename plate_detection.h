#include "sod/sod.h"

static sod_img create_and_save_binary_img(sod_img grayscale_image,
                                        const char *save_dir,
                                        float thresh)
{
    /* TODO: find best val for thresh */
	sod_img binary_image = sod_threshold_image(grayscale_image, thresh);
    sod_img_save_as_png(binary_image, save_dir);

    return binary_image;
}
