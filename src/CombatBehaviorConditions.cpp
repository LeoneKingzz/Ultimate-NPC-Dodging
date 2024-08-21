#include "CombatBehaviorConditions.h"




#define DODGE_ID 1
#define DODGE_SPEED 1.647f

using PA = Utils::PolarAngle;

bool has_manyStamina(RE::Actor* a)
{
	auto cur = a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
	auto total = get_total_av(a, RE::ActorValue::kStamina);
	return total >= 100.0f && total * 0.5 <= cur;
}

bool has_enoughStamina(RE::Actor* a)
{
	auto cur = a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
	auto total = get_total_av(a, RE::ActorValue::kStamina);
	return total >= 25.0f && total * 0.25 <= cur;
}

bool is_powerattacking(RE::Actor* a)
{
	return _generic_foo<37639, bool, RE::Actor*>(a);
}

bool is_staying(RE::Actor* a) {
	return !a->AsActorState()->actorState1.running && !a->AsActorState()->actorState1.walking && !a->AsActorState()->actorState1.sprinting;
}

bool has_stamina(RE::Actor* a)
{
	auto cur = a->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
	auto total = get_total_av(a, RE::ActorValue::kStamina);
	return total <= 20.0f || cur >= 15.0f;
}

bool is_staggered(RE::Actor* a)
{
	return _generic_foo<46064, bool, RE::Actor*>(a);
}

bool check_collisions(RE::Actor* a, RE::NiPoint3* a_pos, RE::NiPoint3* a_new_pos)
{
	return _generic_foo<46050, bool, RE::Actor*, RE::NiPoint3*, RE::NiPoint3*, float, float>(a, a_pos, a_new_pos, 2.0f, 64.0f);
}

bool is_blocking(RE::Actor* a)
{
	return _generic_foo<36927, bool, RE::Actor*>(a);
}

bool is_bashing(RE::Actor* a)
{
	if (a->AsActorState()->actorState1.meleeAttackState == RE::ATTACK_STATE_ENUM::kNone)
		return false;

	auto adata = Utils::get_attackData(a);
	if (!adata)
		return false;

	return adata->data.flags.any(RE::AttackData::AttackFlag::kBashAttack);
}

bool is_attacking(RE::Actor* a)
{
	auto adata = Utils::get_attackData(a);
	if (!adata)
		return false;
	auto state = a->AsActorState()->actorState1.meleeAttackState;
	return state == RE::ATTACK_STATE_ENUM::kDraw || state == RE::ATTACK_STATE_ENUM::kSwing || state == RE::ATTACK_STATE_ENUM::kHit;
}

bool is_juststarted_attacking(RE::Actor* a)
{
	auto adata = Utils::get_attackData(a);
	if (!adata)
		return false;
	auto state = a->AsActorState()->actorState1.meleeAttackState;
	return state == RE::ATTACK_STATE_ENUM::kDraw;
}

bool is_moving(RE::Actor* a)
{
	auto state = a->AsActorState()->actorState1;
	return state.running || state.walking || state.sprinting;
}

float get_angle_he_me(RE::Actor* me, RE::Actor* he, RE::BGSAttackData* attackdata)
{
	auto he_me = PA(me->GetPosition() - he->GetPosition());
	auto head = PA(he->GetHeading(false) * 180.0f / PI);
	if (attackdata)
		head = head.add(attackdata->data.attackAngle);
	auto angle = he_me.sub(head).to_normangle();
	return angle;
}

void interruptattack(RE::Actor* me)
{
	me->NotifyAnimationGraph("attackStop");
	me->NotifyAnimationGraph("bashStop");
	me->NotifyAnimationGraph("blockStop");
	me->NotifyAnimationGraph("staggerStop");
	me->NotifyAnimationGraph("recoilStop");
}

bool is_AttackEnded(RE::Actor* a)
{
	auto state = a->AsActorState()->actorState1.meleeAttackState;
	return state != RE::ATTACK_STATE_ENUM::kDraw && state != RE::ATTACK_STATE_ENUM::kSwing && state != RE::ATTACK_STATE_ENUM::kNone;
}



float get_Reach(RE::Actor* a)
{
	return _generic_foo<37588, float, RE::Actor*>(a);
}

float get_dist2(RE::Actor* a, RE::Actor* b)
{
	return _generic_foo<46058, float, RE::Actor*, RE::Actor*>(a, b);
}

float get_combat_reach(RE::Actor* a)
{
	return get_Reach(a) + 30.0f;
}


