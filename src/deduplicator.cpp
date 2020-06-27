#include "deduplicator.h"

#include <algorithm>
#include <cassert>  // #define NDEBUG // uncomment to disable assert()
#include <iostream>
#include <queue>
#include <unordered_map>

#include "hasher.h"

deduplicator::deduplicator(const fs::path& media_root_path, const fs::path& trash_root_path, const bool print_media,
                           const bool print_duplicates, const bool dedup_without_confirmation) {
  this->media_root_path = media_root_path;
  this->trash_root_path = trash_root_path;
  this->should_print_media = print_media;
  this->should_print_duplicates = print_duplicates;
  this->dedup_without_confirmation = dedup_without_confirmation;
  num_media_files = 0;
}

bool deduplicator::run() {
  find_media();
  if (this->should_print_media) {
    print_media();
  }

  find_duplicates();
  if (this->should_print_duplicates) {
    print_duplicates();
  }

  return true;
}

void deduplicator::find_media() {
  std::cout << "Recursively searching for media in " << this->media_root_path << std::endl;

  std::unordered_map<std::string, uint64_t> media_extensions;
  uint64_t num_media_files = 0;

  std::queue<fs::path> paths_queue;
  paths_queue.push(media_root_path);
  while (!paths_queue.empty()) {
    const fs::path& curr_path = paths_queue.front();
    std::cout << "Searching directory: " << curr_path << std::endl;

    for (const auto& p : fs::directory_iterator(curr_path)) {
      if (fs::is_directory(p.path())) {
        paths_queue.push(p.path());
      } else if (fs::is_regular_file(p.path())) {
        std::string file_extension = p.path().extension().string();

        std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (this->supported_media_extensions.count(file_extension) > 0) {
          this->media_paths.push_back(p.path());
          num_media_files++;

          if (media_extensions.count(file_extension) == 0) {
            media_extensions[file_extension] = 0;
          }
          media_extensions[file_extension]++;
        }
      }
    }

    paths_queue.pop();
  }

  this->num_media_files = num_media_files;
  std::cout << "Found " << this->num_media_files << " media files in " << this->media_root_path
            << " with the following breakdown:" << std::endl;

  for (const auto& media_extension : media_extensions) {
    std::cout << "\t- " << media_extension.second << " " << media_extension.first << " files" << std::endl;
  }
  std::cout << "\n";
}

void deduplicator::print_media() const {
  std::cout << "Printing media files found recrusively in " << this->media_root_path << std::endl;

  for (const auto& path : this->media_paths) {
    std::cout << path << std::endl;
  }
  std::cout << "\n";
}

void deduplicator::find_duplicates() {
  std::cout << "Recursively searching for media duplicates in " << this->media_root_path << std::endl;

  uint64_t num_duplicates_found = 0;
  assert(this->num_media_files == this->media_paths.size());
  uint64_t progress_step = this->num_media_files / 100;

  for (uint64_t i = 0; i < this->num_media_files; ++i) {
    if (progress_step > 0 && i % progress_step == 0) {
      uint64_t progress = i / progress_step;
      std::cout << progress << "% done" << std::endl;
    }
    const fs::path& file_path = this->media_paths[i];

    std::optional<std::string> hash = hash_file(file_path);
    if (!hash.has_value() || hash.value().empty()) {
      std::cerr << "Error: Failed to hash media file at " << file_path << std::endl;
    } else {
      // std::cout << "Hash: " << hash.value() << std::endl;
      this->media_hashes[hash.value()].push_back(i);

      switch (this->media_hashes[hash.value()].size()) {
        case 1: {  // File was not seen before
          break;
        }
        case 2: {  // First duplicate is found mark file as duplicated
          this->duplicated_hashes.push_back(hash.value());
          num_duplicates_found++;
          break;
        }
        default: {  // Another duplicate is found
          num_duplicates_found++;
          break;
        }
      }
    }
  }
  std::cout << "Found " << num_duplicates_found << " duplicated media files" << std::endl;
  std::cout << "\n";
}

void deduplicator::print_duplicates() const {
  std::cout << "Printing duplicated media files found recrusively in " << this->media_root_path << std::endl;

  for (const std::string& duplicated_hash : duplicated_hashes) {
    const auto& duplicates_paths_idxs = this->media_hashes.at(duplicated_hash);
    assert(duplicates_paths_idxs.size() >= 2);

    std::cout << "Duplicate for " << this->media_paths[duplicates_paths_idxs[0]] << " found at:" << std::endl;
    for (uint64_t i = 1; i < duplicates_paths_idxs.size(); ++i) {
      std::cout << "\t " << this->media_paths[duplicates_paths_idxs[i]] << std::endl;
    }
  }
  std::cout << "\n";
}