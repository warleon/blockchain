#pragma once
#include <fstream>
#include <mutex>
#include <vector>

class Database {
 public:
  typedef std::scoped_lock<std::mutex> lock_t;

 private:
  std::fstream& fios;
  std::mutex mutex;

 public:
  Database(std::fstream& fs) : fios(fs) {}
  ~Database() {}
  void append(const char* data, std::streamsize size) {
    lock_t lock(mutex);
    auto pos = fios.tellp();
    fios.seekp(std::ios_base::end);
    fios.write(data, size);
    fios.seekp(pos);
  }

  // sets g to the begining of the first appearance of the key in the file
  bool search(const std::string& key) {
    lock_t lock(mutex);
    std::string value;
    fios.seekg(std::ios_base::beg);
    // Knuth-Morris-Pratt algorithm
    long n = key.size();
    std::vector<long> pi(n, 0);
    long i = 1;
    char c;
    long count = 0;
    while (fios.good()) {
      fios.get(c);
      long j = count;
      while (j > 0 && c != key[j]) j = pi[j - 1];
      if (c == key[j]) j++;
      count = j;
      if (count < n) pi[j] = count;
      if (count == n) break;
    }
    if (count == n) {
      fios.seekg(-n, std::ios_base::cur);
      return true;
    }
    return false;
  }
};