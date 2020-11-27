//CSF Assignment 4
//Name: Yuheng Shi
//Email: yshi58@jhu.edu
//Name: Yuntao Li
//Email: yli346@jhu.edu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include "pnglite.h"
#include "image.h"

//data structure for saving handle pointer and function pointers
struct Plugin {
    void *handle;
    const char *(*get_plugin_name)(void);
    const char *(*get_plugin_desc)(void);
    void *(*parse_arguments)(int num_args, char *args[]);
    struct Image *(*transform_image)(struct Image *source, void *arg_data);
};

//print usage information
void printUsageInfo(){
    printf("Usage: imgproc <command> [<command args...>]\nCommands are:\n  list\n  exec <plugin> <input img> <output img> [<plugin args...>]\n");
}

//format the command line arguments in argv to arguments in new_argv for plugin use
void get_new_argv(int argc, char** argv, char** new_argv){
    for (int i = 5; i < argc; i++){
        new_argv[i - 5] = argv[i];
    }
}

//get the name of directory where plugin is saved. Default is subdirectory "plugins" of running directory
const char* get_plugin_dir() {
    const char* dir_name;
    if ((dir_name = getenv("PLUGIN_DIR")) == NULL) {
        dir_name = "./plugins";
    }
    return dir_name;
}

//given a pointer to a plugin whose handle is already found, store pointers to its functions
int load_functions(struct Plugin* plugin){
    if ((*(void **)(&plugin->get_plugin_name) = dlsym(plugin->handle, "get_plugin_name")) == NULL) {
        printf("Error: get_plugin_name can't be found in plugin\n");
        return 1;
    }
    if ((*(void **)(&plugin->get_plugin_desc) = dlsym(plugin->handle, "get_plugin_desc")) == NULL) {
        printf("Error: get_plugin_desc can't be found in plugin\n");
        return 1;
    }
    
    if ((*(void **)(&plugin->parse_arguments) = dlsym(plugin->handle, "parse_arguments")) == NULL) {
        printf("Error: parse_arguments can't be found in plugin\n");
        return 1;
    }
    if ((*(void **)(&plugin->transform_image) = dlsym(plugin->handle, "transform_image")) == NULL) {
        printf("Error: transform_image can't be found in plugin\n");
        return 1;
    }
    return 0;
}

//fill the array of plugins using dynamic allocation, so that we have access to pointers to their functions
// and can use the functions individually without loading the whole plugin file
int load_plugins(int* num_plugin, struct Plugin* plugins, const char* plugin_dir_name, DIR* plugin_dir){
    char temp_path[100];
    struct dirent *entry;
    while((entry=readdir(plugin_dir)) != NULL) {
        const char* name = entry->d_name;
        if (name[strlen(name)-3] == '.' && name[strlen(name)-2] == 's' && name[strlen(name)-1] == 'o'){
        strcpy(temp_path,plugin_dir_name);
        strcat(temp_path,"/");
        strcat(temp_path,name); // form the directory of plugin. For example: ./plugins/swapbg.so
        plugins[*num_plugin].handle = dlopen(temp_path, RTLD_LAZY);
        if (!plugins[*num_plugin].handle) {
            printf("Error: an image plugin can't be loaded\n");
            return 1;
        }
        if (load_functions(&plugins[*num_plugin]) == 1) return 1;
        *num_plugin = *num_plugin+1;
        }
    }
    closedir(plugin_dir);
    return 0;
}

// print the list of plugins and their descriptions
void print_list(int num_plugin, struct Plugin* plugins){
    printf("Loaded %d plugin(s)\n", num_plugin);
    for (int i = 0; i < num_plugin; i++) {
        printf("%s: %s\n", plugins[i].get_plugin_name(), plugins[i].get_plugin_desc());
    }
}

//close all dynamically loaded files and free all dynamically allocated memory
void free_all(int num_plugin, struct Plugin* plugins, struct Image* input_image, struct Image* output_image, char** new_argv){
    for (int i = 0; i < num_plugin; i++){
        dlclose(plugins[i].handle);
    }
    free(input_image->data);
    free(input_image);
    free(output_image->data);
    free(output_image);
    free(new_argv);
    free(plugins);
}

//execute a single plugin given input file name, output file name, and optional arguments for plugin
int execute_plugin(int num_plugin, struct Plugin* plugins, char** new_argv, char** argv, int argc){
    for (int i = 0; i < num_plugin; i++) {
        if(!strcmp(argv[2], plugins[i].get_plugin_name())) {
            struct Image* input_image;
            struct Image* output_image;
            if((input_image = img_read_png(argv[3])) == NULL) {
                printf("Error: Invalid name or format of input image.\n");
                printUsageInfo();
                return 1;
            }
            void* arguments = plugins[i].parse_arguments(argc - 5, new_argv);
            if (arguments == NULL) {
                printf("Error: Invalid format of command line arguments.\n");
                printUsageInfo();
                return 1;
            }
            if ((output_image = plugins[i].transform_image(input_image, arguments)) == NULL){
                printf("Error: Invalid format of input image.\n");
                return 1;
            }
            if (img_write_png(output_image, argv[4]) == 0) {
                printf("Error: Write Failure.\n");
                return 1;
            };
            free_all(num_plugin, plugins, input_image, output_image, new_argv);
            return 0;
        }
    }
    printf("Error: Invalid name of plugin.\n");
    printUsageInfo();
    return 1;
}

int main(int argc, char* argv[]){
    const char* plugin_dir_name = get_plugin_dir();
    DIR *plugin_dir = opendir(plugin_dir_name);
    if(plugin_dir == NULL) {
        printf("Error: Unable to read directory\n");
        return 1;
    }
    int num_plugin = 0;
    struct Plugin* plugins = calloc(1, 5 * sizeof(struct Plugin)); //use calloc to avoid uninitialized memory, because we may have less than 5 plugins
    if (plugins == NULL) {
        printf("Memory allocation failure for plugins\n");
        return 1;
    }
    if (load_plugins(&num_plugin, plugins, plugin_dir_name, plugin_dir) == 1) return 1;
    if (argc == 2 && !strcmp(argv[1], "list")) {
        print_list(num_plugin, plugins);
        return 0;
    } else if (argc >= 5 && !strcmp(argv[1], "exec")) {
        char** new_argv = malloc(sizeof(argv[0]) * (argc - 5)); //new_argv is for saving (optional) arguments directly given to plugin
        if(new_argv == NULL){
            printf("Memory allocation failure for new_argv\n");
            return 1;
        }
        get_new_argv(argc, argv, new_argv);
        return execute_plugin(num_plugin, plugins, new_argv, argv, argc);
    } else{
        printf("Error: Invalid format of command line arguments.\n");
        printUsageInfo();
        return 1;
    }
}


