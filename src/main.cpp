#include <filesystem>
#include <iostream>

#include "deduplicator.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: dedup <path>" << std::endl;
    return -1;
  }

  // Validate path
  fs::path root_path = argv[1];
  std::cout << "Provided root path: " << root_path << std::endl;
  if (!root_path.is_absolute()) {
    std::cout << "Path is not absolute. Converting to absolute path" << std::endl;
    root_path = fs::absolute(root_path).lexically_normal();
    std::cout << "Modified root path: " << root_path << std::endl;
  }
  if (!fs::exists(root_path)) {
    std::cerr << "Path: " << root_path << " does not exist" << std::endl;
    return -1;
  }
  if (!fs::is_directory(root_path)) {
    std::cerr << "Path: " << root_path << " is not a directory" << std::endl;
    return -1;
  }

  deduplicator dedup(root_path);
  dedup.find_media();
  //dedup.print_media();
  dedup.find_duplicates();
  dedup.print_duplicates();

  return 0;
}