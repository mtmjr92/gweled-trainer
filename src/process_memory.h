#ifndef PROCESS_MEMORY_H_
#define PROCESS_MEMORY_H_

#include <cstdint>
#include <string>
#include <stdexcept>
#include <sys/types.h>

namespace gwl {

// Thrown when the kernel denies access to another process's memory
// (not root, no CAP_SYS_PTRACE, or blocked by Yama ptrace_scope).
class PermissionError : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

class ProcessMemory {
public:
	explicit ProcessMemory(const std::string& process_name);

	pid_t pid() const { return pid_; }
	uint64_t getModuleBase(const std::string& module) const;
	bool read(uint64_t addr, void* buf, size_t size) const;
	bool write(uint64_t addr, const void* buf, size_t size) const;

	template <typename T>
	T readValue(uint64_t addr) const {
		T val{};
		if (!read(addr, &val, sizeof(T)))
			throw std::runtime_error("Failed to read from address");
		return val;
	}

	template <typename T>
	bool writeValue(uint64_t addr, T val) const {
		return write(addr, &val, sizeof(T));
	}

private:
	static pid_t findPID(const std::string& process_name);
	uint64_t parseModuleBase(const std::string& module) const;

	pid_t pid_;
};

} // namespace gwl

#endif
