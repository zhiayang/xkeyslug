// uinput.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

namespace slug
{
	UInputDevice::UInputDevice(struct libevdev* based_on)
	{
		auto err = libevdev_uinput_create_from_device(/* based? based on what? */ based_on,
			LIBEVDEV_UINPUT_OPEN_MANAGED, &m_uinput);

		if(err != 0)
		{
			zpr::fprintln(stderr, "failed to create uinput: {}", err);
			exit(1);
		}
	}

	UInputDevice::~UInputDevice()
	{
		libevdev_uinput_destroy(m_uinput);
	}

	bool UInputDevice::send(unsigned int type, unsigned int code, int value, bool should_sync)
	{
		libevdev_uinput_write_event(m_uinput, type, code, value);
		if(should_sync)
			this->sync();

		return true;
	}

	bool UInputDevice::sendKeyMomentary(keycode_t keycode, bool should_sync)
	{
		libevdev_uinput_write_event(m_uinput, EV_KEY, keycode, static_cast<int>(KeyAction::Press));
		libevdev_uinput_write_event(m_uinput, EV_KEY, keycode, static_cast<int>(KeyAction::Release));

		if(should_sync)
			this->sync();

		return true;
	}

	bool UInputDevice::sendKey(keycode_t key, KeyAction action, bool should_sync)
	{
		libevdev_uinput_write_event(m_uinput, EV_KEY, key, static_cast<int>(action));
		if(should_sync)
			this->sync();

		return true;
	}

	bool UInputDevice::sendCombo(const std::unordered_set<keycode_t>& modifiers, keycode_t keycode, bool should_sync)
	{
		// send one set of stuff (without syncing); release any unrelated modifiers,
		// press the modifiers we need to press, send press the keycode, then re-press the unrelated modifiers,
		// then sync the whole set of actions.

		std::unordered_set<keycode_t> extra_modifiers {};
		std::unordered_set<keycode_t> unrelated_modifiers {};
		for(auto x : m_modifiers)
		{
			if(modifiers.find(x) == modifiers.end())
			{
				unrelated_modifiers.insert(x);
				this->send(EV_KEY, x, static_cast<int>(KeyAction::Release), /* sync: */ false);
			}
		}

		for(auto x : modifiers)
		{
			if(m_modifiers.find(x) == m_modifiers.end())
			{
				extra_modifiers.insert(x);
				this->send(EV_KEY, x, static_cast<int>(KeyAction::Press), /* sync: */ false);
			}
		}

		this->send(EV_KEY, keycode, static_cast<int>(KeyAction::Press), /* sync: */ false);
		this->send(EV_KEY, keycode, static_cast<int>(KeyAction::Release), /* sync: */ false);

		for(auto x : unrelated_modifiers)
			this->send(EV_KEY, x, static_cast<int>(KeyAction::Press), /* sync: */ false);

		for(auto x : extra_modifiers)
			this->send(EV_KEY, x, static_cast<int>(KeyAction::Release), /* sync: */ false);

		if(should_sync)
			this->sync();

		return true;
	}

	void UInputDevice::press(keycode_t key)
	{
		m_modifiers.insert(key);
	}

	void UInputDevice::unpress(keycode_t key)
	{
		m_modifiers.erase(key);
	}

	bool UInputDevice::isPressed(keycode_t key) const
	{
		return m_modifiers.find(key) != m_modifiers.end();
	}

	void UInputDevice::pressReal(keycode_t key)
	{
		m_real_modifiers.insert(key);
	}

	void UInputDevice::unpressReal(keycode_t key)
	{
		m_real_modifiers.erase(key);
	}

	bool UInputDevice::isPressedReal(keycode_t key) const
	{
		return m_real_modifiers.find(key) != m_real_modifiers.end();
	}



	void UInputDevice::sync()
	{
		libevdev_uinput_write_event(m_uinput, EV_SYN, SYN_REPORT, 0);
	}
}
