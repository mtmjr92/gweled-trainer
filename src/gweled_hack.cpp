#include "gweled_hack.h"

namespace gwl {

GweledHack::GweledHack(ProcessMemory& mem, const Config& config)
	: mem_(mem),
	  base_(mem.getModuleBase(config.module_name)),
	  score_offset_(config.score_offset),
	  current_score_offset_(config.current_score_offset) {}

int GweledHack::readScore() {
	return mem_.readValue<int>(base_ + score_offset_);
}

bool GweledHack::writeScore(int score) {
	bool ok = mem_.writeValue<int>(base_ + score_offset_, score);
	ok &= mem_.writeValue<int>(base_ + current_score_offset_, score);
	return ok;
}

} // namespace gwl
