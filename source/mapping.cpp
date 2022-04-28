// mapping.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"

#include <linux/input.h>
#include <unordered_map>
#include <unordered_set>


using namespace slug;

static inline bool is_modifier(keycode_t key)
{
	return key == KEY_LEFTMETA
		|| key == KEY_RIGHTMETA
		|| key == KEY_LEFTCTRL
		|| key == KEY_RIGHTCTRL
		|| key == KEY_LEFTALT
		|| key == KEY_RIGHTALT
		|| key == KEY_LEFTSHIFT
		|| key == KEY_RIGHTSHIFT
		|| key == KEY_MACRO1;
}

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

static bool remap_key_combo(const slug::WindowInfo& window_info, UInputDevice* ui, keycode_t keycode, KeyAction action)
{
	if(window_info.wm_class == "firefox")
	{
		if(ui->isPressedReal(KEY_LEFTMETA) && KEY_1 <= keycode && keycode <= KEY_9)
			return ui->sendCombo({ KEY_LEFTALT }, keycode);
	}


	return false;
}





void slug::processKeyEvent(UInputDevice* uinput, Display* x_display, unsigned int keycode, KeyAction action)
{
	auto window_info = getCurrentWindowInfo(x_display);

	auto real_keycode = keycode;
	uinput->pressReal(real_keycode);

	// first, perform single remappings.
	keycode = remap_single_key(window_info, real_keycode);

	if(action == KeyAction::Release)
	{
		// if we're releasing keys, then just always release the key.
		if(is_modifier(keycode))
			uinput->unpress(keycode);

		if(is_modifier(real_keycode))
			uinput->unpressReal(real_keycode);

		uinput->send(EV_KEY, keycode, static_cast<int>(action), /* sync: */ true);
		return;
	}

	if(is_modifier(keycode))
		uinput->press(keycode);

	if(is_modifier(real_keycode))
		uinput->press(real_keycode);

	// if there was no mapping, then just forward the key.
	if(not remap_key_combo(window_info, uinput, keycode, action))
		uinput->send(EV_KEY, keycode, static_cast<int>(action), /* sync: */ true);
}
