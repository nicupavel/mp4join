#include <string.h>
#include <stdio.h>
#include "config.h"

int parse_cmd_line(int argc, char **argv, config *params)
{
	int i = 0;

	if (argc < 4) return -1;
	
	strncpy(params->infile1, argv[i + 1], MAX_PATH_LEN);
	strncpy(params->infile2, argv[i + 2], MAX_PATH_LEN);
	strncpy(params->outfile, argv[i + 3], MAX_PATH_LEN);
	
	return 1;
}
