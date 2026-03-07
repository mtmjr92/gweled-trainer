#include "config.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gwl {

namespace {

std::string readFile(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file: " + path);
	return std::string((std::istreambuf_iterator<char>(file)),
	                    std::istreambuf_iterator<char>());
}

std::string parseStringField(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	size_t pos = json.find(search);
	if (pos == std::string::npos)
		throw std::runtime_error("Missing config key: " + key);

	pos = json.find(':', pos);
	if (pos == std::string::npos)
		throw std::runtime_error("Malformed config for key: " + key);

	size_t start = json.find('"', pos + 1);
	size_t end = json.find('"', start + 1);
	if (start == std::string::npos || end == std::string::npos)
		throw std::runtime_error("Malformed value for key: " + key);

	return json.substr(start + 1, end - start - 1);
}

uint64_t parseHexField(const std::string& json, const std::string& key) {
	std::string val = parseStringField(json, key);
	uint64_t result;
	std::stringstream ss;
	ss << std::hex << val;
	ss >> result;
	if (ss.fail())
		throw std::runtime_error("Invalid hex value for key: " + key);
	return result;
}

} // anonymous namespace

Config Config::loadFromFile(const std::string& path) {
	std::string json = readFile(path);

	Config cfg;
	cfg.game_version = parseStringField(json, "game_version");
	cfg.process_name = parseStringField(json, "process_name");
	cfg.module_name = parseStringField(json, "module_name");
	cfg.score_offset = parseHexField(json, "score");
	cfg.current_score_offset = parseHexField(json, "current_score");
	return cfg;
}

} // namespace gwl
