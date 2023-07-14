#pragma once
#include <fstream>
#include <shared_mutex>

class tsfile {
 public:
  typedef std::unique_lock<std::shared_mutex> writer_lock;
  typedef std::shared_lock<std::shared_mutex> reader_lock;

 private:
  std::fstream fios;
  std::shared_mutex mutex;

 public:
  tsfile() {}
  void open(std::string name,
            std::ios_base::openmode mode = (std::ios_base::openmode)24) {
    writer_lock lock(mutex);
    fios.open(name, mode);
  }
  tsfile(std::string name,
         std::ios_base::openmode mode = (std::ios_base::openmode)24) {
    open(name, mode);
  }
  void close() {
    writer_lock lock(mutex);
    fios.close();
  }
  ~tsfile() { close(); }
  void write(const char* data, std::streamsize size) {
    writer_lock lock(mutex);
    fios.write(data, size);
  }
  void read(char* data, std::streamsize size) {
    reader_lock lock(mutex);
    fios.read(data, size);
  }
};