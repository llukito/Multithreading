#ifndef PARADIGMS_PROBLEMS_INVERTED_INDEX_INDEX_H_
#define PARADIGMS_PROBLEMS_INVERTED_INDEX_INDEX_H_

#include "inverted_index.h"

void BuildInvertedIndex(int num_docs,
			char** documents,
			int num_threads,
			InvertedIndex* ii);

#endif
