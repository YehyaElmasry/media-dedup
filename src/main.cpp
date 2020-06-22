#include <iostream>
#include <filesystem>

#include "deduplicator.h"

int main(int argc, char ** argv){
	if (argc < 2) {
		std::cerr << "Usage: dedup <path>" << std::endl;
		return -1;
	}

	// Validate path
	std::filesystem::path root_path = argv[1];
	std::cout << "Provided root path: " << root_path << std::endl;	
	if (!root_path.is_absolute()) {
		std::cout << "Path is not absolute. Converting to absolute path" << std::endl;
		root_path = std::filesystem::absolute(root_path).lexically_normal();
		std::cout << "Modified root path: " <<  root_path << std::endl;
	}
	if (!std::filesystem::exists(root_path)) {
		std::cerr << "Path: " << root_path << " does not exist" << std::endl;
		return -1;
	}
	if (!std::filesystem::is_directory(root_path)) {
		std::cerr << "Path: " << root_path << " is not a directory" << std::endl;
		return -1;
	}

	enumerate_media(root_path);

	return 0;
}