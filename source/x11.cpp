// x11.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "slug.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace slug
{
	WindowInfo getCurrentWindowInfo(Display* x_display)
	{
		Window focused_window {};
		int revert_to = 0;
		XGetInputFocus(x_display, &focused_window, &revert_to);

	retry:
		XClassHint hints {};
		if(XGetClassHint(x_display, focused_window, &hints) == BadWindow)
			return {};

		auto name_str = hints.res_name == nullptr ? std::string{} : std::string(hints.res_name);
		auto class_str = hints.res_class == nullptr ? std::string{} : std::string(hints.res_class);

		if(hints.res_name != nullptr)  XFree(hints.res_name);
		if(hints.res_class != nullptr) XFree(hints.res_class);

		// https://github.com/JetBrains/jdk8u_jdk/blob/master/src/solaris/classes/sun/awt/X11/XFocusProxyWindow.java#L35
		if((name_str.empty() && class_str.empty()) || class_str.find("FocusProxy") != std::string::npos)
		{
			// we don't care about any of these, but we still need to provide them...
			Window root_window {};
			Window parent_window {};
			Window* children {};
			unsigned int num_children = 0;
			XQueryTree(x_display, focused_window, /* root: */ &root_window, &parent_window, &children, &num_children);

			focused_window = parent_window;
			goto retry;
		}
		else
		{
			return { .wm_name = name_str, .wm_class = class_str };
		}
	}

	bool matchWindowClass(Display* x_display, std::string_view window_class)
	{
		return getCurrentWindowInfo(x_display).wm_class == window_class;
	}
}
