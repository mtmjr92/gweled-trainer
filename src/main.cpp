#include <iostream>
#include <cstdlib>
#include "process_memory.h"
#include "gweled_hack.h"

int main(int argc, char* argv[]) {
	int score = 450;
	if (argc > 1)
		score = std::atoi(argv[1]);

	try {
		ProcessMemory mem("gweled");
		std::cout << "gweled PID: " << mem.pid() << std::endl;

		GweledHack hack(mem);

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
