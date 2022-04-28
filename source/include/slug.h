// slug.h
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <utility>
#include <string_view>
#include <unordered_set>

#include "zpr.h"

#include <X11/Xlib.h>

struct libevdev;
struct libevdev_uinput;

namespace slug
{
	using keycode_t = unsigned int;


	struct UInputDevice
	{
		UInputDevice(struct libevdev* based_on);
		~UInputDevice();

		// note: syncs by default. always returns true (kek)
		bool send(unsigned int type, unsigned int code, int value, bool sync = true);
		bool sendCombo(const std::unordered_set<keycode_t>& modifiers, keycode_t keycode, bool sync = true);
		void sync();

		void pressReal(keycode_t key);
		void unpressReal(keycode_t key);
		bool isPressedReal(keycode_t key) const;

		void press(keycode_t key);
		void unpress(keycode_t key);
		bool isPressed(keycode_t key) const;

	private:
		struct libevdev_uinput* m_uinput;
		std::unordered_set<keycode_t> m_modifiers;
		std::unordered_set<keycode_t> m_real_modifiers;
	};

	void loop(struct libevdev* device_ev);

	bool matchWindowClass(Display* x_display, std::string_view window_class);

	struct WindowInfo
	{
		std::string wm_name;
		std::string wm_class;
	};

	WindowInfo getCurrentWindowInfo(Display* x_display);

	enum class KeyAction
	{
		Release = 0,
		Press   = 1,
		Repeat  = 2
	};

	void processKeyEvent(UInputDevice* uinput, Display* x_display, unsigned int code, KeyAction action);
}
