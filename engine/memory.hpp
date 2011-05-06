#ifndef MEMORY_H_ 
#define MEMORY_H_

#include <stddef.h>                                                                                                                                          

extern void *emalloc(size_t size);

extern void *erealloc(void *ptr, size_t size);

/*  read_entire_file() function was written by Andrew Trotman for the ANT search engine
    
    Given the filename, it will read the entire contents of the file into a char*
    and is terminated by '\0'.
*/
extern char *read_entire_file(const char *filename);

extern int write_to_disk(const char *filename, const char *buffer, long length);

#endif
