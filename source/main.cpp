// main.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"
#include <signal.h>

#include <atomic>

#include <X11/Xlib.h>
#include <libevdev/libevdev.h>

static constexpr const char* KEYBOARD_EVENT_DEVICE = "/dev/input/by-id/usb-Apple_Inc._Apple_Internal_Keyboard___Trackpad_FM7036205D9N1R1B3+TNN-if01-event-kbd";

static std::atomic<bool> g_quit = false;

int main(int argc, char** argv)
{
	auto device_ev = libevdev_new();
	auto device_fd = open(KEYBOARD_EVENT_DEVICE, O_RDONLY);
	if(device_fd == -1)
	{
		zpr::fprintln(stderr, "failed to open device ('{}'): {} ({})",
			KEYBOARD_EVENT_DEVICE, strerror(errno), errno);
		exit(-1);
	}

	libevdev_set_fd(device_ev, device_fd);

	slug::loop(device_ev);

	libevdev_free(device_ev);
	close(device_fd);
}


void slug::loop(struct libevdev* device_ev)
{
	// if(auto err = libevdev_grab(device_ev, LIBEVDEV_GRAB); err != 0)
	// {
	// 	zpr::fprintln(stderr, "failed to grab device: {} ({})", strerror(err), err);
	// 	exit(-1);
	// }

	zpr::println("xkeyslug: grabbed device '{}'", KEYBOARD_EVENT_DEVICE);

	auto x_display = XOpenDisplay(0);
	if(x_display == nullptr)
	{
		zpr::fprintln(stderr, "X11 error: could not open display '{}'", XDisplayName(0));
		exit(1);
	}

	auto uinputter = slug::UInputDevice(device_ev);

	signal(SIGINT, [](int) {
		zpr::println("xkeyslug: quitting");
		g_quit.store(true, std::memory_order_relaxed);
	});

	while(not g_quit.load(std::memory_order_relaxed))
	{
		struct input_event event {};
		auto r = libevdev_next_event(device_ev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, &event);
		if(r < 0)
			zpr::fprintln(stderr, "libevdev error: {}", r);

		if(event.type != EV_KEY)
		{
			uinputter.send(event.type, event.code, event.value, /* sync: */ true);
			continue;
		}

		bool transformed = processKeyEvent(&uinputter, x_display, event.code, KeyAction { event.value });
		if(not transformed)
			uinputter.send(event.type, event.code, event.value, /* sync: */ true);
	}

	XCloseDisplay(x_display);

	// libevdev_grab(device_ev, LIBEVDEV_UNGRAB);
}
