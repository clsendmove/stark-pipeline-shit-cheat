#include "interfaces.hpp"

#include "shared.hpp" // im scared of what the compiler will say

client_class* c_hl_client::get_all_classes(void) {
    return memory::find_vfunc<client_class * (__thiscall*)(void*)>(this, 8u)(this);
}

void c_hl_client::create_move(int sequence_number, float input_sample_frametime, bool active) {
    memory::find_vfunc<void(__thiscall*)(void*, int, float, bool)>(this, 21u)(this, sequence_number, input_sample_frametime, active);
}

bool c_hl_client::write_cmd_to_buffer(bf_write* buf, int from, int to, bool is_new_cmd) {
    return memory::find_vfunc<bool(__thiscall*)(void*, bf_write*, int from, int to, bool is_new_cmd)>(
        this, 23u)(this, buf, from, to, is_new_cmd);
}

bool c_hl_client::get_player_view(c_view_setup& player_view)
{
    return CALL_VFUNC(bool, 59, c_view_setup&)(this, player_view);
}

void c_engine_client::get_screen_size(int& w, int& h) {
    memory::find_vfunc<void(__thiscall*)(void*, int&, int&)>(this, 5u)(this, w, h);
}


void c_engine_client::client_cmd_unrestricted(const char* input) {
    return memory::find_vfunc< void(__thiscall*)(void*, const char*) >(this, 106u)(this, input);
}

int c_engine_client::get_local_player(void) {
    return memory::find_vfunc<int(__thiscall*)(void*)>(this, 12u)(this);
}

float c_engine_client::plat_float_time(void) {
    return memory::find_vfunc<float(__thiscall*)(void*)>(this, 14u)(this);
}

void c_engine_client::get_view_angles(QAngle& va) {
    memory::find_vfunc<void(__thiscall*)(void*, QAngle&)>(this, 19u)(this, va);
}

vector c_engine_client::get_view_angles()
{
    vector out{};

    get_view_angles(out);

    return out;
}

void c_engine_client::set_view_angles(QAngle& va) {
    memory::find_vfunc<void(__thiscall*)(void*, QAngle&)>(this, 20u)(this, va);
}

int c_engine_client::get_max_clients(void) {
    return memory::find_vfunc<int(__thiscall*)(void*)>(this, 21u)(this);
}

bool c_engine_client::is_in_game(void) {
    return memory::find_vfunc<bool(__thiscall*)(void*)>(this, 26u)(this);
}

bool c_engine_client::is_connected(void) {
    return memory::find_vfunc<bool(__thiscall*)(void*)>(this, 27u)(this);
}

const char* c_engine_client::get_game_directory(void) {
    return memory::find_vfunc<const char* (__thiscall*)(void*)>(this, 35u)(this);
}

const matrix4x4& c_engine_client::world_to_screen_matrix(void) {
    return memory::find_vfunc<const matrix4x4 & (__thiscall*)(void*)>(this, 36u)(this);
}

bool c_engine_client::get_player_info(int entidx, player_info_t* info) {
    return memory::find_vfunc<bool(__thiscall*)(void*, int, player_info_t*)>(this, 8u)(this, entidx, info);
}

void i_input::cam_to_third_person() {
    memory::find_vfunc<void(__thiscall*)(void*)>(this, 32u)(this);
}

void i_input::cam_to_first_person() {
    memory::find_vfunc<void(__thiscall*)(void*)>(this, 33u)(this);
}

void c_prediction::run_command(tf_entity* ent, c_user_cmd* cmd, i_move_helper* move_helper)
{
    memory::find_vfunc<void(__thiscall*)(void*, tf_entity*, c_user_cmd*, i_move_helper*)>(this, 17u)(
        this, ent, cmd, move_helper);
}

void c_studio_render::set_color_modulation(const float* color) {
    memory::find_vfunc<void(__thiscall*)(void*, const float*)>(this, 27)(this, color);
}

void c_studio_render::set_alpha_modulation(float alpha) {
    memory::find_vfunc<void(__thiscall*)(void*, float)>(this, 28)(this, alpha);
}

void c_studio_render::forced_material_override(i_material* material, override_type_t override_type) {
    memory::find_vfunc<void(__thiscall*)(void*, i_material*, override_type_t)>(this, 33)(this, material,
        override_type);
}

void i_material::set_material_var_flag(material_var_flags_t flag, bool state) {
    memory::find_vfunc<void(__thiscall*)(void*, material_var_flags_t, bool)>(this, 29)(this, flag, state);
}

i_material* c_material_system::find_material(std::string_view name, std::string_view group, bool complain,
    const char* complain_prefix) {
    return memory::find_vfunc<i_material * (__thiscall*)(void*, const char*, const char*, bool, const char*)>(
        this, 73u)(this, name.data(), group.data(), complain, complain_prefix);
}

model_t* c_model_info_client::get_model(int entidx) {
    return memory::find_vfunc<model_t * (__thiscall*)(void*, int)>(this, 1u)(this, entidx);
}

studiohdr_t* c_model_info_client::get_studio_model(const model_t* model) {
    return memory::find_vfunc<studiohdr_t * (__thiscall*)(void*, const model_t*)>(this, 28u)(this, model);
}

const char* c_model_info_client::get_model_name(const model_t* model)
{
    return CALL_VFUNC(const char*, 3u, const model_t*)(this, model);
}

void c_render_view::get_matrices_for_view(const c_view_setup* view, D3DMATRIX* world_to_view, D3DMATRIX* view_to_projection, D3DMATRIX* world_to_projection, D3DMATRIX* world_to_pixels)
{
    return memory::find_vfunc<void(__thiscall*)(void*, const c_view_setup*, D3DMATRIX*, D3DMATRIX*, D3DMATRIX*, D3DMATRIX*)>(this, 50)(this, view, world_to_view, view_to_projection, world_to_projection, world_to_pixels);
}

const c_view_setup* c_view_setup::get_view_setup()
{
    return memory::find_vfunc<const c_view_setup * (__thiscall*)(void*)>(this, 13)(this);
}
