//============================================================================
// Name        : index.cpp
// Copyright   : BSDNew
// Description : Reads raw ascii input to make the terms list:
//                   {Term} -> {Doc_Id, Term_Frequency}
//               The terms list is then compressed with variable byte compression,
//               with output generated in the following form:
//                   index.index -> {<term_start><term_length>}
//                   terms.index -> {<Term><docs_start><docs_length><freqs_start><freqs_length>}
//                   docs.index  -> <docs_array>
//                   freqs.index -> <freqs_array>
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <new>
#include <ftw.h>
#include <string.h>

#include "parse.hpp"
#include "support.hpp"

#define HTABLE_SIZE 100003
#define LIST_SIZE 10      // The start length of each list
#define COLLECTION_SIZE 1 // The Wall Street Journal (WSJ)

class ByteArray {
private:
	char *start;
	unsigned long offset;
	unsigned long *length;

	// Warning: This function has *strong* side effects
	void inline write_length(char *&array, unsigned long size) {
		length = reinterpret_cast<unsigned long*>(array);
		*length = size;
		array += sizeof(long);
	}

	void inline resize_array(unsigned long size) {
		char *new_array = (char*) emalloc(size * sizeof(char) + sizeof(long));
		write_length(new_array, size);

		for (unsigned long i = 0; i < offset; i++)
			new_array[i] = start[i];

		free(start - sizeof(long));
		start = new_array;
	}

public:
	ByteArray(unsigned long size) {
        start = (char*) emalloc(size * sizeof(char) + sizeof(long));
        write_length(start, size);
        offset = 0;
	}

	~ByteArray() {
		free(start- sizeof(long));
	}

	void inline truncate() {
		resize_array(offset - sizeof(long));
	}

	unsigned long add(unsigned long value) {
		if ((offset + (value / 128)) >= *length)
			resize_array((*length)*2);

	    while (value > 127) {
	    	start[offset++] = ((value & 127)|128);
	        value >>= 7;
	    }
	    start[offset++] = value;
	    return offset;
	}

	unsigned long concat(const char *tail, unsigned long size) {
		if (offset + size >= *length)
			resize_array(((*length) + size)*2);

		unsigned long index = 0;
		while (index < size)
			start[offset++] = tail[index++];

		return offset;
	}

	unsigned long concat(ByteArray *tail) {
		return concat(tail->get_array(), tail->get_offset());
	}

	inline char *get_array() {
		return start;
	}

	inline char *get_entire_array() {
		return start - sizeof(long);
	}

	const inline unsigned long get_offset() {
		return offset;
	}
};

struct BSTNode {
    char *term;

    BSTNode *left;
    BSTNode *right;

    ByteArray *docs;
    ByteArray *freqs;

    // The last is held uncompressed
    unsigned long last_doc;
    unsigned long last_freq;

    BSTNode(char *word, unsigned long doc_id) {
    	term = word;

        left = NULL;
        right = NULL;

        docs = new ByteArray(LIST_SIZE);
        freqs = new ByteArray(LIST_SIZE);

        last_doc = doc_id;
        last_freq = 1;
    }
    ~BSTNode() { // Remember to delete leaves first
    	delete [] term;
    	delete docs;
    	delete freqs;
    }
};

unsigned long num_docs = 1; // Document number; 1 is the first document, etc...
BSTNode *terms[HTABLE_SIZE];

void inline add_doc(unsigned long doc_id, BSTNode *term) {
	if (doc_id == term->last_doc)
		term->last_freq++;
	else {
		term->docs->add(term->last_doc);
		term->freqs->add(term->last_freq);

		term->last_doc = doc_id;
		term->last_freq = 1;
	}
}

void add_term(char *term, unsigned long doc_id, BSTNode *&terms) {
    if (terms == NULL) {
        terms = new BSTNode(term, doc_id);
    } else {
        int direction = strcmp(term, terms->term);
        if (direction > 0)
            add_term(term, doc_id, terms->left);
        else if (direction < 0)
            add_term(term, doc_id, terms->right);
        else
            add_doc(doc_id, terms);
    }
}

