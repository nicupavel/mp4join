#define MAX_PATH_LEN 256

typedef struct _config_s config;
struct _config_s {
    char infile1[MAX_PATH_LEN + 1];
    char infile2[MAX_PATH_LEN + 1];
    char outfile[MAX_PATH_LEN + 1];
};


int parse_cmd_line(int argc, char **argv, config *params);
