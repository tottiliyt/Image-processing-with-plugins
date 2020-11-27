//CSF Assignment 4
//Name: Yuheng Shi
//Email: yshi58@jhu.edu
//Name: Yuntao Li
//Email: yli346@jhu.edu
//
// The expose plugin changes all red/green/blue color component values by a specified factor.
//
#include <stdio.h>
#include <stdlib.h>
#include "image_plugin.h"

struct Arguments {
  //A floating point value to use as the factor. The factor must not be negative.
	float factor;
};

//
// Get the plugin name as a NUL-terminated character string.
//
const char *get_plugin_name(void) {
	return "expose";
}

//
// Get a one-sentence description of what the plugin does
// as a NUL-terminated character string.
//
const char *get_plugin_desc(void) {
	return "adjust the intensity of all pixels";
}

//
// Parse the plugin's command line arguments.
// num_args indicates how many command line arguments are being passed
// to the plugin, args is an array of pointers to the command line
// arguments.
//
// Returns NULL if the command line arguments are invalid; otherwise
// returns a pointer to "argument data" which will be passed to the
// plugin's transform_image function.
//
void *parse_arguments(int num_args, char *args[]) {

	if (num_args != 1||atof(args[0])<0) {
		return NULL;
	}

	struct Arguments *arg = calloc(1, sizeof(struct Arguments));
	  arg->factor = atof(args[0]);
	return arg;
}

// Helper function to change pixel's color
static uint8_t change_color(uint8_t color, float factor){
  	uint32_t newcolor = (uint32_t)color;
	newcolor *= factor;
	if(newcolor > 255){
	  color = 255;
	}
	else{
	  color = (uint8_t)newcolor;
	}
	return color;
}

// Helper function to swap the blue and green color component values.
static uint32_t change_expose(uint32_t pix, float factor) {
	uint8_t r, g, b, a;
	img_unpack_pixel(pix, &r, &g, &b, &a);
	r = change_color(r, factor);
	g = change_color(g, factor);
	b = change_color(b, factor);
	return img_pack_pixel(r, g, b, a);
}

//
// Transform a source Image.
// The arg_data parameter is the pointer returned by the plugin's
// parse_arguments function.
//
// Returns a pointer to the result Image, or NULL if the plugin
// could not generate a result image for some reason.
//
// This function should free any memory allocated for arg_data
// by the previous call to parse_arguments.
//
struct Image *transform_image(struct Image *source, void *arg_data) {
  struct Arguments *args = arg_data;
	//load factor from arguments
	float factor = args->factor;
	
	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
        printf("Memory allocation fails.\n");
		free(args);
		return NULL;
	}
    out->width = source->width;
    out->height = source->height;

	unsigned num_pixels = source->width * source->height;
	for (unsigned i = 0; i < num_pixels; i++) {
	  out->data[i] = change_expose(source->data[i], factor);
	}

	free(args);

	return out;
}
