#pragma once

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <fstream>
#include <iostream>
#include <string>

#include "./errorCode.hpp"
#include "./hash.hpp"
#include "./keygen.hpp"
#include "./util.hpp"

namespace Transaction {

enum category { null, claim, transference };
const char* what[] = {"null", "claim", "transference"};
typedef struct {
  category type;
  Hash::type hash;
  std::string publickey;
  std::string signature;
  Hash::type id;
} type;

errorCode sign(type& trans, std::string privatekey) {
  Keygen::Key keypair;
  Hash::type transactionHash;
  std::string buffer((char*)&trans,
                     (char*)&trans + sizeof(category) + sizeof(Hash::type));
  buffer += trans.publickey;
  Hash::bytes(buffer.c_str(), buffer.size(), transactionHash);

  keypair.readSerializedPrivateKey(privatekey);
  unsigned char signature[RSA_size(keypair.getKeys())];
  unsigned int signature_len = 0;

  int result = RSA_sign(NID_sha256, transactionHash, Hash::hashSize, signature,
                        &signature_len, keypair.getKeys());
  if (result != 1) {
    return cantSign;
  }
  trans.signature = std::string(signature, signature + signature_len);
  return good;
}
errorCode setId(type& trans) {
  std::string buffer((char*)&trans,
                     (char*)&trans + sizeof(category) + sizeof(Hash::type));
  buffer += trans.publickey;
  buffer += trans.signature;
  Hash::bytes(buffer.c_str(), buffer.size(), trans.id);
  return good;
}

void write(std::ofstream& ofs, const type& transaction) {
  ofs.write((char*)&transaction.type, sizeof(category));
  ofs.write((char*)&transaction.hash, Hash::hashSize);
  int pubkeysize = transaction.publickey.size();
  ofs.write((char*)&pubkeysize, sizeof(int));
  ofs.write(transaction.publickey.c_str(), pubkeysize);
  int signsize = transaction.publickey.size();
  ofs.write((char*)&signsize, sizeof(int));
  ofs.write(transaction.signature.c_str(), signsize);
  ofs.write((char*)&transaction.id, Hash::hashSize);
}
}  // namespace Transaction

std::ostream& operator<<(std::ostream& os,
                         const Transaction::type& transaction) {
  os << Transaction::what[transaction.type] << std::endl;
  os << transaction.hash << std::endl;
  os << transaction.publickey << std::endl;
  os << util::asHex(transaction.signature) << std::endl;
  os << transaction.id << std::endl;
  return os;
}
