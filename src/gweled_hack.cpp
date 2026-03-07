#include "gweled_hack.h"
#include <iostream>

GweledHack::GweledHack(ProcessMemory& mem)
	: mem_(mem), base_(mem.getModuleBase("gweled")) {
	std::cout << "gweled base @ 0x" << std::hex << base_ << std::dec << std::endl;
}

int GweledHack::readScore() {
	return mem_.readValue<int>(base_ + SCORE_OFFSET);
}

bool GweledHack::writeScore(int score) {
	bool ok = mem_.writeValue<int>(base_ + SCORE_OFFSET, score);
	ok &= mem_.writeValue<int>(base_ + CURRENT_SCORE_OFFSET, score);
	return ok;
}
