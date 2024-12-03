#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

#include "../hooks/hooks.hpp"


#include <windows.h>
#include <thread>
#include <chrono>
#include <cstdint>
#include <array>
#include <algorithm>
#include <string>
#include <string_view>
#include <intrin.h>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <winternl.h>
#include <ImageHlp.h>
#include <vector>
#include <list>
#include <mmsystem.h>
#include <fstream>
#include <string>
#include <filesystem>
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "imagehlp.lib")


class c_factory {
public:
	void startup();
	void shutdown();

	bool should_unload();
	void print_on_inject();

	int dx_level = 0;
	std::string configpath = {};
	bool failed = false;
};

singleton(factory, c_factory);
//singleton(g_hooks, c_hook_manager);