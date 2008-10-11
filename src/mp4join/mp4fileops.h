#include <mpeg4ip.h>
#include <mp4.h>
#include <mp4av.h>

#define TOOLTAG "panic mpeg joiner"
#define TOOLVERSION "0.0.2"

/* joins 2 mp4 files into another */
void do_join(char *infile1, char *infile2, char *outfile);
