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

void write(std::ostream& os, const type& transaction) {
  os.write((char*)&transaction.id, Hash::hashSize);
  os.write((char*)&transaction.type, sizeof(category));
  os.write((char*)&transaction.hash, Hash::hashSize);
  int pubkeysize = transaction.publickey.size();
  os.write((char*)&pubkeysize, sizeof(int));
  os.write(transaction.publickey.c_str(), pubkeysize);
  int signsize = transaction.signature.size();
  os.write((char*)&signsize, sizeof(int));
  os.write(transaction.signature.c_str(), signsize);
}
void read(std::istream& is, type& transaction) {
  is.read((char*)&transaction.id, Hash::hashSize);
  is.read((char*)&transaction.type, sizeof(category));
  is.read((char*)&transaction.hash, Hash::hashSize);
  int pubkeysize = 0;
  is.read((char*)&pubkeysize, sizeof(int));
  char* pubkeybuff = new char[pubkeysize];
  is.read(pubkeybuff, pubkeysize);
  transaction.publickey = std::string(pubkeybuff, pubkeybuff + pubkeysize);
  int signsize = 0;
  is.read((char*)&signsize, sizeof(int));
  char* signbuff = new char[signsize];
  is.read(signbuff, signsize);
  transaction.signature = std::string(signbuff, signbuff + signsize);

  delete[] pubkeybuff;
  delete[] signbuff;
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
