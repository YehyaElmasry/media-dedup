#include "hasher.h"

#include <openssl/evp.h>

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>

std::optional<std::string> hash_file(const fs::path& file_path) {
  std::optional<std::string> final_hash = std::nullopt;
  bool hash_success = true;

  std::ifstream in_stream(file_path, std::ios::binary);
  if (!in_stream.is_open()) {
    std::cerr << "Error: Failed to open " << file_path << std::endl;
    return std::nullopt;
  }

  // std::cout << "Hashing " << file_path << std::endl;
  EVP_MD_CTX* context = EVP_MD_CTX_new();
  if (context != NULL) {
    if (EVP_DigestInit_ex(context, EVP_sha512(), NULL)) {
      const int BUFFER_LENGTH = 16384;
      std::byte buffer[BUFFER_LENGTH];
      bool more_to_read = true;
      int bytes_read = BUFFER_LENGTH;
      do {
        in_stream.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
        if (in_stream.bad()) {
          std::cerr << "Error: Failed to read from file " << file_path << std::endl;
          hash_success = false;
          break;
        }
        if (in_stream.eof()) {
          more_to_read = false;
          bytes_read = in_stream.gcount();
        }

        if (!EVP_DigestUpdate(context, buffer, bytes_read)) {
          std::cerr << "Error: Failed to hash file " << file_path << ". Could not update EVP digest" << std::endl;
          hash_success = false;
          break;
        }
      } while (more_to_read);

      if (hash_success) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_length = 0;

        if (EVP_DigestFinal_ex(context, hash, &hash_length)) {
          std::stringstream string_stream;
          for (unsigned int i = 0; i < hash_length; ++i) {
            string_stream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
          }
          final_hash = string_stream.str();
        } else {
          std::cerr << "Error: Failed to hash file " << file_path << ". Could not finalize EVP digest" << std::endl;
          hash_success = false;
        }
      }
    } else {
      std::cerr << "Error: Failed to hash file " << file_path << ". Could not initialize EVP digest" << std::endl;
      hash_success = false;
    }
    EVP_MD_CTX_free(context);
  } else {
    std::cerr << "Error: Failed to hash file " << file_path << ". Could not create EVP context" << std::endl;
    hash_success = false;
  }

  if (hash_success) {
    return final_hash;
  } else {
    return std::nullopt;
  }
}