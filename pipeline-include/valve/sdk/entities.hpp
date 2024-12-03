#pragma once

#include "shared.hpp"
#include "interfaces.hpp"
#include "netvars.h"

class c_mouth_info;
struct spatialization_info_t;

class i_client_entity : public i_client_unknown,
    public i_client_renderable,
    public i_client_networkable,
    public i_client_thinkable {
public:
    virtual void                    release(void) = 0;
    virtual const vector& get_abs_origin(void) const = 0;
    virtual const QAngle& get_abs_angles(void) const = 0;
    virtual c_mouth_info* get_mouth(void) = 0;
    virtual bool                    get_sound_spatialization(spatialization_info_t& info) = 0;

    template <typename T> inline T* as() { return reinterpret_cast<T*>(this); }
};

enum e_should_transmit_state {
    SHOULDTRANSMIT_START = 0,
    SHOULDTRANSMIT_END
};

enum e_data_update_type {
    DATA_UPDATE_CREATED = 0,
    DATA_UPDATE_DATATABLE_CHANGED,
};

class i_client_networkable {
public:
    virtual i_client_unknown* get_i_client_unknown() = 0;
    virtual void              release() = 0;
    virtual client_class* get_client_class() = 0;
    virtual void              notify_should_transmit(e_should_transmit_state state) = 0;
    virtual void              on_pre_data_changed(e_data_update_type update_type) = 0;
    virtual void              on_data_changed(e_data_update_type update_type) = 0;
    virtual void              pre_data_update(e_data_update_type update_type) = 0;
    virtual void              post_data_update(e_data_update_type update_type) = 0;
    virtual bool              is_dormant(void) = 0;
    virtual int               entidx(void) const = 0;
    virtual void              receive_message(int class_id, bf_read& msg) = 0;
    virtual void* get_data_table_base_ptr() = 0;
    virtual void              set_destroyed_on_recreate_entities(void) = 0;
    virtual void              on_data_unchanged_in_pvs() = 0;
    inline e_class_ids        get_class_id() {
        if (auto client_class = get_client_class())
            return static_cast<e_class_ids>(client_class->m_ClassID);

        return static_cast<e_class_ids>(0);
    }
};


class i_client_networkable;
class c_base_entity;
class i_client_renderable;
class i_collideable;
class i_client_entity;
class i_client_thinkable;

// straight out of the hl2src leak
class i_client_unknown : public i_handle_entity {
public:
    virtual i_collideable* get_collideable() = 0;
    virtual i_client_networkable* get_client_networkable() = 0;
    virtual i_client_networkable* get_client_renderable() = 0;
    virtual i_client_entity* get_i_client_entity() = 0;
    virtual c_base_entity* get_base_entity() = 0;
    virtual i_client_thinkable* get_client_thinkable() = 0;
};

class c_client_think_handle_ptr;

typedef c_client_think_handle_ptr* client_think_handle_t;

class i_client_thinkable {
public:
    virtual i_client_unknown* get_i_client_unknown() = 0;
    virtual void                  client_think() = 0;
    virtual client_think_handle_t get_think_handle() = 0;
    virtual void                  set_think_handle(client_think_handle_t think) = 0;
    virtual void                  release() = 0;
};


typedef unsigned short client_shadow_handle;
typedef unsigned short client_render_handle;
typedef unsigned short model_instance_handle;

struct model_t;

class i_pvs_notify {
public:
    virtual void on_pvs_status_changed(bool pvs) = 0;
};

enum e_shadow_type {
    SHADOWS_NONE = 0,
    SHADOWS_SIMPLE,
    SHADOWS_RENDER_TO_TEXTURE,
    SHADOWS_RENDER_TO_TEXTURE_DYNAMIC,
    SHADOWS_RENDER_TO_DEPTH_TEXTURE,
};

