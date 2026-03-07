#include "process_memory.h"

#include <sys/uio.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <dirent.h>

namespace gwl {

ProcessMemory::ProcessMemory(const std::string& process_name)
	: pid_(findPID(process_name)) {}

pid_t ProcessMemory::findPID(const std::string& process_name) {
	DIR* dir = opendir("/proc");
	if (!dir)
		throw std::runtime_error("Cannot open /proc");

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_type != DT_DIR)
			continue;

		char* end;
		long pid = strtol(entry->d_name, &end, 10);
		if (*end != '\0')
			continue;

		std::string comm_path = "/proc/" + std::string(entry->d_name) + "/comm";
		std::ifstream comm_file(comm_path);
		if (!comm_file.is_open())
			continue;

		std::string comm;
		std::getline(comm_file, comm);
		if (comm == process_name) {
			closedir(dir);
			return static_cast<pid_t>(pid);
		}
	}

	closedir(dir);
	throw std::runtime_error("Process not found: " + process_name);
}

uint64_t ProcessMemory::parseModuleBase(const std::string& module) {
	std::string maps_path = "/proc/" + std::to_string(pid_) + "/maps";
	std::ifstream maps_file(maps_path);
	if (!maps_file.is_open())
		throw std::runtime_error("Cannot open " + maps_path);

	std::string line;
	while (std::getline(maps_file, line)) {
		if (line.find(module) == std::string::npos)
			continue;

		std::string addr_str = line.substr(0, line.find('-'));
		uint64_t addr;
		std::stringstream ss;
		ss << std::hex << addr_str;
		ss >> addr;
		return addr;
	}

	throw std::runtime_error("Module not found in maps: " + module);
}

uint64_t ProcessMemory::getModuleBase(const std::string& module) {
	return parseModuleBase(module);
}

bool ProcessMemory::read(uint64_t addr, void* buf, size_t size) {
	if (pid_ == 0 || size == 0)
		return false;

	iovec local{buf, size};
	iovec remote{reinterpret_cast<void*>(addr), size};

	ssize_t n = process_vm_readv(pid_, &local, 1, &remote, 1, 0);
	return n == static_cast<ssize_t>(size);
}

bool ProcessMemory::write(uint64_t addr, const void* buf, size_t size) {
	if (pid_ == 0 || size == 0)
		return false;

	iovec local{const_cast<void*>(buf), size};
	iovec remote{reinterpret_cast<void*>(addr), size};

	ssize_t n = process_vm_writev(pid_, &local, 1, &remote, 1, 0);
	return n == static_cast<ssize_t>(size);
}

} // namespace gwl
