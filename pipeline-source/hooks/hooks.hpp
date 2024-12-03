#pragma once

#include "link.hpp"

#include <d3d9.h>
#include <intrin.h>

namespace hooks {

    namespace directx {
        HRESULT api present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND window,
            const RGNDATA* dirty);
        void        startup();
        void        shutdown();
    } // namespace directx

    namespace _prediction {
        void vf run_command(registers, tf_entity* ent, c_user_cmd* ucmd, i_move_helper* _move_helper);
        void startup();
        void shutdown();
    }

    namespace _client {
        void __cdecl cl_move(float samples, bool final_tick);
        void vf frame_stage_notify(registers, frame_stage stage);
        bool vf create_move(registers, float sample_time, c_user_cmd* cmd);
        void vf override_view(registers, c_view_setup* setup);
        void __cdecl check_for_pure_server_whitelist(void **files);
        void    startup();
        void    shutdown();
    } // namespace client

    namespace _surface {
        void vf lock_cursor(registers);
        void vf paint_traverse(registers, u32 panel, bool repaint, bool force);
        LRESULT api matsurfacewindowproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        void    startup();
        void    shutdown();
    } // namespace surface

    namespace studio {
        void vf draw_model(registers, draw_model_results_t* results, draw_model_info_t* info, matrix3x4* bone2world,
            float* weights, float* delayed, vec3* origin, int flags);
        void    startup();
        void    shutdown();
    } // namespace studio

} // namespace hooks

class c_hook_manager {
public:
    void startup_hooks();
    void shutdown_hooks();
};

singleton(hook_manager, c_hook_manager);