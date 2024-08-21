#include "UselessFenixUtils.h"

namespace Utils
{
	RE::BGSAttackData* get_attackData(RE::Actor* a) {
		if (!a->GetActorRuntimeData().currentProcess || !a->GetActorRuntimeData().currentProcess->high)
			return nullptr;
		return a->GetActorRuntimeData().currentProcess->high->attackData.get();
	}
}

RE::Character* CombatAI__get_he()
{
	return _generic_foo<46265, RE::Character*>();  // SkyrimSE.exe+7c7b20
}

RE::Character* CombatAI__get_me()
{
	return _generic_foo<46264, RE::Character*>();  // SkyrimSE.exe+7C7A40
}


float Actor__GetActorValueModifier(RE::Actor* a, RE::ACTOR_VALUE_MODIFIER mod, RE::ActorValue av)
{
	return _generic_foo<37524, float, RE::Actor*, RE::ACTOR_VALUE_MODIFIER, RE::ActorValue>(a, mod, av);  // SkyrimSE.exe+621350
}

float get_total_av(RE::Actor* a, RE::ActorValue av)
{
	float permanent = a->AsActorValueOwner()->GetPermanentActorValue(av);
	float temporary = Actor__GetActorValueModifier(a, RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
	return permanent + temporary;
}
