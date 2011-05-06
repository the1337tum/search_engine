#include <stdio.h>
#include <ctype.h>
#include "parse.h"
#include "index.hpp"

/*
  Parses the following languages:
  Tokens 
    In:  ((A-B) U (a-b) U (0-9))*
    Out: ((A-B) U (a-b) U (0-9))*
  Words
    In:  ((A-B) U (a-b))*
    Out: (a-b)*
*/

void inline parse_collection(FILE *collection) {
	char *buffer = new char[BUF_SIZE];
	int size = 0;
    
    begin_indexing();

	char c = getc(collection);
	while (c != EOF) {
	    size = 0;
		// Token 
		if (c == '<') {
			c = getc(collection);
			// End Tag
			if (c == '/') {
			    c = getc(collection);
				while (c != '>' && isalnum(c)) {
					buffer[size++] = c;
					c = getc(collection);
				}
                if (c == '>') {
                    buffer[size] = '\0';
                    end_tag(buffer);
                } else {
                    printf("End tag error\n");
                }
               
			// Start Tag
			} else {
				while (c != '>' && isalnum(c)) {
					buffer[size++] = c;
					c = getc(collection);
				}
                if (c == '>') {
                    buffer[size] = '\0';
                    start_tag(buffer);
                } else {
                    printf("End tag error\n");
                }
			}
		}

		// Words
		// This is lazy, I know; but ambiguous cases are a nearly bottomless rabbit hole.
        // Furthermore, the WSJ doesn't escape & properly, so nither do I.
		if (isalpha(c)) {
			do {
			    buffer[size++] = tolower(c);
			    c = getc(collection);
			} while(isalpha(c));
			buffer[size] = '\0';
            word(buffer);
            continue;
		}
        c = getc(collection);
    }
    end_indexing();
}

