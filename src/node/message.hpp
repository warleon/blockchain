#include <string>
namespace message {
enum category {
  set_rol_as_node,
  set_rol_as_client,
  check_and_add_transaction,
  ask_for_block,
  block_to_be_mine,
  hash_of_mined_block,
};
typedef unsigned long int size_t;
typedef struct {
  category cat;
  size_t size;
} header;
typedef std::string body;

typedef struct {
  header header;
  body body;
} type;

type make(category cat, body body) {
  type msg;
  msg.header.cat = cat;
  msg.header.size = body.size();
  msg.body = body;
  return msg;
}

}  // namespace message