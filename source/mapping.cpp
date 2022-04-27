// mapping.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"

#include <linux/input.h>
#include <unordered_map>
#include <unordered_set>

using keycode_t = unsigned int;

static std::unordered_set<keycode_t> g_pressedKeys;

static keycode_t remap_single_key(const slug::WindowInfo& window_info, keycode_t keycode)
{
	if(keycode == KEY_CAPSLOCK)
		return KEY_MACRO1;

	// for sublime text, keep meta as meta.
	if(keycode == KEY_LEFTMETA)
	{
		if(window_info.wm_class == "Sublime_text")
			return KEY_LEFTMETA;
		else
			return KEY_RIGHTCTRL;
	}

	return keycode;
}






void slug::processKeyEvent(UInputDevice* uinput, Display* x_display, unsigned int keycode, KeyAction action)
{
	auto window_info = getCurrentWindowInfo(x_display);

	// first, perform single remappings.
	keycode = remap_single_key(window_info, keycode);

	if(action == KeyAction::Release)
	{
		// if we're releasing keys, then just always release the key.
		g_pressedKeys.erase(keycode);
		uinput->send(EV_KEY, keycode, static_cast<int>(action), /* sync: */ true);
		return;
	}

	g_pressedKeys.insert(keycode);
	uinput->send(EV_KEY, keycode, static_cast<int>(action), /* sync: */ true);
}