#define STACK_SIZE 10
#define TOKEN_SIZE 10
char stack[STACK_SIZE][TOKEN_SIZE];
int stack_ptr = 0;
int next_is_docno = 0;
ByteArray *doc_ids;

void begin_indexing() {
    doc_ids = new ByteArray(HTABLE_SIZE);
}

void end_indexing() {
    return; 
}

// This function (and everything it stands for) disgusts me.
void word(char const *word) {
    if (next_is_docno) {
        char *tmp = new char [strlen(word)+1];
        int i = 0;
        for (int c = 0; c < strlen(word); c++)
            if (isdigit(word[c]))
                tmp[i++] = word[c];

        doc_ids->add(atoi(tmp));
        delete [] tmp;
        next_is_docno = 0;
    } else {
        char *term = new char [strlen(word)+1];
        strcpy(term, word);
	    add_term(term, num_docs, terms[rjhash(term, strlen(term), HTABLE_SIZE) % HTABLE_SIZE]);
    }
}

void start_tag(char const *tag) {
    if (strlen(tag)+1 > TOKEN_SIZE)
        printf("Token larger than max size\n");
    else
        strcpy(stack[++stack_ptr], tag);
}

int inline match(char const *first, char const *secound) {
   for (int i = 0; i < strlen(first); i++)
        if (first[i] != secound[i])
            return 0;
    return 1;
}

void end_tag(char const *tag) {
    if (match(tag,stack[stack_ptr])) {
        if (match(tag, "DOC"))
 	       num_docs++;
        else if (match(tag, "DOCNO"))
            next_is_docno = 1; // what a discusting hack...
        
        stack_ptr--;
    } else {
        printf("Tag mismatch\n");
    }
}

static int read_collection(const char *filename, const struct stat *sb, int typeflag) {
    FILE *collection;
    if (typeflag == FTW_F)
        if ((collection = fopen(filename, "rb")) != NULL)
        	parse_collection(collection);
    
    return 0;
}

// Length stored at the start of the string
ByteArray *index_file = new ByteArray(6 * num_docs + sizeof(unsigned long));

ByteArray *term_file  = new ByteArray(HTABLE_SIZE);
ByteArray *doc_file   = new ByteArray(num_docs);
ByteArray *freq_file  = new ByteArray(num_docs);

void write_index() {
	index_file->truncate();
	printf("index size: %ld\n", index_file->get_offset());
	write_to_disk("../index/index.index", index_file->get_entire_array(), index_file->get_offset());
	write_to_disk("../index/term.index",  term_file->get_array(),         term_file->get_offset());
	write_to_disk("../index/docs.index",  doc_file->get_array(),          doc_file->get_offset());
	write_to_disk("../index/freqs.index", freq_file->get_array(),         freq_file->get_offset());
}

void write_list(BSTNode *term) {
	term->docs->add(term->last_doc);   // add last doc
	term->freqs->add(term->last_freq); // add last freq
	unsigned long start;

	index_file->add(start = term_file->get_offset());                           // term_seek
	index_file->add(term_file->concat(term->term, strlen(term->term)) - start); // term_offset

	term_file->add(start = doc_file->get_offset());                             // doc_seek
	term_file->add(doc_file->concat(term->docs) - start);                       // doc_offset
	term_file->add(start = freq_file->get_offset());                            // freq_seek
	term_file->add(freq_file->concat(term->freqs) - start);                     // freq_offset
}

void write_term(BSTNode *term) {
    if (term == NULL)
        return;

    write_term(term->left);
    write_term(term->right);

    write_list(term);
    delete term;
}

int main(int argc, char **argv) {

    if (argc == 0) {
        printf("Usage:\n\t index <dir> <dir> ...");
        exit(EXIT_FAILURE);
    }

    for (int root_dir = 1; root_dir < argc; root_dir++)
        ftw(argv[root_dir], read_collection, 100);


    for (unsigned long term = 0; term < HTABLE_SIZE; term++)
    	if (terms[term] != NULL)
    		write_term(terms[term]);

    write_index();

    printf("%s\n", "FINISHED");
    return EXIT_SUCCESS;
}
