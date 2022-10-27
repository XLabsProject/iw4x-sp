#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace player_movement {
namespace {
void __declspec(naked) pm_step_slide_move_stub() {
  __asm {
    push eax;
    mov eax, dvars::pm_bounce;
    cmp byte ptr [eax + 0x10], 1;
    pop eax;

    je bounce;

    cmp dword ptr [esp + 0x24], 0;
    jnz no_bounce;

   bounce:
    push 0x4E905B;
    ret;

   no_bounce:
    push 0x4E906F;
    ret;
  }
}

void pm_project_velocity_stub(const float* vel_in, const float* normal,
                              float* vel_out) {
  const auto length_squared_2d = vel_in[0] * vel_in[0] + vel_in[1] * vel_in[1];

  if (std::fabsf(normal[2]) < 0.001f || length_squared_2d == 0.0f) {
    vel_out[0] = vel_in[0];
    vel_out[1] = vel_in[1];
    vel_out[2] = vel_in[2];
    return;
  }

  auto new_z = vel_in[0] * normal[0] + vel_in[1] * normal[1];
  new_z = -new_z / normal[2];

  const auto length_scale =
      std::sqrtf((vel_in[2] * vel_in[2] + length_squared_2d) /
                 (new_z * new_z + length_squared_2d));

  if (dvars::pm_bouncingAllAngles->current.enabled ||
      (length_scale < 1.f || new_z < 0.f || vel_in[2] > 0.f)) {
    vel_out[0] = vel_in[0] * length_scale;
    vel_out[1] = vel_in[1] * length_scale;
    vel_out[2] = new_z * length_scale;
  }
}

game::gentity_s* weapon_rocket_launcher_fire_stub(
    game::gentity_s* ent, unsigned int weapon_index, float spread,
    game::weaponParms* wp, const float* gun_vel,
    game::lockonFireParms* lock_parms, bool magic_bullet) {
  auto* result = utils::hook::invoke<game::gentity_s*>(
      0x443F20, ent, weapon_index, spread, wp, gun_vel, lock_parms,
      magic_bullet);

  if (ent->client != nullptr && dvars::pm_rocketJump->current.enabled) {
    const auto scale = dvars::pm_rocketJumpScale->current.value;
    ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * scale;
    ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * scale;
    ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * scale;
  }

  return result;
}

void cm_transformed_capsule_trace_stub(game::trace_t* results,
                                       const float* start, const float* end,
                                       const game::Bounds* bounds,
                                       const game::Bounds* capsule,
                                       int contents, const float* origin,
                                       const float* angles) {
  if (dvars::pm_playerCollision->current.enabled) {
    utils::hook::invoke<void>(0x47C7D0, results, start, end, bounds, capsule,
                              contents, origin, angles);
  }
}

void pm_trace_stub(game::pmove_t* pm, game::trace_t* results,
                   const float* start, const float* end,
                   const game::Bounds* bounds, int pass_entity_num,
                   int content_mask) {
  game::PM_trace(pm, results, start, end, bounds, pass_entity_num,
                 content_mask);
  // Allow the player to stand even when there is no headroom
  if (dvars::pm_elevators->current.enabled) {
    results->allsolid = false;
  }
}

void pm_player_trace_stub(game::pmove_t* pm, game::trace_t* results,
                          const float* start, const float* end,
                          const game::Bounds* bounds, int pass_entity_num,
                          int content_mask) {
  game::PM_playerTrace(pm, results, start, end, bounds, pass_entity_num,
                       content_mask);

  if (dvars::pm_elevators->current.enabled) {
    results->startsolid = false;
  }
}
} // namespace

class component final : public component_interface {
public:
  static_assert(sizeof(game::weaponParms) == 0x48);
  static_assert(sizeof(game::lockonFireParms) == 0x18);

  void post_load() override {
    utils::hook(0x4E9054, pm_step_slide_move_stub, HOOK_JUMP)
        .install()
        ->quick(); // PM_StepSlideMove
    utils::hook(0x4E90BE, pm_project_velocity_stub, HOOK_CALL)
        .install()
        ->quick(); // PM_StepSlideMove

    utils::hook(0x4FA809, weapon_rocket_launcher_fire_stub, HOOK_CALL)
        .install()
        ->quick(); // FireWeapon

    utils::hook(0x4B6FC0, cm_transformed_capsule_trace_stub, HOOK_CALL)
        .install()
        ->quick(); // SV_ClipMoveToEntity
    utils::hook(0x57635F, cm_transformed_capsule_trace_stub, HOOK_CALL)
        .install()
        ->quick(); // CG_ClipMoveToEntity

    utils::hook(0x64F439, pm_trace_stub, HOOK_CALL)
        .install()
        ->quick(); // PM_CheckDuck
    utils::hook(0x651A60, pm_player_trace_stub, HOOK_CALL)
        .install()
        ->quick(); // PM_CorrectAllSolid
    utils::hook(0x651AF2, pm_player_trace_stub, HOOK_CALL)
        .install()
        ->quick(); // PM_CorrectAllSolid

    register_dvars();
  }

  static void register_dvars() {
    // clang-format off
    dvars::pm_bounce = game::Dvar_RegisterBool(
        "pm_bounce", false, game::DVAR_NONE, "CoD4 Bounces");
    dvars::pm_bouncingAllAngles = game::Dvar_RegisterBool(
        "pm_bouncingAllAngles", false, game::DVAR_NONE, "Enable bouncing from all angles");
    dvars::pm_rocketJump = game::Dvar_RegisterBool(
        "pm_rocketJump", true, game::DVAR_NONE, "CoD4 rocket jumps");
    dvars::pm_rocketJumpScale = game::Dvar_RegisterFloat(
        "pm_rocketJumpScale", 64.0f, 0.0f, 1024.0f, game::DVAR_NONE, "");
    dvars::pm_playerCollision = game::Dvar_RegisterBool(
         "pm_playerCollision", true, game::DVAR_NONE, "Push intersecting players away from each other");
    dvars::pm_elevators = game::Dvar_RegisterBool(
         "pm_elevators", false, game::DVAR_NONE, "CoD4 elevators");
    // clang-format on
  }
};
} // namespace player_movement

REGISTER_COMPONENT(player_movement::component)
