//CSF Assignment 4
//Name: Yuheng Shi
//Email: yshi58@jhu.edu
//Name: Yuntao Li
//Email: yli346@jhu.edu
//
// The tile plugin generates an image containing an N x N arrangement of tiles, //each tile being a smaller version of the original image, and the overall resul//t image having the same dimensions as the original image. It takes one command// line parameter, an integer specifying the tiling factor N.
//
#include <stdio.h>
#include <stdlib.h>
#include "image_plugin.h"

struct Arguments {
  // the tiling factor N
	int factor;
};

//
// Get the plugin name as a NUL-terminated character string.
//
const char *get_plugin_name(void) {
	return "tile";
}

//
// Get a one-sentence description of what the plugin does
// as a NUL-terminated character string.
//
const char *get_plugin_desc(void) {
	return "tile source image in an NxN arrangement";
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
	if (num_args != 1||atoi(args[0])<1) {
		return NULL;
	}
	struct Arguments *arg = calloc(1, sizeof(struct Arguments));
	  arg->factor = atoi(args[0]);
	return arg;
}

//convert the coordinate of source picture to the coordinate of compressed version
static int convert_coordinate(int coordinate, int switch_coordinate, int tileSize, int factor){
  int source;
	if(coordinate >= switch_coordinate) {
	  source = ((coordinate - switch_coordinate) % (tileSize)) * factor;
        } else{
          source = (coordinate % (tileSize + 1)) * factor;
        }
	return source;
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
    int factor = args->factor;

    // Allocate a result Image
    struct Image *out = img_create(source->width, source->height);
    if (!out) {
        printf("Memory allocation fails.\n");
        free(args);
        return NULL;
    }
    out->width = source->width;
    out->height = source->height;
    
    int tileWidth = source->width / factor;
    int tileHeight = source->height / factor;

    // if x-coordinate (j) >= switchX, then it's in a smaller tile (with no excess)
    int switchX = (source->width - tileWidth * factor) * (tileWidth + 1);

    // if y-coordinate (i) >= switchY, then it's in a smaller tile (with no excess)
    int switchY = (source->height - tileHeight * factor) * (tileHeight + 1);
    
    // sourceJ, sourceI represent the coordinates that (j,i) on output image corresponds to
    int sourceI, sourceJ;
    for (int i = 0; i < (int)source->height; i++) {
      for (int j = 0; j < (int)source->width; j++){
	sourceI = convert_coordinate(i, switchY, tileHeight, factor);
	sourceJ = convert_coordinate(j, switchX, tileWidth, factor);
	out->data[i*source->width+j] = source->data[sourceI*source->width+sourceJ];
      }
    }
    free(args);
    return out;
}
