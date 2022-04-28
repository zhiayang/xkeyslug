// mapping.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"

#include <linux/input.h>
#include <unordered_map>
#include <unordered_set>


using namespace slug;

constexpr auto MOD_CAPSLOCK = KEY_MACRO1;

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
		|| key == MOD_CAPSLOCK;
}

static keycode_t remap_single_key(const slug::WindowInfo& window_info, UInputDevice* ui, keycode_t keycode)
{
	if(keycode == KEY_CAPSLOCK)
		return MOD_CAPSLOCK;

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
	if(ui->isPressed(MOD_CAPSLOCK))
	{
		if(keycode == KEY_Q || keycode == KEY_K)    return ui->sendKeyMomentary(KEY_BACKSPACE);
		else if(keycode == KEY_W)                   return ui->sendKeyMomentary(KEY_UP);
		else if(keycode == KEY_A)                   return ui->sendKeyMomentary(KEY_LEFT);
		else if(keycode == KEY_S)                   return ui->sendKeyMomentary(KEY_DOWN);
		else if(keycode == KEY_D)                   return ui->sendKeyMomentary(KEY_RIGHT);
		else if(keycode == KEY_SEMICOLON || keycode == KEY_LEFT)
			return ui->sendKeyMomentary(KEY_HOME);
		else if(keycode == KEY_APOSTROPHE || keycode == KEY_RIGHT)
			return ui->sendKeyMomentary(KEY_END);
	}

	if(window_info.wm_class == "konsole")
	{
		if(ui->isPressedReal(KEY_LEFTMETA))
		{
			if(keycode == KEY_K)
				return ui->sendCombo({ KEY_LEFTCTRL, KEY_LEFTSHIFT }, KEY_K);
			else if(keycode == KEY_T)
				return ui->sendCombo({ KEY_LEFTCTRL, KEY_LEFTSHIFT }, KEY_T);
			else if(keycode == KEY_W)
				return ui->sendCombo({ KEY_LEFTCTRL, KEY_LEFTSHIFT }, KEY_W);
			else if(keycode == KEY_C)
				return ui->sendCombo({ KEY_LEFTCTRL, KEY_LEFTSHIFT }, KEY_C);
			else if(keycode == KEY_V)
				return ui->sendCombo({ KEY_LEFTCTRL, KEY_LEFTSHIFT }, KEY_V);
		}
	}
	else if(window_info.wm_class != "Sublime_text")
	{
		if(window_info.wm_class == "firefox" && ui->isPressedReal(KEY_LEFTMETA) && KEY_1 <= keycode && keycode <= KEY_9)
			return ui->sendCombo({ KEY_LEFTALT }, keycode);

		if(ui->isPressedReal(KEY_LEFTALT))
		{
			if(keycode == KEY_LEFT)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_LEFT);
			else if(keycode == KEY_RIGHT)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_RIGHT);
			else if(keycode == KEY_UP)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_UP);
			else if(keycode == KEY_DOWN)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_DOWN);
			else if(keycode == KEY_BACKSPACE)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_BACKSPACE);
			else if(keycode == KEY_DELETE)
				return ui->sendCombo({ KEY_LEFTCTRL }, KEY_DELETE);
		}
	}

	return false;
}
















static std::unordered_map<keycode_t, keycode_t> g_currentMapping;

void slug::processKeyEvent(UInputDevice* uinput, Display* x_display, unsigned int real_keycode, KeyAction action)
{
	// special handling for function key
	if(real_keycode == KEY_FN)
	{
		uinput->changeFnKeyState(action);
		return;
	}

	auto window_info = getCurrentWindowInfo(x_display);

	if(is_modifier(real_keycode))
		uinput->pressReal(real_keycode);

	if(action == KeyAction::Release)
	{
		// if we're releasing keys, then just always release the key.
		auto keycode = real_keycode;
		if(auto it = g_currentMapping.find(keycode); it != g_currentMapping.end())
		{
			keycode = it->second;
			g_currentMapping.erase(it);
		}

		if(is_modifier(keycode))
			uinput->unpress(keycode);

		if(is_modifier(real_keycode))
			uinput->unpressReal(real_keycode);

		uinput->sendKey(keycode, action, /* sync: */ true);
		return;
	}

	// first, perform single remappings.
	auto keycode = remap_single_key(window_info, uinput, real_keycode);
	if(keycode != real_keycode)
		g_currentMapping[real_keycode] = keycode;

	if(is_modifier(keycode))
		uinput->press(keycode);

	if(is_modifier(real_keycode))
		uinput->press(real_keycode);

	// if there was no mapping, then just forward the key.
	if(not remap_key_combo(window_info, uinput, keycode, action))
		uinput->sendKey(keycode, action, /* sync: */ true);
}
