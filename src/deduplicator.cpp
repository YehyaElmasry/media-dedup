#include "deduplicator.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_map>

#include "hasher.h"

deduplicator::deduplicator(const fs::path& media_root_path, const fs::path& trash_root_path, const bool print_media,
                           const bool print_duplicates, const bool dedup_without_confirmation) {
  this->media_root_path = media_root_path;
  this->trash_root_path = trash_root_path;
  this->should_print_media = print_media;
  this->should_print_duplicates = print_duplicates;
  this->dedup_without_confirmation = dedup_without_confirmation;
  this->num_media_files = 0;
  this->size_media_files = 0;
  this->num_media_duplicates = 0;
}

bool deduplicator::init(fs::path config_file_path) {
  std::ifstream config_stream(config_file_path);
  if (!config_stream.is_open()) {
    std::cerr << "Error: Failed to open configuration file at " << config_file_path << std::endl;
    return false;
  }
  std::string line;
  while (std::getline(config_stream, line)) {
    if (!line.empty() && line[0] == '.') {  // extension
      if (!this->supported_media_extensions.insert(line).second) {
        std::cerr << "Error: Could not insert the extension \"" << line << "\" to the set of accepted extensions"
                  << std::endl;
        return false;
      }
    }
  }

  return true;
}

bool deduplicator::run() {
  if (!find_media()) {
    std::cerr << "Error: Failed to find all media files in " << this->media_root_path << std::endl;
    return false;
  }
  if (this->should_print_media) {
    if (!print_media()) {
      std::cerr << "Error: Failed to print media files in " << this->media_root_path << std::endl;
      return false;
    }
  }

  if (!find_duplicates()) {
    std::cerr << "Error: Failed to find all media duplicates in " << this->media_root_path << std::endl;
    return false;
  }
  if (this->should_print_duplicates) {
    if (!print_duplicates()) {
      std::cerr << "Error: Failed to print media duplicates in " << this->media_root_path << std::endl;
      return false;
    }
  }

  if (!remove_duplicates()) {
    if (this->trash_root_path.empty()) {
      std::cerr << "Error: Failed to delete all media duplicates in " << this->media_root_path << std::endl;
    } else {
      std::cerr << "Error: Failed to move all media duplicates in " << this->media_root_path << " to "
                << this->trash_root_path << std::endl;
    }
    return false;
  }

  return true;
}

bool deduplicator::find_media() {
  std::cout << "Recursively searching for media in " << this->media_root_path << std::endl;

  std::unordered_map<std::string, std::uintmax_t> media_extensions;
  std::set<fs::path> ordered_media_paths;

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
          ordered_media_paths.insert(p.path());
          this->num_media_files++;
          this->size_media_files += fs::file_size(p.path());

          if (media_extensions.count(file_extension) == 0) {
            media_extensions[file_extension] = 0;
          }
          media_extensions[file_extension]++;
        }
      }
    }

    paths_queue.pop();
  }

  // Copy from ordered set to vector, preserving alphabetical order of paths
  this->media_paths.reserve(ordered_media_paths.size());
  std::copy(ordered_media_paths.begin(), ordered_media_paths.end(), std::back_inserter(this->media_paths));

  std::cout << "Found " << this->num_media_files << " (" << (static_cast<double>(size_media_files) / 1e6)
            << " MB) media files in " << this->media_root_path << " with the following breakdown:" << std::endl;

  for (const auto& media_extension : media_extensions) {
    std::cout << "\t- " << media_extension.second << " " << media_extension.first << " files" << std::endl;
  }
  std::cout << "\n";
  return true;
}

bool deduplicator::print_media() const {
  std::cout << "Printing media files found recrusively in " << this->media_root_path << std::endl;

  for (const auto& path : this->media_paths) {
    std::cout << path << std::endl;
  }
  std::cout << "\n";
  return true;
}

bool deduplicator::find_duplicates() {
  std::cout << "Recursively searching for media duplicates in " << this->media_root_path << std::endl;

  if (this->num_media_files != this->media_paths.size()) {
    std::cerr << "Error: Number of media files found does not match the expected value" << std::endl;
    return false;
  }

  std::unordered_map<std::string, std::uintmax_t> duplicates_extensions;

  const int TOTAL_STEPS = 100;
  double progress_step = static_cast<double>(this->size_media_files) / TOTAL_STEPS;
  double next_progress_step = progress_step;
  int current_step = 0;
  std::uintmax_t size_so_far = 0;
  std::uintmax_t duplicates_size = 0;
  std::cout << current_step << "% done\r" << std::flush;
  for (std::uintmax_t i = 0; i < this->num_media_files; ++i) {
    if (size_so_far > next_progress_step) {
      current_step++;
      next_progress_step += progress_step;
      std::cout << current_step << "% done\r" << std::flush;
    }
    const fs::path& file_path = this->media_paths[i];
    std::uintmax_t file_size = fs::file_size(file_path);
    size_so_far += file_size;

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
        }
        default: {  // Another duplicate is found or fell from previous case
          this->num_media_duplicates++;
          duplicates_size += file_size;

          std::string file_extension = file_path.extension().string();
          std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                         [](unsigned char c) { return std::tolower(c); });
          if (duplicates_extensions.count(file_extension) == 0) {
            duplicates_extensions[file_extension] = 0;
          }
          duplicates_extensions[file_extension]++;
          break;
        }
      }
    }
  }
  std::cout << "Found " << this->num_media_duplicates << " (" << (static_cast<double>(duplicates_size) / 1e6)
            << " MB) duplicated media files with the following breakdown:" << std::endl;
  for (const auto& duplicates_extension : duplicates_extensions) {
    std::cout << "\t- " << duplicates_extension.second << " " << duplicates_extension.first << " files" << std::endl;
  }
  std::cout << "\n";

  return true;
}

