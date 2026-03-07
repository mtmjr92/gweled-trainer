#ifndef GWELED_HACK_H_
#define GWELED_HACK_H_

#include "process_memory.h"
#include <cstdint>

class GweledHack {
public:
	explicit GweledHack(ProcessMemory& mem);

	int readScore();
	bool writeScore(int score);

private:
	static constexpr uint64_t SCORE_OFFSET = 0x187E8;
	static constexpr uint64_t CURRENT_SCORE_OFFSET = 0x18800;

	ProcessMemory& mem_;
	uint64_t base_;
};

#endif
