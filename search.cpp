//============================================================================
// Name        : index.cpp
// Copyright   : BSDNew
// Description : The indexer
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <new>
#include <ftw.h>
#include <string.h>

#include "support.hpp"

// Read from RAM
char *index_string = read_entire_file("index.index");
long index_length = *reinterpret_cast<long*>(index_string);
// Read from disk
FILE *term_file = fopen("term.index", "rb");
FILE *docs_file = fopen("docs.index", "rb");
FILE *freqs_file = fopen("freqs.index", "rb");

long inline uncompress_next(char *start, long &offset) {
    unsigned long long x, b;
    int shift;

    x = 0, shift = 0;
    while ((b = start[offset++]) > 127) {
        x |= (b & 127) << shift;
        shift += 7;
    }
    return x | (b << shift);
}

class Term {
private:
    char *term;
    // Held compressed
    char *docs;
    long docs_length;
    char *freqs;
    long freqs_length;

public:
    Term(char *term,
    	 char *docs, unsigned long docs_length,
    	 char *freqs, unsigned long freqs_length
    ) {
    	this->term = term;
    	this->docs = docs;
    	this->docs_length = docs_length;
    	this->freqs = freqs;
    	this->freqs_length = freqs_length;
    }

    void inline map_doc(void (*function)(long num)) {
    	for (long offset = 0; offset < docs_length; offset++)
    		function(uncompress_next(docs, offset));
    }

    void inline map_freq(void (*function)(long num)) {
    	for (long offset = 0; offset < freqs_length; offset++)
    		function(uncompress_next(freqs, offset));
    }
};

/*
	index_file
		{<term_start><term_end>}
	term_file
		{<term_start><term_end><doc_start><doc_end><freq_start><freq_end>}
	doc_file
		{<doc_array>}
	freq_file
		{<freq_array>}
                                                             Side effects of this function:
                                                                Input <
                                                                Output >
*/
Term *get_term(char *term) {
	for (long offset = 0; offset < index_length; offset++) {
		int start = uncompress_next(index_string, offset);		// < index_file->term_start
		int stop = uncompress_next(index_string, offset);		// < index_file->term_end

		int term_length = strlen(term);
		if (term_length == start + stop) {                  	/*   Possible match    */
			char *candidate = new char [term_length];
			fseek(term_file, start, SEEK_SET);					// > term_file->term_start
			fread(candidate, stop, 1, term_file);				// > term_file->term_end
			if (strncmp(term, candidate, term_length) == 0) {   /*   Match confirmed    */
				start = stop;									// < index_file->term_start
				stop = uncompress_next(index_string, offset);   // < index_file->term_end
				char *term_string = new char [stop - start];
				fseek(term_file, start, SEEK_SET);				// > term_file->doc_start
				fread(term_string, stop, 1, term_file);			// > term_file->doc_end

				offset = 0;	// Reset for a new offset
				start = uncompress_next(term_string, offset);   // < term_file->doc_start
				stop = uncompress_next(term_string, offset);    // < term_file->doc_end
				long docs_length = stop - start;
				char *docs_string = new char [docs_length];
				fseek(docs_file, start, SEEK_SET);               // > doc_file->doc_array
				fread(docs_string, stop, 1, docs_file);          // > doc_file->doc_array

				start = uncompress_next(term_string, offset);    // < term_file->start_freqs
				stop = uncompress_next(term_string, offset);     // < term_file->freq_end
				long freqs_length = stop - start;
				char *freqs_string = new char [freqs_length];
				fseek(freqs_file, start, SEEK_SET);	             // > freq_file->freq_array
				fread(freqs_string, stop, 1, freqs_file);        // > freq_file->freq_array

			    return new Term(term, docs_string, docs_length, freqs_string, freqs_length);
			}
		}
	}
}

void print_long(long num) {
	printf("%ld\n", num);
}

int main(int argc, char **argv) {
    if (argc == 0) {
        printf("Usage:\n\t search {query}");
        exit(EXIT_FAILURE);
    }
    index_string += sizeof(index_length);

    for (int term = 1; term < argc; term++)
    	get_term(argv[term])->map_doc(&print_long);

    printf("%s\n", "FINISHED");
    return EXIT_SUCCESS;
}
