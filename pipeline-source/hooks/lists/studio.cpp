#include "../hooks.hpp"
#include "link.hpp"
#include "console/console.h"
#include <safetyhook.hpp>

SafetyHookInline dmod{};

// TODO: Pixy; recode this, this has completely dog shit logic that I hate
void vf hooks::studio::draw_model(registers, draw_model_results_t* results, draw_model_info_t* info,
    matrix3x4* bone2world, float* weights, float* delayed, vec3* origin,
    int flags) {
    dmod.fastcall<void>(rcx, results, info, bone2world, weights, delayed, origin, flags);
}

void hooks::studio::startup() {
    console.print("trying to hook draw_model");
    dmod = safetyhook::create_inline(memory::vfunc_ptr(studio_render, 29), draw_model);
    console.print("hooked draw_model");
}

void hooks::studio::shutdown() {
    dmod = {};
}