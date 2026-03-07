#ifndef CONFIG_H_
#define CONFIG_H_

#include <cstdint>
#include <string>

namespace gwl {

struct Config {
	std::string game_version;
	std::string process_name;
	std::string module_name;
	uint64_t score_offset;
	uint64_t current_score_offset;

	static Config loadFromFile(const std::string& path);
};

} // namespace gwl

#endif
