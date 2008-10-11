#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK_SIZE 4096

int fappend (const char *src, const char *dst)
{
    int hsrc, hdst, nreads, nwrites;
    char buf[BLOCK_SIZE];
    
    if ((hsrc = open (src, O_RDONLY)) == -1)
	return -1;

    if ((hdst = open (dst, O_APPEND | O_RDWR)) == -1)
	return -1;

    while ((nreads = read(hsrc, buf, BLOCK_SIZE)) > 0) nwrites = write(hdst, buf, nreads);
	
    nreads = close(hsrc);
    nwrites = close(hdst);
    
    return ((nreads == -1) || (nwrites == -1)) ? -1 : 0;
}
