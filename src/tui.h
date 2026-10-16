#ifndef TUI_H_
#define TUI_H_

#include "feature.h"
#include "trainer.h"

namespace gwl {

// Runs the interactive trainer menu until the user quits.
void runTui(Trainer& trainer, const Features& features);

} // namespace gwl

#endif
