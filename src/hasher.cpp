#include "hasher.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>

std::optional<std::string> hash_file(const fs::path& file_path) {
  std::ifstream in_stream(file_path, std::ios::binary);

  if (!in_stream.is_open()) {
    std::cerr << "Error: Failed to open " << file_path << std::endl;
    return std::nullopt;
  }

  std::cout << "Reading " << file_path << std::endl;

  const int BUFFER_LENGTH = 1024;
  std::byte buffer[BUFFER_LENGTH];
  bool can_read = true;
  int bytes_read = BUFFER_LENGTH;
  do {
    in_stream.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
    if (in_stream.bad()) {
      std::cerr << "Error: Failed to read from file " << file_path << std::endl;
      return std::nullopt;
    }
    if (in_stream.eof()) {
      can_read = false;
      bytes_read = in_stream.gcount();
    }
		
		for (int i = 0; i < bytes_read; ++i) {
      std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
		}
    std::memset(buffer, 0, sizeof(buffer));

  } while (can_read);

  std::cout << std::endl;
  exit(0);
}