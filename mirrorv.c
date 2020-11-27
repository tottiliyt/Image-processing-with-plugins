//CSF Assignment 4
//Name: Yuheng Shi
//Email: yshi58@jhu.edu
//Name: Yuntao Li
//Email: yli346@jhu.edu
//
// The mirrorv plugin generates a mirror image of the input image, with all pixe// ls being reflected vertically. It does not take any command line parameters. 
//

#include <stdlib.h>
#include "image_plugin.h"
#include <stdio.h>

struct Arguments {
	// This plugin doesn't accept any command line arguments;
	// just define a single dummy field.
	int dummy;
};

//
// Get the plugin name as a NUL-terminated character string.
//
const char *get_plugin_name(void) {
	return "mirrorv";
}

//
// Get a one-sentence description of what the plugin does
// as a NUL-terminated character string.
//
const char *get_plugin_desc(void) {
	return "mirror image vertically";
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
    (void) args; // this is just to avoid a warning about an unused parameter

	if (num_args != 0) {
		return NULL;
	}
	return calloc(1, sizeof(struct Arguments));
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

	// Allocate a result Image
	struct Image *out = img_create(source->width, source->height);
	if (!out) {
        printf("Memory allocation fails.\n");
		free(args);
		return NULL;
	}
	out->width = source->width;
	out->height = source->height;

	//for an vertical flip, the pixel situated at coordinate (x, y) will b        //e situated at coordinate (x, height-1-y) in the new image.
	for (unsigned i = 0; i < source->height; i++) {
	  for (unsigned j = 0; j < source->width; j++){
	    out->data[i*source->width+j] = source->data[(source->height-1-i)*source->width+j];
	  }
	}

	free(args);

	return out;
}
