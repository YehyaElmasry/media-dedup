#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <filesystem>

namespace fs = std::filesystem;

class deduplicator {
 public:
  deduplicator(const fs::path& root_path);
  void find_duplicates() const;
  void print_duplicates() const;

 private:
  fs::path root_path;
};

#endif