
#pragma once
#include "valve/sdk/shared.hpp"
#include <imgui.h>
#include <chrono>

class c_link {
public:
	ImColor           get_health_color(const int health, const int max_health);

	ImColor           get_team_color(tf_entity* entity);
	ImColor           get_team_novis_color(tf_entity* entity);


};


class c_config {
public:
	struct {
		bool bunnyhop = true;
		bool autostrafe = true;
		float retrack_speed = 0.5f;
	} movement;

	struct {
		bool is_open = false;
	} gui;


};

singleton(link, c_link);
singleton(config, c_config);