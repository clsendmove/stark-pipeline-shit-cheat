#include "hooks.hpp"
#include "console/console.h"

using namespace hooks;

void c_hook_manager::startup_hooks()
{
	directx::startup();
	console.print("hooked directx complety");
	_client::startup();
	console.print("hooked client complety");
	_surface::startup();
	console.print("hooked surface complety");
	studio::startup();
	console.print("hooked studio complety");
	_prediction::startup();
	console.print("hooked prediction complety");
	
	
	
}

void c_hook_manager::shutdown_hooks()
{
	directx::shutdown();
	_prediction::shutdown();
	_client::shutdown();
	_surface::shutdown();
	studio::shutdown();
}
