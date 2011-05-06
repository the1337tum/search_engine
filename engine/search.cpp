//============================================================================
// Name        : index.cpp
// Copyright   : BSDNew
// Description : The indexer
//============================================================================

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ftw.h>
#include <vector>
#include <string.h>

#include "support.hpp"

using namespace std {

// Read from RAM
char *char_array = read_entire_file("../index/index.index");

unsigned long index_length = *reinterpret_cast<unsigned long*>(char_array);
char *index_string = char_array + sizeof(long);

// Read from disk
FILE *term_file = fopen("../index/term.index", "rb");
FILE *docs_file = fopen("../index/docs.index", "rb");
FILE *freqs_file = fopen("../index/freqs.index", "rb");


struct ByteArray {
    char *term;
    Vector<unsigned long> freqs(freqs_length);
    Vector<unsigned long> docs(docs_length);

    ByteArray(char *term,
              Vector<unsigned long> freqs,
              Vector<unsigned long> docs
    ) {
    	this->term = term;
    	this->docs = docs;
    	this->freqs = freqs;
    }
};

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

int merge_union(unsigned long *a, unsigned long a_len, unsigned long *b, unsigned long b_len) {
    unsigned long merge_len = 0;
    unsigned long i = 0;
    unsigned long j = 0;

    while (i < a_len && j < b_len) {
        if (a[i] < b[j]) {
            i++;
        } else
        if (a[i] > b[j]) {
            j++;
        } else {
            a[merge_len] = s[i];
            merge_len++;
            i++;
            j++;
        }
    }
    return merge_len;
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
                
                // Write postings out
                offset = 0;
                Vector<unsigned long> docs(docs_length); // will be *at least* compressed size

                offset = 0;
                Vector<unsigned long> freqs(freqs_length); // will be *at least* compressed size
                for (long freq = uncompress_next(freqs_string, offset, freqs_length);
                     freq != -1;
                     freq = uncompress_next(freqs_string, offset, freqs_length)) {
                    freqs.push_back(freq);
                }

			    return new Term(term, docs, freqs);
			}
		}
	}
	return NULL;
}

int main(int argc, char **argv) {
    unsigned long *intersection;
    unsigned long len;

    if (argc == 0) {
        printf("Usage:\n\t search {query}");
        exit(EXIT_FAILURE);
    }
    
    Term *term = get_term(argv[1]);
    len = term->docs->size;
    intersection = new unsigned long [len];
    for (unsigned long i = 0; i < len; i++)
        term->push_back(term->docs[i]);

    for (unsigned long term = 2; term < argc; term++) {
    	Term rv = get_term(argv[term]);
        for (unsigned long i = 0; i < result->docs->size; i++)
            len = merge_union(intersection, len, &(rv->docs[0]), rv->docs->size);
    }
    
    unsigned long *working = new unsigned long [len];    
    merge_sort(intersection, working, len);
    
    lseek
    for (int rv = 0; rv < len; rv++)
        printf("Document: WSJ%ld", );

    return EXIT_SUCCESS;
}

} // end namespace