class i_client_renderable {
public:
    virtual i_client_unknown* get_i_client_unknown() = 0;
    virtual vector const& get_render_origin(void) = 0;
    virtual QAngle const& get_render_angles(void) = 0;
    virtual bool                  should_draw(void) = 0;
    virtual bool                  is_transparent(void) = 0;
    virtual bool                  uses_power_of_two_frame_buffer_texture() = 0;
    virtual bool                  uses_full_frame_buffer_texture() = 0;
    virtual client_shadow_handle  get_shadow_handle() const = 0;
    virtual client_shadow_handle& render_handle() = 0;
    virtual const model_t* get_model() const = 0;
    virtual int                   draw_model(int flags) = 0;
    virtual int                   get_body() = 0;
    virtual void                  compute_fx_blend() = 0;
    virtual int                   get_fx_blend(void) = 0;
    virtual void                  get_color_modulation(float* color) = 0;
    virtual bool                  lod_test() = 0;
    virtual bool          setup_bones(matrix3x4* bone_to_world_out, int max_bones, int bone_mask,
        float current_time) = 0;
    virtual void          setup_weights(const matrix3x4* bone_to_world, int flex_weight_count,
        float* flex_weights, float* flex_delayed_weights) = 0;
    virtual void          do_animation_events(void) = 0;
    virtual i_pvs_notify* get_pvs_notify_instance() = 0;
    virtual void          get_render_bounds(vector& mins, vector& maxs) = 0;
    virtual void          get_render_bounds_worldspace(vector& mins, vector& maxs) = 0;
    virtual void get_shadow_render_bounds(vector& mins, vector& maxs, e_shadow_type shadow_type) = 0;
    virtual bool should_receive_projected_textures(int flags) = 0;
    virtual bool get_shadow_cast_distance(float* dist, e_shadow_type shadow_type) const = 0;
    virtual bool get_shadow_cast_direction(vector* direction, e_shadow_type shadow_type) const = 0;
    virtual bool is_shadow_dirty() = 0;
    virtual void mark_shadow_dirty(bool dirty) = 0;
    virtual i_client_renderable* get_shadow_parent() = 0;
    virtual i_client_renderable* first_shadow_child() = 0;
    virtual i_client_renderable* next_shadow_peer() = 0;
    virtual e_shadow_type         shadow_cast_type() = 0;
    virtual void                  create_model_instance() = 0;
    virtual model_instance_handle get_model_instance() = 0;
    virtual const matrix3x4& renderable_to_world_transform() = 0;
    virtual int                   lookup_attachment(const char* attachment_name) = 0;
    virtual bool                  get_attachment(int number, vector& origin, QAngle& angles) = 0;
    virtual bool                  get_attachment(int number, matrix3x4& matrix) = 0;
    virtual float* get_render_clip_plane(void) = 0;
    virtual int                   get_skin() = 0;
    virtual bool                  is_two_pass(void) = 0;
    virtual void                  on_threaded_draw_setup() = 0;
    virtual bool                  uses_flex_delayed_weights() = 0;
    virtual void                  record_tool_message() = 0;
    virtual bool                  ignores_z_buffer(void) const = 0;
};

class tf_dispenser : public tf_entity {
public:
    netvar(health, int, "DT_ObjectDispenser", "m_iHealth");
    netvar(state, int, "DT_ObjectDispenser", "m_iState");
    netvar(ammo_metal, int, "DT_ObjectDispenser", "m_iAmmoMetal");

    int max_metal() { return is_mini_building() ? MINI_DISPENSER_MAX_METAL : DISPENSER_MAX_METAL_AMMO; }
};

inline std::vector<tf_dispenser*> dispensers = { };

#include <unordered_map>
#define TF_AMMOPACK_SMALL_BDAY		"models/items/ammopack_small_bday.mdl"
#define TF_AMMOPACK_MEDIUM_BDAY		"models/items/ammopack_medium_bday.mdl"
#define TF_AMMOPACK_LARGE_BDAY		"models/items/ammopack_large_bday.mdl"
#define TF_AMMOPACK_SMALL "models/items/ammopack_small.mdl"
#define TF_AMMOPACK_MEDIUM "models/items/ammopack_medium.mdl"
#define TF_AMMOPACK_LARGE "models/items/ammopack_large.mdl"

#define TF_MEDKIT_SMALL_BDAY	"models/items/medkit_small_bday.mdl"
#define TF_MEDKIT_MEDIUM_BDAY	"models/items/medkit_medium_bday.mdl"
#define TF_MEDKIT_LARGE_BDAY	"models/items/medkit_large_bday.mdl"
#define TF_MEDKIT_SMALL_HALLOWEEN	"models/props_halloween/halloween_medkit_small.mdl"
#define TF_MEDKIT_MEDIUM_HALLOWEEN  "models/props_halloween/halloween_medkit_medium.mdl"
#define TF_MEDKIT_LARGE_HALLOWEEN	"models/props_halloween/halloween_medkit_large.mdl"
#define LUNCHBOX_DROP_MODEL  "models/items/plate.mdl"
#define LUNCHBOX_STEAK_DROP_MODEL  "models/items/plate_steak.mdl"
#define LUNCHBOX_ROBOT_DROP_MODEL  "models/items/plate_robo_sandwich.mdl"
#define LUNCHBOX_FESTIVE_DROP_MODEL  "models/items/plate_sandwich_xmas.mdl"
#define TF_MEDKIT_SMALL "models/items/medkit_small.mdl"
#define TF_MEDKIT_MEDIUM "models/items/medkit_medium.mdl"
#define TF_MEDKIT_LARGE "models/items/medkit_large.mdl"

#define TF_POWERUP_AGILITY "models/pickups/pickup_powerup_agility.mdl"
#define TF_POWERUP_CRIT "models/pickups/pickup_powerup_crit.mdl"
#define TF_POWERUP_DEFENSE "models/pickups/pickup_powerup_defense.mdl"
#define TF_POWERUP_HASTE "models/pickups/pickup_powerup_haste.mdl"
#define TF_POWERUP_KING "models/pickups/pickup_powerup_king.mdl"
#define TF_POWERUP_KNOCKOUT "models/pickups/pickup_powerup_knockout.mdl"
#define TF_POWERUP_PLAGUE "models/pickups/pickup_powerup_plague.mdl"
#define TF_POWERUP_PRECISION "models/pickups/pickup_powerup_precision.mdl"
#define TF_POWERUP_REFLECT "models/pickups/pickup_powerup_reflect.mdl"
#define TF_POWERUP_REGEN "models/pickups/pickup_powerup_regen.mdl"
#define TF_POWERUP_RESISTANCE "models/pickups/pickup_powerup_resistance.mdl"
#define TF_POWERUP_STRENGTH "models/pickups/pickup_powerup_strength.mdl"
#define TF_POWERUP_SUPERNOVA "models/pickups/pickup_powerup_supernova.mdl"
//#define TF_POWERUP_THORNS "models/pickups/pickup_powerup_thorns.mdl"		UNUSED
//#define TF_POWERUP_UBER "models/pickups/pickup_powerup_uber.mdl"			UNUSED
#define TF_POWERUP_VAMPIRE "models/pickups/pickup_powerup_vampire.mdl"

