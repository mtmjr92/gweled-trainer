#ifndef TRAINER_H_
#define TRAINER_H_

#include "process_memory.h"
#include "config.h"
#include <cstdint>
#include <memory>

namespace gwl {

// Thrown when the running game is not the version the offsets were made for.
class VersionError : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

// Game-specific layer: attaches to the game process and exposes its values.
// Not thread-safe; callers must serialize access.
class Trainer {
public:
	explicit Trainer(const Config& config);

	const Config& config() const { return config_; }

	// Throws std::runtime_error if the process or module cannot be found,
	// PermissionError if its memory is inaccessible and VersionError if the
	// game binary does not match config().game_version.
	void attach();
	void detach();
	bool attached() const { return mem_ != nullptr; }

	pid_t pid() const { return mem_ ? mem_->pid() : 0; }
	uint64_t baseAddress() const { return base_; }

	int readScore() const;
	bool writeScore(int score);

	// While locked, tick() keeps rewriting the score to the locked value.
	void lockScore(int score) { locked_ = true; locked_score_ = score; }
	void unlockScore() { locked_ = false; }
	bool scoreLocked() const { return locked_; }

	// Applies periodic effects (score lock). Throws if the process is gone.
	void tick();

private:
	const ProcessMemory& memory() const;

	Config config_;
	std::unique_ptr<ProcessMemory> mem_;
	uint64_t base_ = 0;
	bool locked_ = false;
	int locked_score_ = 0;
};

} // namespace gwl

#endif
