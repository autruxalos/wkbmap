#ifndef WKBMAP_ARGS_H
#define WKBMAP_ARGS_H

typedef struct {
    char *layout;
    int help;
    int version;
} WkbConfig;

int parse_args(int argc, char **argv, WkbConfig *config);
void free_config(WkbConfig *config);
void print_help(void);
void print_version(void);

#endif
