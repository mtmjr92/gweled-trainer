#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include "config.h"
#include "feature.h"
#include "trainer.h"
#include "tui.h"

static void printUsage(const char* program) {
	std::cout << "Usage: " << program << " [options] [score]\n"
	          << "Options:\n"
	          << "  -c <path>   Config file path (default: config.json)\n"
	          << "  -h          Show this help\n"
	          << "\nWithout a score, opens the interactive trainer menu.\n"
	          << "With a score, sets it once and exits.\n"
	          << "\nSupports Gweled 1.0-beta1 only (checked against game_version in the config).\n"
	          << "Requires root (sudo) or the CAP_SYS_PTRACE capability.\n";
}

// Non-interactive mode: set the score once and exit.
static int setScoreOnce(gwl::Trainer& trainer, int score) {
	trainer.attach();
	std::cout << "Game version: " << trainer.config().game_version << std::endl;
	std::cout << trainer.config().process_name << " PID: " << trainer.pid() << std::endl;
	std::cout << "Base @ 0x" << std::hex << trainer.baseAddress() << std::dec << std::endl;

	if (!trainer.writeScore(score)) {
		std::cerr << "Failed to write score" << std::endl;
		return 1;
	}
	std::cout << "Score set to " << score << std::endl;
	std::cout << "Current score: " << trainer.readScore() << std::endl;
	return 0;
}

int main(int argc, char* argv[]) {
	std::string config_path = "config.json";
	bool has_score = false;
	int score = 0;

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
			char* end;
			errno = 0;
			long val = std::strtol(argv[i], &end, 10);
			if (*end != '\0' || errno != 0) {
				std::cerr << "Error: invalid score value: " << argv[i] << "\n";
				return 1;
			}
			score = static_cast<int>(val);
			has_score = true;
		}
	}

	try {
		gwl::Trainer trainer(gwl::Config::loadFromFile(config_path));
		if (has_score)
			return setScoreOnce(trainer, score);

		gwl::Features features = gwl::makeFeatures();
		gwl::runTui(trainer, features);
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