#define TF_BOT_SENTRY_BUSTER "models/bots/demo/bot_sentry_buster.mdl"

class tf_entity : public i_client_entity {
public:
    netvar(is_dead, bool, "DT_BasePlayer", "deadflag");
    netvar(flags, int, "DT_BasePlayer", "m_fFlags");
    netvar(mins, vector, "DT_BaseEntity", "m_vecMins");
    netvar(maxs, vector, "DT_BaseEntity", "m_vecMaxs");
    netvar(sim_time, float, "DT_BaseEntity", "m_flSimulationTime");
    netvar(origin, vector, "DT_BaseEntity", "m_vecOrigin");
    netvar(health, int, "DT_BasePlayer", "m_iHealth");
    netvar(observer_mode, int, "DT_BasePlayer", "m_iObserverMode");
    netvar(observer_target, c_base_handle, "DT_BasePlayer", "m_hObserverTarget");
    netvar(velocity, vector, "DT_BasePlayer", "m_vecVelocity[0]");
    netvar(view_offset, vector, "DT_BasePlayer", "m_vecViewOffset[0]");
    netvar(team_num, int, "DT_BaseEntity", "m_iTeamNum");
    netvar(upgrade_level, int, "DT_BaseObject", "m_iUpgradeLevel");
    netvar(is_mini_building, bool, "DT_BaseObject", "m_bMiniBuilding");
    netvar(active_weapon, int, "DT_BaseCombatCharacter", "m_hActiveWeapon");
    netvar(glow_enabled, bool, "DT_BaseCombatCharacter", "m_bGlowEnabled");
    netvar(_condition_bits, int, "DT_TFPlayer", "_condition_bits");
    netvar(player_cond, int, "DT_TFPlayer", "m_nPlayerCond");
    netvar(player_cond_ex, int, "DT_TFPlayer", "m_nPlayerCondEx");
    netvar(player_cond_ex2, int, "DT_TFPlayer", "m_nPlayerCondEx2");
    netvar(player_cond_ex3, int, "DT_TFPlayer", "m_nPlayerCondEx3");
    netvar(player_cond_ex4, int, "DT_TFPlayer", "m_nPlayerCondEx4");
    netvar(force_taunt_cam, int, "DT_TFPlayer", "m_nForceTauntCam");
    netvar(kills, int, "DT_TFPlayer", "m_iKills");
    netvar(deaths, int, "DT_TFPlayer", "m_iDeaths");
    netvar(fov, int, "DT_BasePlayer", "m_iFOV");
    netvar(hitbox_set, int, "DT_BaseAnimating", "m_nHitboxSet");
    int  max_health() { return memory::find_vfunc<int(__thiscall*)(void*)>(this, 107)(this); }
    void update_visibility() { memory::find_vfunc<void(__thiscall*)(void*)>(this, 91u)(this); }
    void update_wearables() { memory::find_vfunc<void(__thiscall*)(void*)>(this, 294)(this); }

    //https://prnt.sc/tusXkRtEU21E annoying extension
    bool setup_bones(matrix3x4* out, int max, int mask, float time);

    vector get_hitbox_position(const int hitbox);
    int get_num_of_hitboxes();

    vector get_world_space_center() {
        auto out = origin();
        out.z += (mins().z + maxs().z) * 0.5f;
        return out;
    }
    vector get_shoot_position() { return origin() + view_offset(); }

    vector eye_position();
};

