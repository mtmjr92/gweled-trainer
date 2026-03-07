#ifndef GWELED_HACK_H_
#define GWELED_HACK_H_

#include "process_memory.h"
#include "config.h"
#include <cstdint>

namespace gwl {

class GweledHack {
public:
	GweledHack(ProcessMemory& mem, const Config& config);

	uint64_t baseAddress() const { return base_; }
	int readScore();
	bool writeScore(int score);

private:
	ProcessMemory& mem_;
	uint64_t base_;
	uint64_t score_offset_;
	uint64_t current_score_offset_;
};

} // namespace gwl

#endif
