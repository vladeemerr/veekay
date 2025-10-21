#pragma once

#include <veekay/types.hpp>

namespace veekay::input {

namespace mouse {

enum class Button {
	left,
	middle,
	right,
	count,
};

bool isButtonDown(Button button);
bool isButtonPressed(Button button);

void setCaptured(bool capture);

vec2 cursorPosition();
vec2 cursorDelta();
vec2 scrollDelta();

} // namespace mouse

namespace keyboard {

enum class Key {
	escape, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
	grave, d1, d2, d3, d4, d5, d6, d7, d8, d9, d0, minus, equal, backspace,
	tab, q, w, e, r, t, y, u, i, o, p, left_bracket, right_bracket, backslash,
	caps_lock, a, s, d, f, g, h, j, k, l, semicolon, apostrophe, enter,
	left_shift, z, x, c, v, b, n, m, comma, period, slash, right_shift,
	left_control, left_alt, space, right_alt, right_control,
	insert, home, page_up, kdelete, end, page_down,
	left, up, down, right,
	count,
};

bool isKeyDown(Key key);
bool isKeyPressed(Key key);

} // namespace keyboard

} // namespace glint::input
