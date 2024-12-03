#include "link.hpp"
#include <glm/common.hpp>
#include "typedefs.hpp"
#include <imgui.h>

ImColor c_link::get_health_color(const int health, const int max_health)
{
	if (health > max_health)
		return { 0.6f, 0.8f, 1.0f, 1.f }; // still not a good color but it will do for now

	const int hp = glm::max(0, glm::min(health, max_health));

	auto      i_r = static_cast<u32>(glm::min((510 * (max_health - hp)) / max_health, 200));
	auto      i_g = static_cast<u32>(glm::min((510 * hp) / max_health, 200));

	auto      f_r = (float)i_r;
	auto      f_g = (float)i_g;

	f_r = f_r / 255.f;
	f_g = f_g / 255.f;

	return { f_r, f_g, 0.f, 1.f };
}

ImColor c_link::get_team_color(tf_entity* entity)
{
	switch (entity->team_num()) {
	case 2:
		return TEAM_RED_COLOR;

	case 3:
		return TEAM_BLU_COLOR;

	default:
		return TEAM_SPEC_COLOR;
	}
}

ImColor c_link::get_team_novis_color(tf_entity* entity)
{
	return ImColor();
}
