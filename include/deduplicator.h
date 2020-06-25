#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class deduplicator {
 public:
  deduplicator(const fs::path& root_path);

  void find_media();
  void print_media() const;

  void find_duplicates();
  void print_duplicates() const;

 private:
  fs::path root_path;
  std::vector<fs::path> media_paths;
  std::unordered_map<std::string, std::vector<uint64_t>> media_hashes;
  std::vector<std::string> duplicated_hashes;

  const std::unordered_set<std::string> supported_media_extensions = {".jpg", ".jpeg", ".png"};
};

#endif