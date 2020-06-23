#include "deduplicator.h"

#include <iostream>

deduplicator::deduplicator(const fs::path& root_path) {
  this->root_path = root_path;
  return;
}

void deduplicator::find_duplicates() const {
  std::cout << "Beginning media enumeration" << std::endl;

  for (auto& p : fs::recursive_directory_iterator(root_path)) {
    std::cout << p.path() << std::endl;
  }

  return;
}

void deduplicator::print_duplicates() const {
  return;
}
