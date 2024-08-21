#pragma once
#include "UselessFenixUtils.h"
#include "dodge.h"


bool has_enoughStamina(RE::Actor* a);
void interruptattack(RE::Actor* me);
float get_combat_reach(RE::Actor* a);
float get_dist2(RE::Actor* a, RE::Actor* b);
float get_angle_he_me(RE::Actor* me, RE::Actor* he, RE::BGSAttackData* attackdata);
bool is_powerattacking(RE::Actor* a);
