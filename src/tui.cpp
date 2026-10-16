#include "tui.h"

#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <mutex>
#include <sstream>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace gwl {

namespace {

using namespace ftxui;
using Clock = std::chrono::steady_clock;

const char* const kTitle = " Gweled Trainer v1.0 ";
const auto kTickInterval = std::chrono::milliseconds(100);
const auto kAttachRetry = std::chrono::seconds(1);

// Feature i is bound to F(i+1); F5 is reserved for reattach.
const char* const kFeatureKeyNames[] = {"F1", "F2", "F3", "F4"};
const size_t kMaxHotkeys = sizeof(kFeatureKeyNames) / sizeof(kFeatureKeyNames[0]);

// Built on demand: copying Event::F1 etc. into a global would depend on
// static initialization order across translation units.
const Event& featureKey(size_t i) {
	static const Event keys[kMaxHotkeys] = {Event::F1, Event::F2, Event::F3, Event::F4};
	return keys[i];
}

std::string hex(uint64_t value) {
	std::ostringstream ss;
	ss << "0x" << std::hex << value;
	return ss.str();
}

bool parseInt(const std::string& s, int& out) {
	if (s.empty())
		return false;
	char* end;
	errno = 0;
	long val = std::strtol(s.c_str(), &end, 10);
	if (*end != '\0' || errno != 0 || val < INT_MIN || val > INT_MAX)
		return false;
	out = static_cast<int>(val);
	return true;
}

// Shared between the UI thread and the background worker.
struct State {
	std::mutex mutex;
	Trainer& trainer;
	bool has_score = false;
	int score = 0;
	// Why the last attach failed in a way retrying will not fix; empty otherwise.
	std::string attach_error;
	std::string log;

	explicit State(Trainer& t) : trainer(t) {}

	// Caller must hold the mutex.
	void tryAttach() {
		attach_error.clear();
		try {
			trainer.attach();
			log = "Attached to PID " + std::to_string(trainer.pid());
		} catch (const PermissionError&) {
			attach_error = "✕ Run with sudo";
			log = "Restart with: sudo ./gweled-trainer";
		} catch (const VersionError&) {
			attach_error = "✕ Wrong game version";
			log = "Requires Gweled " + trainer.config().game_version;
		} catch (const std::exception& e) {
			log = std::string("Waiting for game: ") + e.what();
		}
	}

	// Caller must hold the mutex.
	void lost(const std::exception& e) {
		trainer.detach();
		has_score = false;
		log = std::string("Detached: ") + e.what();
	}
};

} // anonymous namespace

