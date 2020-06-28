#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class deduplicator {
 public:
  deduplicator(const fs::path& media_root_path, const fs::path& trash_root_path, const bool print_media,
               const bool print_duplicates, const bool dedup_without_confirmation);

  bool run();

 private:
  bool find_media();
  bool print_media() const;
  bool find_duplicates();
  bool print_duplicates() const;
  bool remove_duplicates() const;

  std::vector<fs::path> media_paths;
  std::uintmax_t num_media_files;
  std::uintmax_t size_media_files;
  std::uintmax_t num_media_duplicates;
  std::unordered_map<std::string, std::vector<std::uintmax_t>> media_hashes;
  std::vector<std::string> duplicated_hashes;

  fs::path media_root_path = "";
  fs::path trash_root_path = "";
  bool should_print_media = false;
  bool should_print_duplicates = false;
  bool dedup_without_confirmation = false;

  const std::unordered_set<std::string> supported_media_extensions = {".jpg", ".jpeg", ".png", ".mov"};
};

#endif