class tf_weapon;
class tf_player : public tf_entity {
public:
    netvar(is_dead, bool, "DT_BasePlayer", "deadflag");
    netvar(flags, int, "DT_BasePlayer", "m_fFlags");
    netvar(health, int, "DT_BasePlayer", "m_iHealth");
    netvar(observer_mode, int, "DT_BasePlayer", "m_iObserverMode");
    netvar(observer_target, c_base_handle, "DT_BasePlayer", "m_hObserverTarget");
    netvar(velocity, vector, "DT_BasePlayer", "m_vecVelocity[0]");
    netvar(view_offset, vector, "DT_BasePlayer", "m_vecViewOffset[0]");
    netvar(upgrade_level, int, "DT_BaseObject", "m_iUpgradeLevel");
    netvar(_condition_bits, int, "DT_TFPlayer", "_condition_bits");
    netvar(player_cond, int, "DT_TFPlayer", "m_nPlayerCond");
    netvar(player_cond_ex, int, "DT_TFPlayer", "m_nPlayerCondEx");
    netvar(player_cond_ex2, int, "DT_TFPlayer", "m_nPlayerCondEx2");
    netvar(player_cond_ex3, int, "DT_TFPlayer", "m_nPlayerCondEx3");
    netvar(player_cond_ex4, int, "DT_TFPlayer", "m_nPlayerCondEx4");
    netvar(force_taunt_cam, int, "DT_TFPlayer", "m_nForceTauntCam");
    netvar(kills, int, "DT_TFPlayer", "m_iKills");
    netvar(deaths, int, "DT_TFPlayer", "m_iDeaths");
    netvar(m_bSaveMeParity, bool, "DT_BasePlayer", "m_bSaveMeParity");
    netvar(m_bIsMiniBoss, bool, "DT_BasePlayer", "m_bIsMiniBoss");
    netvar(m_bIsABot, bool, "DT_BasePlayer", "m_bIsABot");
    netvar(m_nBotSkill, int, "DT_BasePlayer", "m_nBotSkill");
    netvar(m_hRagdoll, int, "DT_BasePlayer", "m_hRagdoll");
    netvar(m_PlayerClass, void*, "DT_BasePlayer", "m_PlayerClass");
    netvar(m_iClass, int, "DT_BasePlayer", "m_iClass");
    netvar(m_iszClassIcon, const char*, "DT_BasePlayer", "m_iszClassIcon");
    netvar(m_iszCustomModel, const char*, "DT_BasePlayer", "m_iszCustomModel");
    netvar(m_vecCustomModelOffset, vector, "DT_BasePlayer", "m_vecCustomModelOffset");
    netvar(m_angCustomModelRotation, vector, "DT_BasePlayer", "m_angCustomModelRotation");
    netvar(m_bCustomModelRotates, bool, "DT_BasePlayer", "m_bCustomModelRotates");
    netvar(m_bCustomModelRotationSet, bool, "DT_BasePlayer", "m_bCustomModelRotationSet");
    netvar(m_bCustomModelVisibleToSelf, bool, "DT_BasePlayer", "m_bCustomModelVisibleToSelf");
    netvar(m_bUseClassAnimations, bool, "DT_BasePlayer", "m_bUseClassAnimations");
    netvar(m_iClassModelParity, int, "DT_BasePlayer", "m_iClassModelParity");
    netvar(m_Shared, void*, "DT_BasePlayer", "m_Shared");
    netvar(m_nPlayerCond, int, "DT_BasePlayer", "m_nPlayerCond");
    netvar(m_bJumping, bool, "DT_BasePlayer", "m_bJumping");
    netvar(m_nNumHealers, int, "DT_BasePlayer", "m_nNumHealers");
    netvar(m_iCritMult, int, "DT_BasePlayer", "m_iCritMult");
    netvar(m_iAirDash, int, "DT_BasePlayer", "m_iAirDash");
    netvar(m_nAirDucked, int, "DT_BasePlayer", "m_nAirDucked");
    netvar(m_flDuckTimer, float, "DT_BasePlayer", "m_flDuckTimer");
    netvar(m_nPlayerState, int, "DT_BasePlayer", "m_nPlayerState");
    netvar(m_iDesiredPlayerClass, int, "DT_BasePlayer", "m_iDesiredPlayerClass");
    netvar(m_flMovementStunTime, float, "DT_BasePlayer", "m_flMovementStunTime");
    netvar(m_iMovementStunAmount, int, "DT_BasePlayer", "m_iMovementStunAmount");
    netvar(m_iMovementStunParity, int, "DT_BasePlayer", "m_iMovementStunParity");
    netvar(m_hStunner, tf_entity, "DT_BasePlayer", "m_hStunner");
    netvar(m_iStunFlags, int, "DT_BasePlayer", "m_iStunFlags");
    netvar(m_nArenaNumChanges, int, "DT_BasePlayer", "m_nArenaNumChanges");
    netvar(m_bArenaFirstBloodBoost, bool, "DT_BasePlayer", "m_bArenaFirstBloodBoost");
    netvar(m_iWeaponKnockbackID, int, "DT_BasePlayer", "m_iWeaponKnockbackID");
    netvar(m_bLoadoutUnavailable, bool, "DT_BasePlayer", "m_bLoadoutUnavailable");
    netvar(m_iItemFindBonus, int, "DT_BasePlayer", "m_iItemFindBonus");
    netvar(m_bShieldEquipped, bool, "DT_BasePlayer", "m_bShieldEquipped");
    netvar(m_bParachuteEquipped, bool, "DT_BasePlayer", "m_bParachuteEquipped");
    netvar(m_iNextMeleeCrit, int, "DT_BasePlayer", "m_iNextMeleeCrit");
    netvar(m_iDecapitations, int, "DT_BasePlayer", "m_iDecapitations");
    netvar(m_iRevengeCrits, int, "DT_BasePlayer", "m_iRevengeCrits");
    netvar(m_iDisguiseBody, int, "DT_BasePlayer", "m_iDisguiseBody");
    netvar(m_hCarriedObject, int, "DT_BasePlayer", "m_hCarriedObject");
    netvar(m_bCarryingObject, bool, "DT_BasePlayer", "m_bCarryingObject");
    netvar(m_flNextNoiseMakerTime, float, "DT_BasePlayer", "m_flNextNoiseMakerTime");
    netvar(m_iSpawnRoomTouchCount, int, "DT_BasePlayer", "m_iSpawnRoomTouchCount");
    netvar(m_iKillCountSinceLastDeploy, int, "DT_BasePlayer", "m_iKillCountSinceLastDeploy");
    netvar(m_flFirstPrimaryAttack, float, "DT_BasePlayer", "m_flFirstPrimaryAttack");
    netvar(m_flEnergyDrinkMeter, float, "DT_BasePlayer", "m_flEnergyDrinkMeter");
    netvar(m_flHypeMeter, float, "DT_BasePlayer", "m_flHypeMeter");
    netvar(m_flChargeMeter, float, "DT_BasePlayer", "m_flChargeMeter");
    netvar(m_flInvisChangeCompleteTime, float, "DT_BasePlayer", "m_flInvisChangeCompleteTime");
    netvar(m_nDisguiseTeam, int, "DT_BasePlayer", "m_nDisguiseTeam");
    netvar(m_nDisguiseClass, int, "DT_BasePlayer", "m_nDisguiseClass");
    netvar(m_nDisguiseSkinOverride, int, "DT_BasePlayer", "m_nDisguiseSkinOverride");
    netvar(m_nMaskClass, int, "DT_BasePlayer", "m_nMaskClass");
    netvar(m_iDisguiseTargetIndex, int, "DT_BasePlayer", "m_iDisguiseTargetIndex");
    netvar(m_iDisguiseHealth, int, "DT_BasePlayer", "m_iDisguiseHealth");
    netvar(m_bFeignDeathReady, bool, "DT_BasePlayer", "m_bFeignDeathReady");
    netvar(m_hDisguiseWeapon, tf_entity, "DT_BasePlayer", "m_hDisguiseWeapon");
    netvar(m_nTeamTeleporterUsed, int, "DT_BasePlayer", "m_nTeamTeleporterUsed");
    netvar(m_flCloakMeter, float, "DT_BasePlayer", "m_flCloakMeter");
    netvar(m_flSpyTranqBuffDuration, float, "DT_BasePlayer", "m_flSpyTranqBuffDuration");
    netvar(m_nDesiredDisguiseTeam, int, "DT_BasePlayer", "m_nDesiredDisguiseTeam");
    netvar(m_nDesiredDisguiseClass, int, "DT_BasePlayer", "m_nDesiredDisguiseClass");
    netvar(m_flStealthNoAttackExpire, float, "DT_BasePlayer", "m_flStealthNoAttackExpire");
    netvar(m_flStealthNextChangeTime, float, "DT_BasePlayer", "m_flStealthNextChangeTime");
    netvar(m_bLastDisguisedAsOwnTeam, bool, "DT_BasePlayer", "m_bLastDisguisedAsOwnTeam");
    netvar(m_flRageMeter, float, "DT_BasePlayer", "m_flRageMeter");
    netvar(m_bRageDraining, bool, "DT_BasePlayer", "m_bRageDraining");
    netvar(m_flNextRageEarnTime, float, "DT_BasePlayer", "m_flNextRageEarnTime");
    netvar(m_bInUpgradeZone, bool, "DT_BasePlayer", "m_bInUpgradeZone");
    netvar(m_flItemChargeMeter, void*, "DT_BasePlayer", "m_flItemChargeMeter");
    netvar(m_bPlayerDominated, bool, "DT_BasePlayer", "m_bPlayerDominated");
    netvar(m_bPlayerDominatingMe, bool, "DT_BasePlayer", "m_bPlayerDominatingMe");
    netvar(m_ScoreData, void*, "DT_BasePlayer", "m_ScoreData");
    netvar(m_iCaptures, int, "DT_BasePlayer", "m_iCaptures");
    netvar(m_iDefenses, int, "DT_BasePlayer", "m_iDefenses");
    netvar(m_iKills, int, "DT_BasePlayer", "m_iKills");
    netvar(m_iDeaths, int, "DT_BasePlayer", "m_iDeaths");
    netvar(m_iSuicides, int, "DT_BasePlayer", "m_iSuicides");
    netvar(m_iDominations, int, "DT_BasePlayer", "m_iDominations");
    netvar(m_iRevenge, int, "DT_BasePlayer", "m_iRevenge");
    netvar(m_iBuildingsBuilt, int, "DT_BasePlayer", "m_iBuildingsBuilt");
    netvar(m_iBuildingsDestroyed, int, "DT_BasePlayer", "m_iBuildingsDestroyed");
    netvar(m_iHeadshots, int, "DT_BasePlayer", "m_iHeadshots");
    netvar(m_iBackstabs, int, "DT_BasePlayer", "m_iBackstabs");
    netvar(m_iHealPoints, int, "DT_BasePlayer", "m_iHealPoints");
    netvar(m_iInvulns, int, "DT_BasePlayer", "m_iInvulns");
    netvar(m_iTeleports, int, "DT_BasePlayer", "m_iTeleports");
    netvar(m_iResupplyPoints, int, "DT_BasePlayer", "m_iResupplyPoints");
    netvar(m_iKillAssists, int, "DT_BasePlayer", "m_iKillAssists");
    netvar(m_iPoints, int, "DT_BasePlayer", "m_iPoints");
    netvar(m_iBonusPoints, int, "DT_BasePlayer", "m_iBonusPoints");
    netvar(m_iDamageDone, int, "DT_BasePlayer", "m_iDamageDone");
    netvar(m_iCrits, int, "DT_BasePlayer", "m_iCrits");
    netvar(m_RoundScoreData, void*, "DT_BasePlayer", "m_RoundScoreData");
    netvar(m_ConditionList, void*, "DT_BasePlayer", "m_ConditionList");
    netvar(m_iTauntIndex, int, "DT_BasePlayer", "m_iTauntIndex");
    netvar(m_iTauntConcept, int, "DT_BasePlayer", "m_iTauntConcept");
    netvar(m_nPlayerCondEx, int, "DT_BasePlayer", "m_nPlayerCondEx");
    netvar(m_iStunIndex, int, "DT_BasePlayer", "m_iStunIndex");
    netvar(m_nHalloweenBombHeadStage, int, "DT_BasePlayer", "m_nHalloweenBombHeadStage");
    netvar(m_nPlayerCondEx3, int, "DT_BasePlayer", "m_nPlayerCondEx3");
    netvar(m_nPlayerCondEx2, int, "DT_BasePlayer", "m_nPlayerCondEx2");
    netvar(m_nStreaks, void*, "DT_BasePlayer", "m_nStreaks");
    netvar(m_unTauntSourceItemID_Low, int, "DT_BasePlayer", "m_unTauntSourceItemID_Low");
    netvar(m_unTauntSourceItemID_High, int, "DT_BasePlayer", "m_unTauntSourceItemID_High");
    netvar(m_flRuneCharge, float, "DT_BasePlayer", "m_flRuneCharge");
    netvar(m_bHasPasstimeBall, bool, "DT_BasePlayer", "m_bHasPasstimeBall");
    netvar(m_bIsTargetedForPasstimePass, bool, "DT_BasePlayer", "m_bIsTargetedForPasstimePass");
    netvar(m_hPasstimePassTarget, tf_entity, "DT_BasePlayer", "m_hPasstimePassTarget");
    netvar(m_askForBallTime, float, "DT_BasePlayer", "m_askForBallTime");
    netvar(m_bKingRuneBuffActive, bool, "DT_BasePlayer", "m_bKingRuneBuffActive");
    netvar(m_pProvider, int, "DT_BasePlayer", "m_pProvider");
    netvar(m_nPlayerCondEx4, int, "DT_BasePlayer", "m_nPlayerCondEx4");
    netvar(m_flHolsterAnimTime, float, "DT_BasePlayer", "m_flHolsterAnimTime");
    netvar(m_hSwitchTo, tf_entity, "DT_BasePlayer", "m_hSwitchTo");
    netvar(m_hItem, tf_entity, "DT_BasePlayer", "m_hItem");
    netvar(m_angEyeAngles, vector, "DT_BasePlayer", "m_angEyeAngles[0]");
    netvar(m_bIsCoaching, bool, "DT_BasePlayer", "m_bIsCoaching");
    netvar(m_hCoach, tf_entity, "DT_BasePlayer", "m_hCoach");
    netvar(m_hStudent, tf_entity, "DT_BasePlayer", "m_hStudent");
    netvar(m_nCurrency, int, "DT_BasePlayer", "m_nCurrency");
    netvar(m_nExperienceLevel, int, "DT_BasePlayer", "m_nExperienceLevel");
    netvar(m_nExperienceLevelProgress, int, "DT_BasePlayer", "m_nExperienceLevelProgress");
    netvar(m_bMatchSafeToLeave, bool, "DT_BasePlayer", "m_bMatchSafeToLeave");
    netvar(m_bAllowMoveDuringTaunt, bool, "DT_BasePlayer", "m_bAllowMoveDuringTaunt");
    netvar(m_bIsReadyToHighFive, bool, "DT_BasePlayer", "m_bIsReadyToHighFive");
    netvar(m_hHighFivePartner, int, "DT_BasePlayer", "m_hHighFivePartner");
    netvar(m_nForceTauntCam, int, "DT_BasePlayer", "m_nForceTauntCam");
    netvar(m_flTauntYaw, float, "DT_BasePlayer", "m_flTauntYaw");
    netvar(m_nActiveTauntSlot, int, "DT_BasePlayer", "m_nActiveTauntSlot");
    netvar(m_iTauntItemDefIndex, int, "DT_BasePlayer", "m_iTauntItemDefIndex");
    netvar(m_flCurrentTauntMoveSpeed, float, "DT_BasePlayer", "m_flCurrentTauntMoveSpeed");
    netvar(m_flVehicleReverseTime, float, "DT_BasePlayer", "m_flVehicleReverseTime");
    netvar(m_flMvMLastDamageTime, float, "DT_BasePlayer", "m_flMvMLastDamageTime");
    netvar(m_flLastDamageTime, float, "DT_BasePlayer", "m_flLastDamageTime");
    netvar(m_bInPowerPlay, bool, "DT_BasePlayer", "m_bInPowerPlay");
    netvar(m_iSpawnCounter, int, "DT_BasePlayer", "m_iSpawnCounter");
    netvar(m_bArenaSpectator, bool, "DT_BasePlayer", "m_bArenaSpectator");
    netvar(m_AttributeManager, void*, "DT_BasePlayer", "m_AttributeManager");
    netvar(m_hOuter, tf_entity, "DT_BasePlayer", "m_hOuter");
    netvar(m_ProviderType, int, "DT_BasePlayer", "m_ProviderType");
    netvar(m_iReapplyProvisionParity, int, "DT_BasePlayer", "m_iReapplyProvisionParity");
    netvar(m_flHeadScale, float, "DT_BasePlayer", "m_flHeadScale");
    netvar(m_flTorsoScale, float, "DT_BasePlayer", "m_flTorsoScale");
    netvar(m_flHandScale, float, "DT_BasePlayer", "m_flHandScale");
    netvar(m_bUseBossHealthBar, bool, "DT_BasePlayer", "m_bUseBossHealthBar");
    netvar(m_bUsingVRHeadset, bool, "DT_BasePlayer", "m_bUsingVRHeadset");
    netvar(m_bForcedSkin, bool, "DT_BasePlayer", "m_bForcedSkin");
    netvar(m_nForcedSkin, int, "DT_BasePlayer", "m_nForcedSkin");
    netvar(m_nActiveWpnClip, int, "DT_BasePlayer", "m_nActiveWpnClip");
    netvar(m_flKartNextAvailableBoost, float, "DT_BasePlayer", "m_flKartNextAvailableBoost");
    netvar(m_iKartHealth, int, "DT_BasePlayer", "m_iKartHealth");
    netvar(m_iKartState, int, "DT_BasePlayer", "m_iKartState");
    netvar(m_hGrapplingHookTarget, tf_entity, "DT_BasePlayer", "m_hGrapplingHookTarget");
    netvar(m_hSecondaryLastWeapon, tf_entity, "DT_BasePlayer", "m_hSecondaryLastWeapon");
    netvar(m_bUsingActionSlot, bool, "DT_BasePlayer", "m_bUsingActionSlot");
    netvar(m_flInspectTime, float, "DT_BasePlayer", "m_flInspectTime");
    netvar(m_flHelpmeButtonPressTime, float, "DT_BasePlayer", "m_flHelpmeButtonPressTime");
    netvar(m_iCampaignMedals, int, "DT_BasePlayer", "m_iCampaignMedals");
    netvar(m_iPlayerSkinOverride, int, "DT_BasePlayer", "m_iPlayerSkinOverride");
    netvar(m_bViewingCYOAPDA, bool, "DT_BasePlayer", "m_bViewingCYOAPDA");


