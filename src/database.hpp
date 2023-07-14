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
  Database(std::fstream fs) : fios(fs) {}
  ~Database() {}
  void append(const char* data, std::streamsize size) {
    lock_t lock(mutex);
    auto pos = fios.tellp();
    fios.seekp(std::ios_base::end);
    fios.write(data, size);
    fios.seekp(pos);
  }

  // sets g to the begining of the first appearance of the key in the file
  void search(const std::string& key) {
    lock_t lock(mutex);
    std::string value;
    auto pos = fios.tellg();
    fios.seekg(std::ios_base::beg);
    // Knuth-Morris-Pratt algorithm
    size_t n = key.size();
    std::vector<size_t> pi(n, 0);
    size_t i = 1;
    char c;
    while (fios.good()) {
      fios.get(c);
      int j = pi[i - 1];
      while (j > 0 && c != key[j]) j = pi[j - 1];
      if (c == key[j]) j++;
      pi[i] = j;
    }
  }
};