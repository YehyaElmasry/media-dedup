#include "deduplicator.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>

deduplicator::deduplicator(const fs::path& root_path) {
  this->root_path = root_path;
  return;
}

void deduplicator::find_media() {
  std::cout << "Searching for media" << std::endl;

  std::unordered_map<std::string, long long int> media_extensions;

  std::queue<fs::path> paths_queue;

  paths_queue.push(root_path);

  while (!paths_queue.empty()) {
    const fs::path& curr_path = paths_queue.front();
    std::cout << "Entering directory: " << curr_path << std::endl;

    for (const auto& p : fs::directory_iterator(curr_path)) {
      if (fs::is_directory(p.path())) {
        paths_queue.push(p.path());
      } else if (fs::is_regular_file(p.path())) {
        std::string file_extension = p.path().extension().string();

        std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (this->supported_media_extensions.count(file_extension) > 0) {
          this->media_paths.push_back(p.path());
          media_extensions[file_extension]++;
        }
      }
    }

    paths_queue.pop();
  }

  for (const auto& media_extension : media_extensions) {
    std::cout << "Found " << media_extension.second << " " << media_extension.first << " files" << std::endl;
  }

  return;
}

void deduplicator::print_media() const {
  std::cout << "Printing media files found in " << root_path << std::endl;

  for (const auto& path : this->media_paths) {
    std::cout << path << std::endl;
  }

  return;
}
