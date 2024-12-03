#include "../hooks.hpp"
#include "link.hpp"

#include <safetyhook.hpp>
#include <valve/sdk/shared.hpp>
#include "console/console.h"
#include <imgui.h>

SafetyHookInline windowproc{}, l_cursor{}, p_trav{};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
    LPARAM lParam);
void vf hooks::_surface::lock_cursor(registers) {
    if (config->gui.is_open) {
        surface->unlock_cursor();
    }
    else {
        l_cursor.fastcall<void>(rcx);
    }
}

void vf hooks::_surface::paint_traverse(registers, u32 _panel, bool repaint, bool force) {
    const auto panel_name = panel->get_name(_panel);

    if (HASH_RT(panel_name) == HASH_RT("HudScope"))
        return;

    p_trav.fastcall<void>(rcx, _panel, repaint, force);
}

LRESULT api hooks::_surface::matsurfacewindowproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (config->gui.is_open) {
        ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
        if (ImGui::GetIO().WantTextInput) {
            input_system->reset_input_state();
            return 1;
        }
        if (uMsg >= WM_MOUSEFIRST && WM_MOUSELAST >= uMsg)
            return 1;
    }

    return windowproc.stdcall<LRESULT>(hwnd, uMsg, wParam, lParam);
}

void hooks::_surface::startup() {
    console.print("trying to hook matsurfacewindowproc");
    windowproc = safetyhook::create_inline(memory::find_pattern("vguimatsurface.dll", "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8B EC 48 83 EC 50"), matsurfacewindowproc); // this may be whats causing a crash, check into it later - drev
    console.print("hooked matsurfacewindowproc");
    console.print("trying to hook lock_cursor");
    l_cursor = safetyhook::create_inline(memory::vfunc_ptr(surface, 62), lock_cursor);
    console.print("hooked lock_cursor");
    console.print("trying to hook paint_traverse");
    p_trav = safetyhook::create_inline(memory::vfunc_ptr(panel, 41), paint_traverse);
    console.print("hooked paint_traverse");
}

void hooks::_surface::shutdown() {
    windowproc = {};
    l_cursor = {};
    p_trav = {};
}

