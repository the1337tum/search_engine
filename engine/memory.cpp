#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>

void *emalloc(size_t size) {
	void *result = malloc(size);
	if(result == NULL){
		fprintf(stderr, "%s", "Memory allocation failure!\n");
		exit(EXIT_FAILURE);
	}
	return result;
}

void *erealloc(void *ptr, size_t size) {
	void *result = realloc(ptr, size);
	if(result == NULL){
		fprintf(stderr, "%s", "Memory allocation failure!\n");
		exit(EXIT_FAILURE);
	}
	return result;
}

char *read_entire_file(const char *filename) {
    FILE *fp;
    char *block = NULL;
    struct stat details;

    if ((fp = fopen(filename, "rb")) == NULL)
        return NULL;
    
    if (fstat(fileno(fp), &details) == 0)
        if (details.st_size != 0)
            if ((block = new (std::nothrow) char [(size_t)(details.st_size + 1)] ) != NULL) // +1 for the '\0' on the end
                if (fread(block, (long)details.st_size, 1, fp) == 1) {
                    block[details.st_size] = '\0';
                } else {
                    delete [] block;
                    block = NULL;
                }
    fclose(fp);
    return block;
}

int write_to_disk(const char *filename, const char *buffer, long length) {
    FILE *fp;
    long success; // only ever reads one file

    if ((fp = fopen(filename, "wb")) == NULL)
        return 0;

    success = fwrite(buffer, length, 1, fp);

    fclose(fp);
    return success;
}

