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
char *char_array = read_entire_file("../index.index");

unsigned long index_length = *reinterpret_cast<unsigned long*>(char_array);
char *index_string = char_array + sizeof(long);

// Read from disk
FILE *term_file = fopen("../term.index", "rb");
FILE *docs_file = fopen("../docs.index", "rb");
FILE *freqs_file = fopen("../freqs.index", "rb");

long inline uncompress_next(char *start, unsigned long &offset, unsigned long length) {
	if (offset >= length) {
		printf("Array out of bounds\n");
		return -1;
	}

	unsigned long long x, b;
    int shift;

    x = 0, shift = 0;
    while ((b = start[offset++]) > 127) {
        x |= (b & 127) << shift;
        shift += 7;
    }
    return x | (b << shift);
}

// To be considered efficient, this is a 'use once' container.
class ByteArray {
private:
    char *term;

    

    // Held compressed
    char *docs;
    unsigned long docs_offset;
    unsigned long docs_length;

    char *freqs;
    unsigned long freqs_offset;
    unsigned long freqs_length;

public:
    ByteArray(char *term,
    	char *docs, unsigned long docs_length,
    	char *freqs, unsigned long freqs_length
    ) {
    	this->term = term;
    	this->docs = docs;
    	this->docs_offset = 0;
    	this->docs_length = docs_length;
    	this->freqs = freqs;
    	this->freqs_offset = 0;
    	this->freqs_length = freqs_length;
    }
    // for (unsigned long doc_id = term->begin_docs(); term->end_docs(); doc_id = term->next_doc())
    unsigned long inline begin_docs() {
    	docs_offset = 0;
    	return uncompress_next(docs, docs_offset, docs_length);
    }
    unsigned long inline next_doc() {
    	return uncompress_next(docs, docs_offset, docs_length);
    }
    short inline end_docs() {
    	return docs_offset < docs_length;
    }

    // for (unsigned long freq = term->begin_freqs(); term->end_freqs(); doc_id = term->next_freq())
    unsigned long inline begin_freqs() {
    	freqs_offset = 0;
    	return uncompress_next(freqs, freqs_offset, freqs_length);
    }
    unsigned long inline next_freq() {
    	return uncompress_next(freqs, freqs_offset, freqs_length);
    }
    short inline end_freqs() {
    	return freqs_offset < freqs_length;
    }

    // map(&function_name)
    void inline map(void (*function)(unsigned long doc_id, unsigned long freq)) {

        unsigned long doc_id = 0;
        unsigned long freq = 0;
        unsigned long num_docs = 0;
        while (doc_id < docs_length && freq < freqs_length) {
        	 function(uncompress_next(docs, doc_id, docs_length),
        			  uncompress_next(freqs, freq, freqs_length));
             num_docs++;
        }
        printf("Number of documents: %ld\n", num_docs);
    }
};



ByteArray **sort_results(ByteArray **terms) {
   unsigned short start = 0;
   unsigned long lowest = find_lowest(terms, &start);
   unsigned short num_lowest = 1;

   ByteArray **results = emalloc(argc * sizeof(ByteArray*));
   for (int rank = 0; rank < argc; rank++)
       result[rank] = new ByteArray(longest_list(terms));

   for (unsigned short term = start + 1; start > 0; term++ ) {
       if (term + start % argsc == 0) {
           results[num_lowest]->add(lowest);
           lowest = find_lowest(terms, &start);
           term = start + 1;
       }

       if (terms[term]->first_doc == lowest) {
           num_lowest++;
           terms[term]->next();
       }
   }
   return results;
}

/*
	index_file
		{<term_start><term_length>}
	term_file
		{<term><doc_start><doc_length><freq_start><freq_length>}
	doc_file
		{<doc_array>}
	freq_file
		{<freq_array>}
*/
ByteArray *get_term(char *term) {

	unsigned long offset = 0;
	while (offset < index_length) {

		// {<term_start><term_length>}
		unsigned long start = uncompress_next(index_string, offset, index_length);
		unsigned long term_length = uncompress_next(index_string, offset, index_length);

		if (term_length == strlen(term)) {

			// {<term>}
			char *candidate = new char [term_length];
			fseek(term_file, start, SEEK_SET);
			fread(candidate, term_length, 1, term_file);

			if (strncmp(term, candidate, term_length) == 0) {

				// {<doc_start><doc_length><freq_start><freq_length>}
				start += term_length;
				term_length = uncompress_next(index_string, offset, index_length) - start;
				char *term_string = new char [term_length];
				fseek(term_file, start, SEEK_SET);
				fread(term_string, term_length, 1, term_file);

				offset = 0; // Reset offset

				// {<docs>}
				start = uncompress_next(term_string, offset, term_length);
				unsigned long docs_length = uncompress_next(term_string, offset, term_length);
				char *docs_string = new char [docs_length];
				fseek(docs_file, start, SEEK_SET);
				fread(docs_string, docs_length, 1, docs_file);

				// {<freqs>}
				start = uncompress_next(term_string, offset, term_length);
				unsigned long freqs_length = uncompress_next(term_string, offset, term_length);
				char *freqs_string = new char [freqs_length];
				fseek(freqs_file, start, SEEK_SET);
				fread(freqs_string, freqs_length, 1, freqs_file);

			    return new ByteArray(term, docs_string, docs_length, freqs_string, freqs_length);
			}
		}
	}
	return NULL;
}

void print(unsigned long doc_id, unsigned long freq) {
	printf("Doc_Id: %ld\t\t Freq: %ld\n", doc_id, freq);
}

int main(int argc, char **argv) {
	printf("entire_array: %ld\n", index_length);

    if (argc == 0) {
        printf("Usage:\n\t search {query}");
        exit(EXIT_FAILURE);
    }

    for (int term = 1; term < argc; term++) {
    	ByteArray *result = get_term(argv[term]);
    	if (result != NULL)
    		result->map(&print);
    	else
    		printf("result == NULL\n");
    }

    printf("%s\n", "FINISHED");
    return EXIT_SUCCESS;
}