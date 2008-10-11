#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BLOCK_SIZE 4096

/* Concatenates 2 files */
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


/* Gets the path from a filename with full path */
void get_path(char *filename, char *path)
{
    int i;

    i = strlen(filename);
    strcpy (path, filename);
    while (--i >= 0)
        if (path[i] == '/') {
            path[i+1] = '\0';
            break;
        }
}
/* Gets the filename only without extension from a filename with full path */
char * get_filename(char *file)
{
    int i;
    unsigned int n = 0;
    char *buffer;
    
    i = strlen(file);
    buffer = (char *) malloc (sizeof(char) * i +1);
    
    while (--i >= 0){
       if (file[i] == '.'){
            n = 0;
            continue;
        }   
        if (file[i] == '/') 
            break;
        n++;
    }
    strncpy(buffer, file + i + 1, n);
    buffer[n] = '\0';
    
    return buffer;
}
/* Gets the filename only without extension from a filename with full path  --neds rewrite*/
static void get_filename_inplace(char *filename, char **file)
{
    int i;

    i = strlen(filename);
    strcpy (*file, filename);
    while (--i >= 0){
        
        if ((*file)[i] == '.'){
            (*file)[i] = '\0';
        }   
        
        if ((*file)[i] == '/') {
            *file = &(*file)[i+1];
        break;
        }
    }
}

