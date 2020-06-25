#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

std::optional<std::string> hash_file(const fs::path& file_path);