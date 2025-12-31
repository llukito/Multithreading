# Concurrent Inverted Indexer

A high-performance C library designed to build an inverted index from large-scale text collections using parallel processing. This system efficiently maps every term found in a document set to its specific occurrences (Document ID and character offset) while leveraging multi-core architectures through POSIX threads.

---

## Overview

The Concurrent Inverted Indexer processes an array of text documents to identify word locations. It is designed to handle irregular whitespace—treating single or multiple spaces as delimiters—to ensure accurate word extraction and positioning.

---

## Example Mapping

Given two documents:

- **Doc 0:** `"Hello World"`
- **Doc 1:** `"Hello Free Uni"`

The resulting inverted index:

Hello: (0, 0), (1, 0)
World: (0, 6)
Free: (1, 6)
Uni: (1, 12)

Each entry maps a word to a list of `(Document ID, Character Offset)` pairs.

---

## Technical Features

### 1. Optimal Thread Synchronization

The implementation prioritizes thread independence and load balancing.

- **Dynamic Work Distribution**  
  Instead of static partitioning (which can cause straggler threads when document sizes vary), threads dynamically claim work.

- **Low Contention**  
  A shared atomic counter is used for claiming the next available document, minimizing synchronization overhead.

- **Asynchronous Processing**  
  Worker threads process documents independently and only synchronize when inserting results into the shared `InvertedIndex`.

---

### 2. Robust String Parsing

The parsing engine is optimized for both memory safety and performance:

- **Single-Pass Scanning**  
  Each document is scanned in one linear pass (`O(N)` time complexity).

- **Whitespace Resilience**  
  Correctly computes character offsets even with leading, trailing, or multiple consecutive spaces.

- **Memory Ownership Model**  
  Parsed words are dynamically allocated and transferred to the index, which assumes responsibility for deallocation.

---

### 3. Thread Safety

While the underlying `InvertedIndex` data structure is not inherently thread-safe, this library ensures correctness by:

- Wrapping insertions with fine-grained mutex locks
- Preventing race conditions during concurrent updates

---

## API Reference

### `BuildInvertedIndex`

The primary entry point for generating the inverted index.

```c
void BuildInvertedIndex(
    int num_docs,
    char** documents,
    int num_threads,
    InvertedIndex* ii
);

Build and Execution
Requirements

GCC compiler

POSIX threads (pthread)

Compilation
gcc map.c index.c inverted_index.c index_test.c -lpthread -o indexer_suite

Running Tests
./indexer_suite

Performance Considerations

Memory Efficiency
Avoids duplicating full documents; only individual words are extracted and stored.

Work Stealing
Dynamic document assignment ensures no thread remains idle while work is still available.

Scalability
Designed to scale linearly with available CPU cores based on the num_threads parameter.
