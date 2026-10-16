#ifndef FEATURE_H_
#define FEATURE_H_

#include "trainer.h"
#include <memory>
#include <string>
#include <vector>

namespace gwl {

// A single trainer option shown in the menu. Features are stateless:
// any state they need lives in the Trainer.
class Feature {
public:
	virtual ~Feature() = default;

	virtual std::string name() const = 0;
	virtual bool isToggle() const { return false; }
	virtual bool isEnabled(const Trainer&) const { return false; }
	// Whether activate() consumes the value typed in the menu's input field.
	virtual bool usesInput() const { return false; }

	// Runs the feature (or flips the toggle) and returns a log message.
	virtual std::string activate(Trainer& trainer, int input) const = 0;
};

using Features = std::vector<std::unique_ptr<Feature>>;

Features makeFeatures();

} // namespace gwl

#endif
