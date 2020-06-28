#include "cli.h"

#include <cstring>
#include <iostream>
#include <string>

std::string HELP_MENU =
    "Usage: dedup --media-path <path> [optional_flags]\n"
    "Example: dedup --media-path /path/to/media/ --trash-path /path/to/duplicates/trash/ --print-duplicates\n\n"
    "--media-path <path>\tPath of the directory in which the media files will be deduplicated\n"
    "--trash-path <path>\tPath of the directory to which the duplicates will be moved. If not provided\n"
    "                   \tduplicates will be deleted permanently. It is recommended to use this option\n"
    "--print-media      \tPrint paths of media files found\n"
    "--print-duplicates \tPrint paths of duplicates found\n"
    "--no-confirmation  \tNo confirmation before deleting duplicates or moving them trash path (if provided).\n"
    "                   \tBe very careful with this flag especially if no trash path is provided.\n"
    "--help             \tPrint this help message";

bool is_cmd_arg_valid_path(char* arg) {
  fs::path path = arg;
  if (!path.is_absolute()) {
    path = fs::absolute(path).lexically_normal();
  }
  if (!fs::exists(path)) {
    std::cerr << "Path: " << path << " does not exist" << std::endl;
    return false;
  }
  if (!fs::is_directory(path)) {
    std::cerr << "Path: " << path << " is not a directory" << std::endl;
    return false;
  }

  return true;
}

std::optional<cmd_args_t> parse_cmd_args(int argc, char** argv) {
  if (argc < 3) {
    return std::nullopt;
  }

  bool valid_media_path_provided = false;
  cmd_args_t cmd_args;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--media-path")) {
      if (i < argc - 1 && is_cmd_arg_valid_path(argv[i + 1])) {
        cmd_args.media_path = fs::path(argv[i + 1]);
        if (!cmd_args.media_path.is_absolute()) {
          cmd_args.media_path = fs::absolute(cmd_args.media_path).lexically_normal();
        }
        valid_media_path_provided = true;
      } else {
        return std::nullopt;
      }
    }

    if (!strcmp(argv[i], "--trash-path")) {
      if (i < argc - 1 && is_cmd_arg_valid_path(argv[i + 1])) {
        cmd_args.trash_path = fs::path(argv[i + 1]);
        if (!cmd_args.trash_path.is_absolute()) {
          cmd_args.trash_path = fs::absolute(cmd_args.trash_path).lexically_normal();
        }
      } else {
        return std::nullopt;
      }
    }

    if (!strcmp(argv[i], "--print-media")) {
      cmd_args.print_media = true;
    }

    if (!strcmp(argv[i], "--print-duplicates")) {
      cmd_args.print_duplicates = true;
    }

    if (!strcmp(argv[i], "--no-confirmation")) {
      cmd_args.no_confirmation = true;
    }

    if (!strcmp(argv[i], "--help")) {
      return std::nullopt;
    }
  }

  if (valid_media_path_provided) {
    return cmd_args;
  } else {
    return std::nullopt;
  }
}

void print_help_message() { std::cout << HELP_MENU << std::endl; }