#include "log.h"
#include "hooks.h"
#include "settings.h"

void OnDataLoaded()
{
   
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		hooks::on_animation_event::install();
		hooks::IHooks::install();
		// hooks::on_set_rotation::install();
		if (settings::bTacticalDodgeAI_enable)
		{ // install hooks for passive dodge
			hooks::on_combatBehavior_backoff_createPath::install();
			hooks::on_combatBehavior_circle_createPath::install();
			hooks::on_combatBehavior_dodgethreat_createPath::install();
			hooks::on_combatBehavior_fallback_createPath::install();
		}
		settings::init();
		break;

	case SKSE::MessagingInterface::kPostPostLoad:
		dodge::GetSingleton()->init();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		settings::setglobals();
		break;
	default:
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
	SetupLog();


    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	
    return true;
}