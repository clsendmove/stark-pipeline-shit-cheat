#include "entities.hpp"

bool tf_entity::setup_bones(matrix3x4* out, int max, int mask, float time)
{
    const auto renderable = this->get_client_renderable();
    return memory::find_vfunc<bool(__thiscall*)(void*, matrix3x4*, int, int, float)>(renderable, 16)(
        renderable, out, max, mask, time);
}

vector tf_entity::get_hitbox_position(const int hitbox) {
    const auto& model = get_model();
    if (!model)
        return vector();
    const auto& hdr = model_info_client->get_studio_model(model);
    if (!hdr)
        return vector();
    const auto& set = hdr->get_hitbox_set(hitbox_set());
    if (!set)
        return vector();
    matrix3x4 matrix[128];
    if (!setup_bones(matrix, 128, BONE_USED_BY_ANYTHING, sim_time()))
        return vector();
    const auto& box = set->pHitbox(hitbox);
    if (!box)
        return vector();
    vector out;
    vector pos = (box->bbmin + box->bbmax) * 0.5f;
    math::vector_transform(pos, matrix[box->bone], out);
    return out;
}
int tf_entity::get_num_of_hitboxes() {
    const auto& model = get_model();
    if (!model) return 0;
    const auto& hdr = model_info_client->get_studio_model(model);
    if (!hdr) return 0;
    const auto& set = hdr->get_hitbox_set(hitbox_set());
    if (!set) return 0;

    return set->numhitboxes;
}

vector tf_entity::eye_position()
{
    return this->get_abs_origin() + this->view_offset();
}


tf_weapon* tf_player::get_active_weapon()
{
    if (active_weapon())
        return entity_list->get_client_entity_from_handle(this->active_weapon())->as<tf_weapon>();

    return nullptr;
}