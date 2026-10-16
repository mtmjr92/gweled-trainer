#include "feature.h"

namespace gwl {

namespace {

const int kAddAmount = 1000;

class FreezeScore : public Feature {
public:
	std::string name() const override { return "Freeze Score"; }
	bool isToggle() const override { return true; }
	bool isEnabled(const Trainer& t) const override { return t.scoreLocked(); }

	std::string activate(Trainer& t, int) const override {
		if (t.scoreLocked()) {
			t.unlockScore();
			return "Freeze disabled";
		}
		int score = t.readScore();
		t.lockScore(score);
		return "Score frozen at " + std::to_string(score);
	}
};

class SetScore : public Feature {
public:
	std::string name() const override { return "Set Score"; }
	bool usesInput() const override { return true; }

	std::string activate(Trainer& t, int input) const override {
		if (!t.writeScore(input))
			return "Failed to write score";
		return "Score set to " + std::to_string(input);
	}
};

class AddScore : public Feature {
public:
	std::string name() const override { return "Add +" + std::to_string(kAddAmount); }

	std::string activate(Trainer& t, int) const override {
		int score = t.readScore() + kAddAmount;
		if (!t.writeScore(score))
			return "Failed to write score";
		return "Score set to " + std::to_string(score);
	}
};

} // anonymous namespace

Features makeFeatures() {
	Features features;
	features.emplace_back(new FreezeScore);
	features.emplace_back(new SetScore);
	features.emplace_back(new AddScore);
	return features;
}

} // namespace gwl
