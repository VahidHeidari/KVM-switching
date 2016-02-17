/**
 * Switching is a utility to switch KVM switch with mouse movements.
 *
 * Copyright (C) 2016  Vahid Heidari (DeltaCode)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Library and example found at
 * 		http://stackoverflow.com/questions/3230761/how-to-change-keyboard-layout-a-x11-api-solution
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <iomanip>

#include <syslog.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "xkb/XKeyboard.h"
#include "xkb/X11Exception.h"

#include "SingletonApplication.h"

using namespace std;

struct Point
{
	int x;
	int y;

	Point() = default;
	Point(int x, int y) : x(x), y(y) {}
	Point(const Point& p) : x(p.x), y(p.y) {}

	ostream& print(ostream& s) const
	{
		s << setw(4) << x << ", " << setw(4) << y;
		return s;
	}

	double length() const
	{
		return sqrt(x * x + y * y);
	}

	static double distance(const Point p1, const Point p2)
	{
		int x = p2.x - p1.x;
		int y = p2.y - p1.y;
		return sqrt(x * x + y * y);
	}

	Point operator =(const Point& p)
	{
		x = p.x;
		y = p.y;
		return *this;
	}

	Point operator -(const Point& r)
	{
		Point p = Point(x - r.x, y - r.y);
		return p;
	}

	Point operator +(const Point& r)
	{
		Point p = Point(x + r.x, y + r.y);
		return p;
	}
};

ostream& operator <<(ostream& s, const Point& p)
{
	return p.print(s);
}

typedef Point MousePos;

struct Rectangle
{
	int x;
	int y;
	int width;
	int height;

	bool contains(const Point& p) const
	{
		return (p.x >= x && p.x <= x + width
				&& p.y >= y && p.y <= y + height);
	}
};

struct Config
{
	Rectangle rect;
	int checking_interval;
	int layout_change_delay;
	float move_speed;

	bool read_config(string path = "config.cfg")
	{
		ifstream input(path);

		if (!input.is_open())
			return false;

		input >> rect.x >> rect.y >> rect.width >> rect.height;
		input >> checking_interval;
		input >> layout_change_delay;
		input >> move_speed;

		return true;
	}
};

Config config;
Display* dpy = nullptr;
Window root;

template <bool PRINT_DEBUG = false>
bool check_mouse_pos()
{
	static MousePos last_pos;

	Window ret_root;
	Window ret_child;
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	unsigned int mask;

	if (XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask)) {
		MousePos current_pos(root_x, root_y);

		// Calculate velocity of cursor. (Average velocity = Delta Movement / Delta Time)
		double d = (last_pos - current_pos).length();
		double v = d / (config.checking_interval / 1000000.0);

		if (PRINT_DEBUG) {
			cout << "last:" << last_pos
				<< "        current:" << current_pos
				<< "       velocity:" << setw(4) << (int)v;

			if (config.rect.contains(last_pos))
				cout << "      MOUSE IS IN THE AREA";

			cout << endl;
		}

		last_pos = current_pos;

		if (v > config.move_speed && config.rect.contains(last_pos))
			return true;
	}

	return false;
}

template <bool PRINT_DEBUG = false>
void switch_desktop(XKeyboard& xkb, int delay_us)
{
	int group_count = xkb.groupCount();
	int current_group = xkb.currentGroupNum();

	if (PRINT_DEBUG)
		cout << "Switch desktop" << endl;

	xkb.setGroupByNum((current_group + 1) % group_count);
	usleep(delay_us);
	xkb.setGroupByNum(current_group);
	usleep(delay_us);
}

void log_info(const char* message)
{
	syslog(LOG_INFO, "%s", message);
}

void log_error(const char* message)
{
	syslog(LOG_ERR, "%s", message);
}

void exit_error(int ret_val, const char* message)
{
	log_error(message);
	log_info("Switching terminated with and error.");
	closelog();
	exit(ret_val);
}

int main()
{
	cout << "Switching test program had been started." << endl;
	openlog(nullptr, LOG_PERROR | LOG_PID, LOG_USER);
	log_info("Switching started.");

	if (!config.read_config())
		exit_error(1, "Could not read config file!");

	try {
		SingletonApplication app("\'KVM Switching\'");

		dpy = XOpenDisplay(nullptr);
		root = XDefaultRootWindow(dpy);

		XKeyboard xkb;

		StringVector name = xkb.groupNames();
		StringVector syms = xkb.groupSymbols();

		int group_count = xkb.groupCount();
		cout << "Group count : " << group_count << endl;
		for (int i = 0 ; i < group_count; ++i)
		    cout << "    Group " << i << " : " << name[i] << "  " << syms[i] << endl;
		
		if (group_count < 2)
			exit_error(1, "We need atleast 2 layout groups!");

		cout << "Switch test!" << endl;
		while (true) {
			if (check_mouse_pos<true>()) {
				switch_desktop<true>(xkb, config.layout_change_delay);
			}
			usleep(config.checking_interval);
		}

	} catch (SingletonException* e) {
		exit_error(1, e->what());
	} catch (X11Exception* e) {
		log_error(e->what());
		delete e;

		return 1;
	}

	log_info("Switching successfully terminated.");
	closelog();
	return 0;
}
