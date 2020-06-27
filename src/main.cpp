#include "cli.h"
#include "deduplicator.h"

int main(int argc, char** argv) {
  std::optional<cmd_args_t> parsing_output = parse_cmd_args(argc, argv);
  if (parsing_output.has_value()) {
    const cmd_args_t& parsed_args = parsing_output.value();

    deduplicator dedup(parsed_args.media_path, parsed_args.trash_path, parsed_args.print_media,
                       parsed_args.print_duplicates, parsed_args.no_confirmation);
    if (dedup.run()){
      return EXIT_SUCCESS;
    } else {
      return EXIT_FAILURE;
    }
  } else {
    print_help_message();
    return EXIT_FAILURE;
  }
}