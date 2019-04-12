/*
 * Compile this file together with the SOD embedded source code to generate
 * the executable. For example:
 * gcc sod/sod.c plate_detection.c -lm -Ofast -march=native -Wall -std=c99 -o plate_detection
 *  
 * Make sure you have the latest release of SOD from:
 * https://pixlab.io/downloads
 */

#include <stdio.h>
#include "sod/sod.h"
#include "plate_detection.h"

#define REPEAT_DILATION 2

/*
 * License Plate Detection without deep-learning. Only image processing code.
 */

static int filter_cb(int width, int height)
{
	/* A filter callback invoked by the blob routine each time
	 * a potential blob region is identified.
	 * We use the `width` and `height` parameters supplied
	 * to discard regions of non interest (i.e. too big or too small).
	 */
    if (((width > 300 && height > 200) || (width < 25 || height < 25)) || 
        (width/height < 2.7 || width/height > 3.3)) {
	    /* Ignore small or big boxes (You should take in consideration
		 * U.S plate size here and adjust accordingly).
		 */
		return 0; /* Discarded region */
	}
    
	return 1; /* Accepted region */
}

int main(int argc, char *argv[])
{
	/* Input image (pass a path, deffault is ./plate.jpg) */
	const char *input_image_path = argc > 1 ? argv[1] : "./plate.jpg";
	/* Output image path */
	const char *out_image_path = argc > 2 ? argv[2] : "./out_plate.png";
	/* Grayscale image path */
    const char *grayscale_image_path = "./grayscale_plate.png";
    /* Binary image path */ 
    const char *binary_image_path = "./binary_plate.png";
    /* Canny edge image path */
    const char *canny_image_path = "./canny_plate.png";
    /* Dilate image path */
    const char *dilate_image_path = "./dilate_plate.png";
    /* Cropped image path */
    const char *cropped_image_path = "./cropped_plate.png";

    /* Load the input image in the grayscale colorspace */
	sod_img grayscale_image = sod_img_load_grayscale(input_image_path);
	
    if (grayscale_image.data == 0) {
		/* Invalid path, unsupported format, memory failure, etc. */
		puts("Cannot load input image...exiting");
		return 0;
	}
    /* Save grayscale image */
    sod_img_save_as_png(grayscale_image, grayscale_image_path);

	/* 
     * A full color copy of the input image so we can draw 
     * rose boxes marking the plate in question if any.
	 */
	sod_img copy_image = sod_img_load_color(input_image_path);
    
    /* Obtain a binary image */
    sod_img binary_image = create_and_save_binary_img(grayscale_image, binary_image_path, 0.1);

    /* Perform Canny edge detection next which is a mandatory step  */
	sod_img canny_image = sod_canny_edge_image(binary_image, 1); /* Reduce noise */
	sod_img_save_as_png(canny_image, canny_image_path);

    /*
	 * Dilate the image say 12 times but you should experiment
	 * with different values for best results which depend
	 * on the quality of the input image/frame. */
	sod_img dilate_image = sod_dilate_image(canny_image, REPEAT_DILATION);
    sod_img_save_as_png(dilate_image, dilate_image_path);

	/* Perform connected component labeling or blob detection
	 * now on the binary, canny edged, Gaussian noise reduced and
	 * finally dilated image using our filter callback that should
	 * discard small or large rectangle areas.
	 */
	sod_box *box = 0;
	int i, nbox;
	sod_image_find_blobs(dilate_image, &box, &nbox, filter_cb);

    if (nbox < 1) {
        printf("Can not find license plate.\n");
        return 0;
    }

    /* Print box fields */
    printf("confidence threshold: %f\n", box[0].score);
    printf("x: %d\n", box[0].x);
    printf("y: %d\n", box[0].y);
    printf("width: %d\n", box[0].w);
    printf("height: %d\n", box[0].h);

    /* Crop license plate */
    sod_img cropped_image = sod_crop_image(copy_image, box[0].x, box[0].y, box[0].w, box[0].h);

    /* Save cropped image */
    sod_img_save_as_png(cropped_image, cropped_image_path);

	/* Draw a box on each potential plate coordinates */
	for (i = 0; i < nbox; i++) {
		sod_image_draw_bbox_width(copy_image, box[i], 5, 255., 0, 225.); // rose box
	}
	sod_image_blob_boxes_release(box);

	/* Finally save the output image to the specified path */
	sod_img_save_as_png(copy_image, out_image_path);
    
    /* Cleanup */
	sod_free_image(grayscale_image);
	sod_free_image(canny_image);
	sod_free_image(binary_image);
	sod_free_image(dilate_image);
	sod_free_image(copy_image);
	sod_free_image(cropped_image);

    return 0;
}
