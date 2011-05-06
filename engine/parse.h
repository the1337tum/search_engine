#include <stdio.h>

// Crude, I know. I was considering using a template instead, but wanted
// to keep it C compatible
#define BUF_SIZE 100

extern void inline parse_collection(FILE *collection);
