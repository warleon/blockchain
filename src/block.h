#include "common_types.h"

typedef size_t nonce_t;

typedef struct {
  nonce_t nonce;
  size_t content_size;
} block_metadata;