#include <string>

class connection;
namespace message {
enum category {
  // requests
  set_rol_as_worker,
  set_rol_as_client,
  check_and_add_transaction,
  ask_for_block,
  block_to_be_mine,
  hash_of_mined_block,
  // responses
  transaction_accepted,
  transaction_rejected,
  rol_changed_to_worker,
  rol_changed_to_client,
  wrong_message_body,
  failed_to_change_rol,
  connection_success,
};
std::string str[]{
    // requests
    "set_rol_as_worker",
    "set_rol_as_client",
    "check_and_add_transaction",
    "ask_for_block",
    "block_to_be_mine",
    "hash_of_mined_block",
    // responses
    "transaction_accepted",
    "transaction_rejected",
    "rol_changed_to_worker",
    "rol_changed_to_client",
    "wrong_message_body",
    "failed_to_change_rol",
    "connection_success",
};

typedef unsigned long int size_t;
typedef struct {
  category cat;
  size_t size;
} header_t;
typedef std::string body_t;

typedef struct {
  header_t head;
  body_t body;
} type;

type make(category cat, body_t body) {
  type msg;
  msg.head.cat = cat;
  msg.head.size = body.size();
  msg.body = body;
  return msg;
}

typedef struct {
  std::shared_ptr<connection> remote = nullptr;
  type msg;
} owned_t;

}  // namespace message