    bool in_cond(const ETFCond cond) {
        const int _cond = static_cast<int>(cond);

        switch (_cond / 32) {
        case 0: {
            const int bit = (1 << _cond);

            if ((player_cond() & bit) == bit || (_condition_bits() & bit) == bit)
                return true;

            break;
        }

        case 1: {
            const int bit = 1 << (_cond - 32);

            if ((player_cond_ex() & bit) == bit)
                return true;

            break;
        }

        case 2: {
            const int bit = 1 << (_cond - 64);

            if ((player_cond_ex2() & bit) == bit)
                return true;

            break;
        }

        case 3: {
            const int bit = 1 << (_cond - 96);

            if ((player_cond_ex3() & bit) == bit)
                return true;

            break;
        }

        case 4: {
            const int bit = 1 << (_cond - 128);

            if ((player_cond_ex4() & bit) == bit)
                return true;

            break;
        }

        default:
            break;
        }

        return false;
    }
    bool is_burning() { return in_cond(ETFCond::TF_COND_BURNING) || in_cond(ETFCond::TF_COND_BURNING_PYRO); }
    bool has_high_kd() {
        if ((float)kills() / (float)deaths() > 15.f)
            return true;

        return false;
    }
    bool is_critboosted() {
        int cond = player_cond();
        int cond_ex = player_cond_ex();

        return cond & TFCond_Kritzkrieged || _condition_bits() & TFCond_Kritzkrieged ||
            cond_ex & TFCondEx_CritCanteen || cond_ex & TFCondEx_CritOnFirstBlood ||
            cond_ex & TFCondEx_CritOnWin || cond_ex & TFCondEx_CritOnKill ||
            cond_ex & TFCondEx_CritDemoCharge || cond_ex & TFCondEx_CritOnFlagCapture ||
            cond_ex & TFCondEx_HalloweenCritCandy || cond_ex & TFCondEx_PyroCrits;
    }
    bool is_mini_critboosted() { return player_cond() & TFCond_MiniCrits; }
    bool is_bonked() { return player_cond() & TFCond_Bonked; }
    bool is_charging() { return player_cond() & TFCond_Charging; }
    bool is_taunting() { return player_cond() & TFCond_Taunting; }
    bool is_marked() {
        return player_cond() & TFCond_Jarated || player_cond() & TFCond_MarkedForDeath ||
            player_cond_ex() & TFCondEx_MarkedForDeathSilent;
    }
    bool is_stunned() { return player_cond() & TFCond_Stunned; }
    bool is_in_kart() { return player_cond_ex() & TFCondEx_InKart; }
    bool is_ghost() { return player_cond_ex2() & TFCondEx2_HalloweenGhostMode; }
    bool is_cloaked() {
        return player_cond() & TFCond_Cloaked || player_cond() & TFCond_CloakFlicker ||
            player_cond_ex2() & TFCondEx2_Stealthed || player_cond_ex2() & TFCondEx2_StealthedUserBuffFade;
    }
    bool is_invuln() {
        return player_cond() & TFCond_Ubercharged || player_cond_ex() & TFCondEx_UberchargedCanteen;
    }
    bool is_zoomed() { return player_cond() & TFCond_Zoomed; }
    bool is_quickfix_ubered() { return player_cond() & TFCond_MegaHeal; }
    bool is_in_conch() { return player_cond() & TFCond_RegenBuffed; }
    bool is_in_backup() { return player_cond() & TFCond_DefenseBuffed; }
    bool is_in_banner() { return player_cond() & TFCond_Buffed; }
    bool is_bleeding() { return player_cond() & TFCond_Bleeding; }
    bool is_milked() { return player_cond() & TFCond_Milked; }
    bool is_slowed() { return player_cond() & TFCond_Slowed; }
    bool bullet_res() { return player_cond_ex() & TFCondEx_BulletResistance; }
    bool bullet_uber() { return player_cond_ex() & TFCondEx_BulletCharge; }
    bool blast_res() { return player_cond_ex() & TFCondEx_ExplosiveResistance; }
    bool blast_uber() { return player_cond_ex() & TFCondEx_ExplosiveCharge; }
    bool fire_res() { return player_cond_ex() & TFCondEx_FireResistance; }
    bool fire_uber() { return player_cond_ex() & TFCondEx_FireCharge; }


