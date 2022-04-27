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

	void UInputDevice::send(unsigned int type, unsigned int code, int value, bool should_sync)
	{
		libevdev_uinput_write_event(m_uinput, type, code, value);
		if(should_sync)
			this->sync();
	}

	void UInputDevice::sync()
	{
		libevdev_uinput_write_event(m_uinput, EV_SYN, SYN_REPORT, 0);
	}
}