void runTui(Trainer& trainer, const Features& features) {
	State state(trainer);
	auto screen = ScreenInteractive::Fullscreen();

	{
		std::lock_guard<std::mutex> lock(state.mutex);
		state.tryAttach();
	}

	std::string input_value = "9999";
	int selected = 0;

	auto activate = [&](size_t index) {
		const Feature& f = *features[index];
		std::lock_guard<std::mutex> lock(state.mutex);
		if (!trainer.attached()) {
			state.log = "Not attached";
			return;
		}
		int value = 0;
		if (f.usesInput() && !parseInt(input_value, value)) {
			state.log = "Invalid value: " + input_value;
			return;
		}
		try {
			state.log = f.activate(trainer, value);
		} catch (const std::exception& e) {
			state.lost(e);
		}
	};

	auto reattach = [&] {
		std::lock_guard<std::mutex> lock(state.mutex);
		trainer.detach();
		state.has_score = false;
		state.tryAttach();
	};

	// Menu: one entry per feature, then Reattach and Quit.
	const size_t reattach_index = features.size();
	const size_t quit_index = features.size() + 1;

	auto entryLabel = [&](size_t i) -> Element {
		if (i == reattach_index)
			return hbox({text("        Reattach"), filler(), text("F5") | dim});
		if (i == quit_index)
			return hbox({text("        Quit"), filler(), text("Esc") | dim});

		const Feature& f = *features[i];
		Element tag = text("      ");
		if (f.isToggle()) {
			bool on = trainer.attached() && f.isEnabled(trainer);
			tag = on ? text("[ON ] ") | color(Color::Green) | bold : text("[   ] ") | dim;
		}
		std::string key = i < kMaxHotkeys ? kFeatureKeyNames[i] : "";
		std::string hint = f.usesInput() ? "  (uses value)" : "";
		return hbox({text("  "), tag, text(f.name()), text(hint) | dim, filler(), text(key) | dim});
	};

	Components entries;
	for (size_t i = 0; i <= quit_index; ++i) {
		MenuEntryOption opt;
		opt.transform = [&, i](const EntryState& s) {
			Element marker = static_cast<int>(i) == selected
				? text(">") | color(Color::Cyan) | bold
				: text(" ");
			Element e = hbox({marker, entryLabel(i) | xflex});
			if (s.focused)
				e = e | inverted;
			return e;
		};
		entries.push_back(MenuEntry("", opt));
	}
	auto menu = Container::Vertical(entries, &selected);

	int input_cursor = 0;
	InputOption input_opt;
	input_opt.multiline = false;
	input_opt.cursor_position = &input_cursor;
	auto input = Input(&input_value, "score", input_opt);

	auto layout = Container::Vertical({menu, input});

	auto renderer = Renderer(layout, [&] {
		std::lock_guard<std::mutex> lock(state.mutex);
		const Config& cfg = trainer.config();

		Element status = trainer.attached()
			? text("● Attached") | color(Color::Green) | bold
			: !state.attach_error.empty()
			? text(state.attach_error) | color(Color::Red) | bold
			: text("○ Waiting for game") | color(Color::Yellow) | bold;
		// The version is only confirmed once attach() has checked the binary.
		Element version = trainer.attached()
			? text("Gweled " + cfg.game_version + " ✓ ") | color(Color::Green)
			: text("Requires Gweled " + cfg.game_version + " ") | dim;
		std::string pid = trainer.attached() ? std::to_string(trainer.pid()) : "-";
		std::string base = trainer.attached() ? hex(trainer.baseAddress()) : "-";
		std::string score = state.has_score ? std::to_string(state.score) : "-";

		auto panel = vbox({
			hbox({text(" Process: "), text(cfg.process_name) | bold,
			      text("  PID "), text(pid), filler(), status, text(" ")}),
			hbox({text(" Base: ") | dim, text(base) | dim, filler(), version}),
			separator(),
			menu->Render(),
			separator(),
			hbox({text(" Value: "), input->Render() | size(WIDTH, EQUAL, 14)}),
			separator(),
			hbox({text(" Score: "), text(score) | bold | color(Color::Cyan),
			      text("   "), text(state.log) | dim | flex}),
		});

		auto window_el = window(text(kTitle) | bold | center, panel) | size(WIDTH, EQUAL, 60);
		auto help = text(input->Focused()
			? "Type the score · Enter apply · Tab back · Esc quit"
			: "↑↓ move · Enter select · Tab value · F1-F5 keys · Esc quit") | dim | center;
		return vbox({window_el, help}) | center;
	});

	auto root = CatchEvent(renderer, [&](Event event) {
		for (size_t i = 0; i < features.size() && i < kMaxHotkeys; ++i) {
			if (event == featureKey(i)) {
				activate(i);
				return true;
			}
		}
		if (event == Event::F5) {
			reattach();
			return true;
		}
		// Tab switches between the menu and the value field.
		if (event == Event::Tab || event == Event::TabReverse) {
			if (input->Focused()) {
				menu->TakeFocus();
			} else {
				input->TakeFocus();
				input_cursor = static_cast<int>(input_value.size());
			}
			return true;
		}
		if (event == Event::Escape || (event == Event::Character('q') && !input->Focused())) {
			screen.ExitLoopClosure()();
			return true;
		}
		if (event == Event::Return) {
			if (input->Focused()) {
				for (size_t i = 0; i < features.size(); ++i) {
					if (features[i]->usesInput()) {
						activate(i);
						break;
					}
				}
				return true;
			}
			size_t i = static_cast<size_t>(selected);
			if (i < features.size() && features[i]->usesInput()) {
				// Edit the value first; Enter in the field applies it.
				input->TakeFocus();
				input_cursor = static_cast<int>(input_value.size());
			} else if (i < features.size())
				activate(i);
			else if (i == reattach_index)
				reattach();
			else if (i == quit_index)
				screen.ExitLoopClosure()();
			return true;
		}
		// Keep the value field numeric; parseInt() validates the rest.
		if (input->Focused() && event.is_character()) {
			const std::string& c = event.character();
			bool numeric = c.size() == 1 && (std::isdigit(static_cast<unsigned char>(c[0])) || c[0] == '-');
			if (!numeric)
				return true;
		}
		return false;
	});

	// Background worker: applies periodic effects, refreshes the score and
	// reattaches when the game is (re)started.
	std::atomic<bool> running(true);
	std::thread worker([&] {
		Clock::time_point last_attempt = Clock::now();
		while (running) {
			{
				std::lock_guard<std::mutex> lock(state.mutex);
				if (trainer.attached()) {
					try {
						trainer.tick();
						state.score = trainer.readScore();
						state.has_score = true;
					} catch (const std::exception& e) {
						state.lost(e);
						last_attempt = Clock::now();
					}
				} else if (Clock::now() - last_attempt >= kAttachRetry) {
					state.tryAttach();
					last_attempt = Clock::now();
				}
			}
			screen.PostEvent(Event::Custom);
			std::this_thread::sleep_for(kTickInterval);
		}
	});

	screen.Loop(root);
	running = false;
	worker.join();
}

} // namespace gwl
