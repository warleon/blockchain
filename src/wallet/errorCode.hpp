#pragma once

#include <string>
#include <unordered_map>

enum errorCode {
  good = 0,
  noMatch = 100,
  noCommand,
  incompleteArguments,
  execiveArguments,
  invalidArguments,

  cantCreateBigNum = 200,
  cantCreateRSA,
  cantWritePublicKey,
  cantWritePrivateKey,
  multipleErrors,
};

const std::unordered_map<errorCode, std::string> what{
    {good, "Al good"},
    {noMatch, "Unknown command"},
    {noCommand, "At least one command must be suplied"},
    {incompleteArguments, "Not enough arguments suplied to the chosen command"},
    {execiveArguments,
     "Too many arguments suplied to the chosen command some will be ignored"},
    {invalidArguments,
     "Some of the arguments suplid for the command is not a valid one"},

    {cantCreateBigNum, "Can't create a big number"},
    {cantCreateRSA, "Can't create an RSA object"},
    {cantWritePublicKey, "Can't write the public key to the chosen file"},
    {cantWritePrivateKey, "Can't write the private key to the chosen file"},
    {multipleErrors, "Multiple errors have arrised"},
};