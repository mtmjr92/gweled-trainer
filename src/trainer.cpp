#include "trainer.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace gwl {

namespace {

// Gweled embeds its version as a NUL-terminated string (e.g. "1.0-beta1").
bool binaryHasVersion(pid_t pid, const std::string& version) {
	std::ifstream exe("/proc/" + std::to_string(pid) + "/exe", std::ios::binary);
	if (!exe.is_open())
		throw std::runtime_error("Cannot read game binary to check its version");
	std::string data((std::istreambuf_iterator<char>(exe)), std::istreambuf_iterator<char>());
	std::string needle = std::string(1, '\0') + version + '\0';
	return data.find(needle) != std::string::npos;
}

} // anonymous namespace

Trainer::Trainer(const Config& config) : config_(config) {}

void Trainer::attach() {
	std::unique_ptr<ProcessMemory> mem(new ProcessMemory(config_.process_name));
	uint64_t base = mem->getModuleBase(config_.module_name);

	// Finding the PID and reading /proc/<pid>/maps works without privileges,
	// so probe the actual memory access before reporting success.
	int probe;
	if (!mem->read(base + config_.score_offset, &probe, sizeof(probe))) {
		if (errno == EPERM)
			throw PermissionError("Permission denied: run with sudo");
		throw std::runtime_error(std::string("Cannot read game memory: ") + std::strerror(errno));
	}

	// Offsets only fit one build; writing elsewhere could corrupt the game.
	if (!binaryHasVersion(mem->pid(), config_.game_version))
		throw VersionError("Unsupported game version (requires Gweled " + config_.game_version + ")");

	base_ = base;
	mem_ = std::move(mem);
}

void Trainer::detach() {
	mem_.reset();
	base_ = 0;
	locked_ = false;
}

const ProcessMemory& Trainer::memory() const {
	if (!mem_)
		throw std::runtime_error("Not attached to " + config_.process_name);
	return *mem_;
}

int Trainer::readScore() const {
	return memory().readValue<int>(base_ + config_.score_offset);
}

bool Trainer::writeScore(int score) {
	const ProcessMemory& mem = memory();
	bool ok = mem.writeValue<int>(base_ + config_.score_offset, score);
	ok &= mem.writeValue<int>(base_ + config_.current_score_offset, score);
	if (ok && locked_)
		locked_score_ = score;
	return ok;
}

void Trainer::tick() {
	if (locked_ && readScore() != locked_score_ && !writeScore(locked_score_))
		throw std::runtime_error("Failed to write locked score");
}

} // namespace gwl
