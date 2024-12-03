#include "../hooks.hpp"

#include <safetyhook.hpp>
#include <valve/sdk/shared.hpp>
#include <drawsystem/drawsystem.hpp>

SafetyHookInline fsn{}, cr_move{}, c_move{}, o_view{}, pure{};


void vf          hooks::_client::frame_stage_notify(registers, frame_stage stage) {
    fsn.fastcall<void>(rcx, stage);

    switch (stage) {
    case FRAME_UNDEFINED:
    case FRAME_START:
    case FRAME_NET_UPDATE_START:
    case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
        break;
    case FRAME_NET_UPDATE_POSTDATAUPDATE_END: {
        break;
    }
    case FRAME_NET_UPDATE_END:
        break;
    case FRAME_RENDER_START: {
        if (!tf2.screenwidth || !tf2.screenheight)
            break;

        draw->string(fonts::FONT_SEGOE, 5, 5, true, e_text_align::TEXT_LEFT, { 255,255,255,255 }, "pipeline v2");

        draw->flush();
        break;
    }
    case FRAME_RENDER_END:
    default:
        break;
    }
}


bool vf hooks::_client::create_move(registers, float sample_time, c_user_cmd* cmd) {
    const auto result = cr_move.fastcall<bool>(rcx, sample_time, cmd);

    if (!cmd->command_number) // do not let createmove run every frame
        return result;

    if (result) {
        engine_client->set_view_angles(cmd->viewangles);
    }

    cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & INT_MAX;

    if (local) {

       
       
        prediction->first_time_predicted = false;
        prediction->in_prediction = true;
        if (prediction->in_prediction) {
            prediction->run_command(local, cmd, move_helper);
            //run aimbot
        }
        prediction->in_prediction = false;
        prediction->first_time_predicted = true;
    }

    // prevent createmove from snapping view angles
    return false;
}

void __cdecl hooks::_client::cl_move(float samples, bool final_tick) {
    c_move.call<void>(samples, final_tick);
}

void vf hooks::_client::override_view(registers, c_view_setup* setup) {
    o_view.fastcall<void>(rcx, setup);
}

void __cdecl hooks::_client::check_for_pure_server_whitelist(void **files) {
}


void hooks::_client::startup() {
    console.print("trying to hook check_for_pure_server_whitelist");
   pure = safetyhook::create_inline(memory::find_pattern("engine.dll", "40 56 48 83 EC ? 83 3D ? ? ? ? ? 48 8B F1 0F 8E"),check_for_pure_server_whitelist);
    console.print("hooked check_for_pure_server_whitelist");
    console.print("trying to hook frame_stage_notify");
    fsn = safetyhook::create_inline(memory::vfunc_ptr(client, 35), frame_stage_notify);
    console.print("hooked frame_stage_notify");
    console.print("trying to hook create_move");
    cr_move = safetyhook::create_inline(memory::vfunc_ptr(client_mode, 21), create_move);
    console.print("hooked create_move");
    console.print("trying to hook cl_move");
   c_move = safetyhook::create_inline(memory::find_pattern("engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D"),cl_move);
    console.print("hooked cl_move");
   console.print("trying to hook override_view");
    o_view = safetyhook::create_inline(memory::vfunc_ptr(client_mode, 16), override_view);
    console.print("hooked override_view");
}

void hooks::_client::shutdown() {
    pure = {};
    fsn = {};
    cr_move = {};
    o_view = {};
    c_move = {};
}