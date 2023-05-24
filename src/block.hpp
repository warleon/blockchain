#include <stdio.h>

#include "common_types.h"

typedef size_t nonce_t;
typedef size_t hash_t;
typedef unsigned version_t;
typedef unsigned difficulty_t;
typedef char* buffer_t;

typedef struct {
  hash_t current_hash;
  version_t version;
  difficulty_t difficulty_target;
  hash_t previous_hash;
  nonce_t nonce;
} block_metadata_t;

typedef struct {
  size_t byte_count;
  buffer_t data;
} content_t;

typedef struct {
  block_metadata_t metadata;
  content_t content;
} block_t;

// read to disk
void read(block_t*, FILE*);
// write to disk
void write(block_t*, FILE*);