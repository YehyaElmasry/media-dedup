#include <filesystem>
#include <optional>
#include <string>

namespace fs = std::filesystem;

struct cmd_args_t {
  fs::path media_path = "";
  fs::path trash_path = "";
  bool print_media = false;
  bool print_duplicates = false;
  bool no_confirmation = false;
};

std::optional<cmd_args_t> parse_cmd_args(int argc, char** argv);

void print_help_message();
