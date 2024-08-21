#include "hooks.h"

namespace hooks
{

	static inline RE::BSFixedString cPtr_bashPowerStart = "bashPowerStart";
	/// <summary>
	/// Handle perilous attacking.
	/// </summary>
	/// <param name="a_actionData"></param>
	/// <returns>Whether the attack action is performed.</returns>
	// "TKDodgeStop"_h:
	// "MCO_DodgeInitiate"_h:
	// case "MCO_Recovery"_h:
	// 	bool bIsDodging = false;
	// 	if (Utils::Actor::isHumanoid(actor) && actor->GetGraphVariableBool("bIsDodging", bIsDodging) && !bIsDodging) {
	// 		if (actor->AsActorState()->actorState2.wantBlocking) {
	// 			actor->NotifyAnimationGraph("MCO_EndAnimation");
	// 			actor->NotifyAnimationGraph("MCO_AnimStop");
	// 			actor->NotifyAnimationGraph("attackStop");
	// 			actor->NotifyAnimationGraph("blockStart");
	// 		}
	// 	}
	// 	break;
	// if (!actor->IsPlayerRef()) {
	// 	actor->NotifyAnimationGraph("recoilStop");
	// }
	// break;

	void on_animation_event::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event->holder) {
			return;
		}
		std::string_view eventTag = a_event->tag.data();
		RE::Actor* actor = const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>();
		switch (hash(eventTag.data(), eventTag.size())) {
		case "TKDR_DodgeStart"_h:
		    if (!actor->IsPlayerRef()) {
				const auto StaminaCost = RE::TESForm::LookupByEditorID<RE::MagicItem>("StaminaCostSpell_UND");
				const auto caster = actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
				auto DS = dodge::GetSingleton();
				const float DodgeRoll_staminacost = DS->get_stamina_basecost(actor, DS->Staminaa, true);
				const float SideStep_staminacost = DS->get_stamina_basecost(actor, DS->Staminaa);
				bool bUND_IsDodgeRoll = false;
				if (actor->GetGraphVariableBool("bUND_IsDodgeRoll", bUND_IsDodgeRoll) && bUND_IsDodgeRoll) {
					if (settings::bStaminaCost_enable) {
						logger::info("Protagnist {} DodgeRollCost {}"sv, actor->GetName(), DodgeRoll_staminacost);
						caster->CastSpellImmediate(StaminaCost, true, actor, 1, false, -(DodgeRoll_staminacost), actor);
					}
					if (settings::biFrames_enable) {
						dodge::Set_iFrames(actor);
					}
					break;
				} else {
					if (settings::bStaminaCost_enable) {
						logger::info("Protagnist {} SideStepCost {}"sv, actor->GetName(), SideStep_staminacost);
						caster->CastSpellImmediate(StaminaCost, true, actor, 1, false, -(SideStep_staminacost), actor);
					}
					if (settings::biFrames_enable) {
						dodge::Set_iFrames(actor);
					}
				}
			}
			break;

		case "TKDR_DodgeEnd"_h:
			if (!actor->IsPlayerRef()) {
				if (settings::biFrames_enable) {
					dodge::Reset_iFrames(actor);
				}
			}
			break;
	
		case "preHitFrame"_h:
			if ((!Utils::Actor::isHumanoid(actor) || Utils::Actor::isPowerAttacking(actor)) 
			&& (!(actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash || actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kHit))) {
				
				dodge::GetSingleton()->react_to_melee(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));

			} else if ((!Utils::Actor::isPowerAttacking(actor) && actor->AsActorState()->GetAttackState() != RE::ATTACK_STATE_ENUM::kBash) 
			&& (actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kDraw || actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kSwing)) {
				
				dodge::GetSingleton()->react_to_melee(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));

			} else if (actor->AsActorState()->IsSprinting() && actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) {
				bool bMaxsuWeaponParry_InWeaponParry = false;
				if ((actor)
						->GetGraphVariableBool("bMaxsuWeaponParry_InWeaponParry", bMaxsuWeaponParry_InWeaponParry) &&
					!bMaxsuWeaponParry_InWeaponParry) {
					dodge::GetSingleton()->react_to_bash_sprint(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));
				}
			} else if (!actor->AsActorState()->IsSprinting() && actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash && Utils::Actor::isPowerAttacking(actor)) {
				bool bMaxsuWeaponParry_InWeaponParry = false;
				if ((actor)
						->GetGraphVariableBool("bMaxsuWeaponParry_InWeaponParry", bMaxsuWeaponParry_InWeaponParry) &&
					!bMaxsuWeaponParry_InWeaponParry) {
					dodge::GetSingleton()->react_to_bash(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));
				}
			}
			break;

		case "Voice_SpellFire_Event"_h:
			// if (dodge::GetSingleton()->GetEquippedShout(actor)) {
			// 	dodge::GetSingleton()->react_to_shouts_spells(actor, 3000.0f);
			// }
			if (GetEquippedShouts(actor)){
				dodge::GetSingleton()->react_to_shouts_spells(actor, 3000.0f);
			}
			break;

		// case "BeginCastVoice"_h:
		// 	if (actor->GetCurrentShout()->variations->spell->As<RE::MagicItem>()->IsHostile()) {
				
		// 		dodge::GetSingleton()->react_to_shouts_spells(actor, 3000.0f);
		// 	}
		// 	break;

		case "MLh_SpellFire_Event"_h:
			// if (actor->GetEquippedObject(true)->As<RE::MagicItem>()->IsHostile()) {
			// 	dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			// }
			if (dodge::GetSingleton()->GetAttackSpell(actor, true)) {
				dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			}
			// dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			break;

		// case "BeginCastLeft"_h:
		// 	if (actor->GetEquippedObject(true)->As<RE::MagicItem>()->IsHostile()) {
				
		// 		dodge::GetSingleton()->react_to_shouts_spells(actor, 2000.0f);
		// 	}
		// 	break;

		case "MRh_SpellFire_Event"_h:
			// if (actor->GetEquippedObject(false)->As<RE::MagicItem>()->IsHostile()) {
			// 	dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			// }
			// break;
			if (dodge::GetSingleton()->GetAttackSpell(actor)) {
				dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			}
			// dodge::GetSingleton()->react_to_shouts_spells_fast(actor, 3000.0f);
			break;

			// case "BeginCastRight"_h:
			// 	if (actor->GetEquippedObject(false)->As<RE::MagicItem>()->IsHostile()) {

			// 		dodge::GetSingleton()->react_to_shouts_spells(actor, 2000.0f);
			// 	}
			// 	break;

		case "PowerAttack_Start_end"_h:
		case "NextAttackInitiate"_h:
			dodge::GetSingleton()->react_to_melee_normal(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));
			break;

		// case "NextPowerAttackInitiate"_h:

		// 	dodge::GetSingleton()->react_to_melee_power(actor, dodge::GetSingleton()->Get_ReactiveDodge_Distance(actor));
		// 	break;

		// case "bashPowerStart"_h:
		// 	dodge::GetSingleton()->react_to_bash(actor, 250.0f);
		// 	break;

		// case "BlockBashSprint"_h:
			
		// 	dodge::GetSingleton()->react_to_bash_sprint(actor, 400.0f); 
		// 	break;

		case "BowFullDrawn"_h:
			
			dodge::GetSingleton()->react_to_ranged(actor, 1500.0f);
			break;
		}
	}

	bool GetEquippedShouts(RE::Actor* actor)
	{
		auto limboshout = actor->GetActorRuntimeData().selectedPower;

		if (limboshout && limboshout->Is(RE::FormType::Shout)) {
			std::string_view Lsht = (clib_util::editorID::get_editorID(limboshout)).data();
			switch (hash(Lsht.data(), Lsht.size())) {
			case "HoY_PullofNirnShout_Miraak"_h:
			case "SlowTimeShout"_h:
			case "Serio_EDR_GravityBlastShoutPAAR"_h:
			case "Serio_EDR_GravityBlastShoutODAH"_h:
			case "KS_SlowTime_Alduin"_h:
				return false;
			default:
			    return true;
			}
		}
		return false;
	}

	class OurEventSink : public RE::BSTEventSink<RE::TESCombatEvent>
	{
		OurEventSink() = default;
		OurEventSink(const OurEventSink&) = delete;
		OurEventSink(OurEventSink&&) = delete;
		OurEventSink& operator=(const OurEventSink&) = delete;
		OurEventSink& operator=(OurEventSink&&) = delete;

	public:
		static OurEventSink* GetSingleton()
		{
			static OurEventSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* event, RE::BSTEventSource<RE::TESCombatEvent>*)
		{
			decltype(auto) sourceName = event->actor;
			auto Protagonist = sourceName->As<RE::Actor>();

			if (Protagonist->IsPlayerRef() || !Utils::Actor::isHumanoid(Protagonist)) {
				return RE::BSEventNotifyControl::kContinue;
			}

			if (!(Protagonist->HasKeywordString("ActorTypeNPC") || Protagonist->HasKeywordString("DLC2ActorTypeMiraak"))) {
				return RE::BSEventNotifyControl::kContinue;
			}

			auto getcombatstate = event->newState.get();

			if (getcombatstate == RE::ACTOR_COMBAT_STATE::kCombat) {
				Protagonist->SetGraphVariableBool("bUND_InCombatFoundEnemy", true);
			} else {
				Protagonist->SetGraphVariableBool("bUND_InCombatFoundEnemy", false);
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void IHooks::install()
	{
		auto eventSink = OurEventSink::GetSingleton();

		// ScriptSource
		auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventSourceHolder->AddEventSink<RE::TESCombatEvent>(eventSink);
	}

	EventResult on_animation_event::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		ProcessEvent(a_sink, a_event, a_eventSource);
		return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
	}

	EventResult on_animation_event::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		ProcessEvent(a_sink, a_event, a_eventSource);
		return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
	}

	ptr_CombatPath on_combatBehavior_backoff_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		// RE::Character* actor = a_actor->As<RE::Character>();
	
		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_back);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_back);
			break;
		}
		
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_circle_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		// RE::Character* actor = a_actor->As<RE::Character>();

		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_horizontal);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_horizontal);
			break;
		}
	
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_fallback_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		// RE::Character* actor = a_actor->As<RE::Character>();

		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_back);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_back);
			break;
		}
		
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}

	ptr_CombatPath on_combatBehavior_dodgethreat_createPath::create_path(RE::Actor* a_actor, RE::NiPoint3* a_newPos, float a3, int speed_ind)
	{
		// RE::Character* actor = a_actor->As<RE::Character>();

		switch (settings::iDodgeAI_Framework) {
		case 0:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_tk_all);
			break;
		case 1:
			dodge::GetSingleton()->attempt_dodge(a_actor, &dodge_directions_dmco_all);
			break;
		}
		
		return _create_path(a_actor, a_newPos, a3, speed_ind);
	}
	


	inline void offset_NPC_rotation(RE::Actor* a_actor, float& a_angle) 
	{
		if (a_actor->IsPlayerRef()) {
			return;
		}
		float angleDelta = Utils::math::NormalRelativeAngle(a_angle - a_actor->data.angle.z);
	
		/* Dodge commitment */
		if (settings::bDodgeAI_Enable && dodge::GetSingleton()->get_is_dodging(a_actor)) {  
			angleDelta = 0;
		}
		a_angle = a_actor->data.angle.z + angleDelta;
	}

	
	void on_set_rotation::Actor_SetRotationX(RE::Actor* a_this, float a_angle)
	{
		offset_NPC_rotation(a_this, a_angle);
		_Actor_SetRotationX(a_this, a_angle);
	}

	void on_set_rotation::Actor_SetRotationZ(RE::Actor* a_this, float a_angle)
	{
		offset_NPC_rotation(a_this, a_angle);
		_Actor_SetRotationZ(a_this, a_angle);
	}

};
