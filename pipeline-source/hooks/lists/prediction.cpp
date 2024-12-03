#include "link.hpp"
#include "../hooks.hpp"
#include "console/console.h"
#include <safetyhook.hpp>

SafetyHookInline pred{};

void  vf hooks::_prediction::run_command(void* rcx, tf_entity* ent, c_user_cmd* ucmd, i_move_helper* _move_helper) {

	if (!move_helper) {
		move_helper = _move_helper;
	}

	pred.fastcall<void>(rcx, ent, ucmd, _move_helper);
}

void hooks::_prediction::startup() {
	pred = safetyhook::create_inline(memory::vfunc_ptr(prediction, 17), run_command);
	console.print("hooked run_command");
}

void hooks::_prediction::shutdown() {
	pred = {};
}