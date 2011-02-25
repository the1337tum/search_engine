#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>

#include "support.hpp"

// rjhash() mixing macro
#define mix(a, b, c) \
    a -= b, a -= c, a ^= c>>13, \
    b -= c, b -= a, b ^= a<< 8, \
    c -= a, c -= b, c ^= b>>13, \
    a -= b, a -= c, a ^= c>>12, \
    b -= c, b -= a, b ^= a<<16, \
    c -= a, c -= b, c ^= b>> 5, \
    a -= b, a -= c, a ^= c>> 3, \
    b -= c, b -= a, b ^= a<<10, \
    c -= a, c -= b, c ^= b>>15

unsigned long rjhash(
    void const                   *p,
    size_t                        L,
    register unsigned long        c
) {
    register unsigned char const *k = (unsigned char const *)p;
    register unsigned long        a;
    register unsigned long        b;
    size_t                        n;

    a = b = (unsigned long)0x9e3779b9;
    for (n = L; n >= 12; n -= 12) {
	a += ( (unsigned long)k[ 0] +
	      ((unsigned long)k[ 1] <<  8) +
	      ((unsigned long)k[ 2] << 16) +
	      ((unsigned long)k[ 3] << 24) );
	b += ( (unsigned long)k[ 4] +
	      ((unsigned long)k[ 5] <<  8) +
	      ((unsigned long)k[ 6] << 16) +
	      ((unsigned long)k[ 7] << 24) );
	c += ( (unsigned long)k[ 8] +
	      ((unsigned long)k[ 9] <<  8) +
	      ((unsigned long)k[10] << 16) +
	      ((unsigned long)k[11] << 24) );
	mix(a, b, c);
	k += 12;
    }
    c += L;
    switch (n) {
	case 11: c += (unsigned long)k[10] << 24; /*FALLTHROUGH*/
	case 10: c += (unsigned long)k[ 9] << 16; /*FALLTHROUGH*/
	case  9: c += (unsigned long)k[ 8] <<  8; /*FALLTHROUGH*/
	case  8: b += (unsigned long)k[ 7] << 24; /*FALLTHROUGH*/
	case  7: b += (unsigned long)k[ 6] << 16; /*FALLTHROUGH*/
	case  6: b += (unsigned long)k[ 5] <<  8; /*FALLTHROUGH*/
	case  5: b += (unsigned long)k[ 4];       /*FALLTHROUGH*/
	case  4: a += (unsigned long)k[ 3] << 24; /*FALLTHROUGH*/
	case  3: a += (unsigned long)k[ 2] << 16; /*FALLTHROUGH*/
	case  2: a += (unsigned long)k[ 1] <<  8; /*FALLTHROUGH*/
	case  1: a += (unsigned long)k[ 0];       /*FALLTHROUGH*/
	default: break;
    }
    mix(a, b, c);
    return c;
}

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