    tf_weapon* get_active_weapon();

    vector get_shoot_position() { return origin() + view_offset(); }
};


class tf_weapon : public tf_entity {
public:
    int get_weapon_id() { return memory::find_vfunc<int(__thiscall*)(void*)>(this, 381u)(this); }
};



inline tf_player* local = nullptr;
inline tf_weapon* local_weapon = nullptr;
inline std::vector<tf_player*> players = { };

class tf_sentry : public tf_entity {
public:
    netvar(ammo_shells, int, "DT_ObjectSentrygun", "m_iAmmoShells");
    netvar(ammo_rockets, int, "DT_ObjectSentrygun", "m_iAmmoRockets");
    netvar(state, int, "DT_ObjectSentrygun", "m_iState");
    netvar(is_controlled_by_player, c_base_handle, "DT_ObjectSentrygun", "m_bPlayerControlled");
    netvar(shield_level, int, "DT_ObjectSentrygun", "m_nShieldLevel");
    netvar(is_shielded, bool, "DT_ObjectSentrygun", "m_bShielded");
    netvar(enemy, c_base_handle, "DT_ObjectSentrygun", "m_hEnemy");
    netvar(autoaim_target, c_base_handle, "DT_ObjectSentrygun", "m_hAutoAimTarget");
    netvar(kills, int, "DT_ObjectSentrygun", "m_iKills");
    netvar(assists, int, "DT_ObjectSentrygun", "m_iAssists");
    netvar(health, int, "DT_ObjectSentrygun", "m_iHealth");

