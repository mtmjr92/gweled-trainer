#include <iostream>
#include <cstdlib>
#include <cstring>
#include "config.h"
#include "process_memory.h"
#include "gweled_hack.h"

static void printUsage(const char* program) {
	std::cout << "Usage: " << program << " [options] [score]\n"
	          << "Options:\n"
	          << "  -c <path>   Config file path (default: config.json)\n"
	          << "  -h          Show this help\n"
	          << "\nScore defaults to 450 if not specified.\n";
}

int main(int argc, char* argv[]) {
	std::string config_path = "config.json";
	int score = 450;

	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-h") == 0) {
			printUsage(argv[0]);
			return 0;
		}
		if (std::strcmp(argv[i], "-c") == 0) {
			if (i + 1 >= argc) {
				std::cerr << "Error: -c requires a path argument\n";
				return 1;
			}
			config_path = argv[++i];
		} else {
			score = std::atoi(argv[i]);
		}
	}

	try {
		gwl::Config config = gwl::Config::loadFromFile(config_path);
		std::cout << "Game version: " << config.game_version << std::endl;

		gwl::ProcessMemory mem(config.process_name);
		std::cout << config.process_name << " PID: " << mem.pid() << std::endl;

		gwl::GweledHack hack(mem, config);
		std::cout << "Base @ 0x" << std::hex << hack.baseAddress() << std::dec << std::endl;

		if (hack.writeScore(score))
			std::cout << "Score set to " << score << std::endl;
		else
			std::cerr << "Failed to write score" << std::endl;

		std::cout << "Current score: " << hack.readScore() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
