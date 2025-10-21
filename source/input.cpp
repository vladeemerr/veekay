#include <veekay/input.hpp>

#include <bitset>
#include <unordered_map>
#include <iostream>

#include <GLFW/glfw3.h>

namespace {
	// TODO: Move window to Application state?
	GLFWwindow* window;
} // namespace

namespace veekay::input {

namespace mouse {

namespace {

std::bitset<static_cast<size_t>(mouse::Button::count)> states, cached_states;
vec2 cursor_position, cached_cursor_position;
vec2 scroll_delta;

} // namespace

bool isButtonDown(Button button) {
	return states[static_cast<size_t>(button)];
}

bool isButtonPressed(Button button) {
	size_t index = static_cast<size_t>(button);
	return states[index] && !cached_states[index];
}

void setCaptured(bool capture) {
	glfwSetInputMode(window, GLFW_CURSOR,
	                 capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

vec2 cursorPosition() {
	return cursor_position;
}

vec2 cursorDelta() {
	return cursor_position - cached_cursor_position;
}

vec2 scrollDelta() {
	return scroll_delta;
}

} // namespace mouse

namespace keyboard {

namespace {

std::bitset<static_cast<size_t>(keyboard::Key::count)> states, cached_states;

} // namespace

bool isKeyDown(Key key) {
	return states[static_cast<size_t>(key)];
}

bool isKeyPressed(Key key) {
	size_t index = static_cast<size_t>(key);
	return states[index] && !cached_states[index];
}

} // namespace keyboard

void setup(void* const window_ptr) {
	window = static_cast<GLFWwindow*>(window_ptr);
	
	glfwSetKeyCallback(window, [](GLFWwindow*, int key, int /*scancode*/,
	                              int action, int /*mods*/) {
		auto convert = [](int glfw_key) -> keyboard::Key {
			switch (glfw_key) {
				case GLFW_KEY_ESCAPE: return keyboard::Key::escape;
				case GLFW_KEY_F1: return keyboard::Key::f1;
				case GLFW_KEY_F2: return keyboard::Key::f2;
				case GLFW_KEY_F3: return keyboard::Key::f3;
				case GLFW_KEY_F4: return keyboard::Key::f4;
				case GLFW_KEY_F5: return keyboard::Key::f5;
				case GLFW_KEY_F6: return keyboard::Key::f6;
				case GLFW_KEY_F7: return keyboard::Key::f7;
				case GLFW_KEY_F8: return keyboard::Key::f8;
				case GLFW_KEY_F9: return keyboard::Key::f9;
				case GLFW_KEY_F10: return keyboard::Key::f10;
				case GLFW_KEY_F11: return keyboard::Key::f11;
				case GLFW_KEY_F12: return keyboard::Key::f12;
				case GLFW_KEY_GRAVE_ACCENT: return keyboard::Key::grave;
				case GLFW_KEY_1: return keyboard::Key::d1;
				case GLFW_KEY_2: return keyboard::Key::d2;
				case GLFW_KEY_3: return keyboard::Key::d3;
				case GLFW_KEY_4: return keyboard::Key::d4;
				case GLFW_KEY_5: return keyboard::Key::d5;
				case GLFW_KEY_6: return keyboard::Key::d6;
				case GLFW_KEY_7: return keyboard::Key::d7;
				case GLFW_KEY_8: return keyboard::Key::d8;
				case GLFW_KEY_9: return keyboard::Key::d9;
				case GLFW_KEY_0: return keyboard::Key::d0;
				case GLFW_KEY_MINUS: return keyboard::Key::minus;
				case GLFW_KEY_EQUAL: return keyboard::Key::equal;
				case GLFW_KEY_BACKSPACE: return keyboard::Key::backspace;
				case GLFW_KEY_Q: return keyboard::Key::q;
				case GLFW_KEY_W: return keyboard::Key::w;
				case GLFW_KEY_E: return keyboard::Key::e;
				case GLFW_KEY_R: return keyboard::Key::r;
				case GLFW_KEY_T: return keyboard::Key::t;
				case GLFW_KEY_Y: return keyboard::Key::y;
				case GLFW_KEY_U: return keyboard::Key::u;
				case GLFW_KEY_I: return keyboard::Key::i;
				case GLFW_KEY_O: return keyboard::Key::o;
				case GLFW_KEY_P: return keyboard::Key::p;
				case GLFW_KEY_LEFT_BRACKET: return keyboard::Key::left_bracket;
				case GLFW_KEY_RIGHT_BRACKET: return keyboard::Key::right_bracket;
				case GLFW_KEY_BACKSLASH: return keyboard::Key::backslash;
				case GLFW_KEY_CAPS_LOCK: return keyboard::Key::caps_lock;
				case GLFW_KEY_A: return keyboard::Key::a;
				case GLFW_KEY_S: return keyboard::Key::s;
				case GLFW_KEY_D: return keyboard::Key::d;
				case GLFW_KEY_F: return keyboard::Key::f;
				case GLFW_KEY_G: return keyboard::Key::g;
				case GLFW_KEY_H: return keyboard::Key::h;
				case GLFW_KEY_J: return keyboard::Key::j;
				case GLFW_KEY_K: return keyboard::Key::k;
				case GLFW_KEY_L: return keyboard::Key::l;
				case GLFW_KEY_SEMICOLON: return keyboard::Key::semicolon;
				case GLFW_KEY_APOSTROPHE: return keyboard::Key::apostrophe;
				case GLFW_KEY_ENTER: return keyboard::Key::enter;
				case GLFW_KEY_LEFT_SHIFT: return keyboard::Key::left_shift;
				case GLFW_KEY_Z: return keyboard::Key::z;
				case GLFW_KEY_X: return keyboard::Key::x;
				case GLFW_KEY_C: return keyboard::Key::c;
				case GLFW_KEY_V: return keyboard::Key::v;
				case GLFW_KEY_B: return keyboard::Key::b;
				case GLFW_KEY_N: return keyboard::Key::n;
				case GLFW_KEY_M: return keyboard::Key::m;
				case GLFW_KEY_COMMA: return keyboard::Key::comma;
				case GLFW_KEY_PERIOD: return keyboard::Key::period;
				case GLFW_KEY_SLASH: return keyboard::Key::slash;
				case GLFW_KEY_RIGHT_SHIFT: return keyboard::Key::right_shift;
				case GLFW_KEY_LEFT_CONTROL: return keyboard::Key::left_control;
				case GLFW_KEY_LEFT_ALT: return keyboard::Key::left_alt;
				case GLFW_KEY_SPACE: return keyboard::Key::space;
				case GLFW_KEY_RIGHT_ALT: return keyboard::Key::right_alt;
				case GLFW_KEY_RIGHT_CONTROL: return keyboard::Key::right_control;
				case GLFW_KEY_INSERT: return keyboard::Key::insert;
				case GLFW_KEY_HOME: return keyboard::Key::home;
				case GLFW_KEY_PAGE_UP: return keyboard::Key::page_up;
				case GLFW_KEY_DELETE: return keyboard::Key::kdelete;
				case GLFW_KEY_END: return keyboard::Key::end;
				case GLFW_KEY_PAGE_DOWN: return keyboard::Key::page_down;
				case GLFW_KEY_LEFT: return keyboard::Key::left;
				case GLFW_KEY_UP: return keyboard::Key::up;
				case GLFW_KEY_DOWN: return keyboard::Key::down;
				case GLFW_KEY_RIGHT: return keyboard::Key::right;
			}

			return keyboard::Key::count;
		};

		const auto result = convert(key);

		if (result == keyboard::Key::count) {
			return;
		}

		size_t index = static_cast<size_t>(result);

		switch (action) {
			case GLFW_PRESS:
				keyboard::states[index] = true;
				break;

			case GLFW_RELEASE:
				keyboard::states[index] = false;
				break;
		}
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action,
	                                      int /*mods*/) {
		size_t index;

		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				index = static_cast<size_t>(mouse::Button::left);
				break;

			case GLFW_MOUSE_BUTTON_MIDDLE:
				index = static_cast<size_t>(mouse::Button::middle);
				break;

			case GLFW_MOUSE_BUTTON_RIGHT:
				index = static_cast<size_t>(mouse::Button::right);
				break;

			default:
				return;
		}

		switch (action) {
			case GLFW_PRESS:
				mouse::states[index] = true;
				break;

			case GLFW_RELEASE:
				mouse::states[index] = false;
				break;
		}
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y) {
		mouse::cursor_position.x = float(x);
		mouse::cursor_position.y = float(y);
	});

	glfwSetScrollCallback(window, [](GLFWwindow*, double x, double y) {
		mouse::scroll_delta.x = float(x);
		mouse::scroll_delta.y = float(y);
	});
}

void cache() {
	keyboard::cached_states = keyboard::states;
	mouse::cached_states = mouse::states;

	mouse::cached_cursor_position = mouse::cursor_position;
	mouse::scroll_delta = {};
}

} // namespace glint::input