    int max_ammo_shells() {
        if ((upgrade_level() == 1) || is_mini_building())
            return SENTRYGUN_MAX_SHELLS_1;
        else
            return SENTRYGUN_MAX_SHELLS_3; // both 2 and 3 are 200
    }

    void get_ammo_count(int& shells, int& max_shells, int& rockets, int& max_rockets) {
        const bool is_mini = is_mini_building();

        shells = ammo_shells();
        max_shells = max_ammo_shells();
        rockets = is_mini ? 0 : ammo_rockets();
        max_rockets = (is_mini || (upgrade_level() < 3)) ? 0 : SENTRYGUN_MAX_ROCKETS;
    }
};

inline std::vector<tf_sentry*> sentry = { };

class tf_teleporter : public tf_entity {
public:
    netvar(health, int, "DT_ObjectTeleporter", "m_iHealth");
    netvar(state, int, "DT_ObjectTeleporter", "m_iState");
    netvar(recharge, float, "DT_ObjectTeleporter", "m_flRechargeTime");
    netvar(cur_charge, float, "DT_ObjectTeleporter", "m_flCurrentRechargeDuration");
    netvar(used, int, "DT_ObjectTeleporter", "m_iTimesUsed");
    netvar(exit_yaw, float, "DT_ObjectTeleporter", "m_flYawToExit");
    netvar(match, bool, "DT_ObjectTeleporter", "m_bMatchBuilding");
};

inline std::vector<tf_teleporter*> teleporters = { };

#include "vector.hpp"
#include "Windows.h"

class c_user_cmd {
public:
    virtual ~c_user_cmd() {};

    // For matching server and client commands for debugging
    int    command_number;

    // the tick the client created this command
    int    tick_count;

    // Player instantaneous view angles.
    QAngle viewangles;
    // Intended velocities
    //	forward velocity.
    float  forwardmove;
    //  sideways velocity.
    float  sidemove;
    //  upward velocity.
    float  upmove;
    // Attack button states
    int    buttons;
    // Impulse command issued.
    byte   impulse;
    // Current weapon id
    int    weaponselect;
    int    weaponsubtype;

    int    random_seed; // For shared random functions

    short  mousedx; // mouse accum in x from create move
    short  mousedy; // mouse accum in y from create move

    // Client only, tracks whether we've predicted this command at least once
    bool   hasbeenpredicted;
};