bool deduplicator::print_duplicates() const {
  std::cout << "Printing duplicated media files found recrusively in " << this->media_root_path << std::endl;

  for (const std::string& duplicated_hash : duplicated_hashes) {
    const auto& duplicates_paths_idxs = this->media_hashes.at(duplicated_hash);
    if (!(duplicates_paths_idxs.size() >= 2)) {
      if (duplicates_paths_idxs.size() == 1) {
        std::cerr << "Error: Unexpected duplicated hash. File " << this->media_paths[duplicates_paths_idxs[0]]
                  << " does not have a duplicate" << std::endl;
      } else {
        std::cerr << "Error: Unexpected duplicated hash. File with this hash was not seen before" << std::endl;
      }
      return false;
    }

    std::cout << "Duplicate for " << this->media_paths[duplicates_paths_idxs[0]] << " found at:" << std::endl;
    for (std::uintmax_t i = 1; i < duplicates_paths_idxs.size(); ++i) {
      std::cout << "\t " << this->media_paths[duplicates_paths_idxs[i]] << std::endl;
    }
    std::cout << "\n";
  }
  std::cout << "\n";
  return true;
}

bool deduplicator::remove_duplicates() const {
  if (this->num_media_duplicates == 0) {
    std::cout << "No duplicates were found. Nothing to remove" << std::endl;
    return true;
  }

  bool copy_to_trash = false;

  if (this->trash_root_path.empty()) {
    std::cout << "No trash path is provided. All media duplicates at: " << this->media_root_path
              << " will be deleted permanently\n";
    if (!this->dedup_without_confirmation) {
      std::cout << "Be careful! This cannot be undone. You might want to provide a trash path instead. See the help "
                   "message at dedup --help"
                << std::endl;
    }
  } else {
    copy_to_trash = true;
    std::cout << "Duplicated media files at: " << this->media_root_path
              << " will be moved to the trash path: " << this->trash_root_path << std::endl;
  }

  if (!this->dedup_without_confirmation) {
    std::string confirmation = "";
    do {
      if (!confirmation.empty()) {
        std::cout << "Please answer with \"yes\" or \"no\"" << std::endl;
      }
      std::cout << "Are you sure you want to proceed? [yes/no]\t" << std::flush;
      std::cin >> confirmation;
    } while (confirmation != "yes" && confirmation != "no");

    if (confirmation == "no") {
      return false;
    }

    std::cout << "\n";
  }

  for (const std::string& duplicated_hash : duplicated_hashes) {
    const auto& duplicates_paths_idxs = this->media_hashes.at(duplicated_hash);
    if (!(duplicates_paths_idxs.size() >= 2)) {
      if (duplicates_paths_idxs.size() == 1) {
        std::cerr << "Error: Unexpected duplicated hash. File " << this->media_paths[duplicates_paths_idxs[0]]
                  << " does not have a duplicate" << std::endl;
      } else {
        std::cerr << "Error: Unexpected duplicated hash. File with this hash was not seen before" << std::endl;
      }

      return false;
    }

    for (std::uintmax_t i = 1; i < duplicates_paths_idxs.size(); ++i) {
      fs::path victim_path = this->media_paths[duplicates_paths_idxs[i]];

      if (copy_to_trash) {  // Copy to trash path before deletion
        fs::path copy_target_path = this->trash_root_path / fs::relative(victim_path, this->media_root_path);
        std::cout << "Copying " << victim_path << " to " << copy_target_path << std::endl;
        fs::create_directories(copy_target_path.parent_path());
        if (!fs::exists(copy_target_path.parent_path())) {
          std::cerr << "Error: Failed to create directory: " << copy_target_path.parent_path() << std::endl;
        } else {
          if (!fs::copy_file(victim_path, copy_target_path)) {
            std::cerr << "Error: Failed to copy " << victim_path << " to " << copy_target_path << std::endl;
          }
        }
      }

      // Permanently delete
      std::cout << "Deleting " << victim_path << std::endl;
      if (!fs::remove(victim_path)) {
        std::cerr << "Error: Failed to delete " << victim_path << std::endl;
        return false;
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  return true;
}