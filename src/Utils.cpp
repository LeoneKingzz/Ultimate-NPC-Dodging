#include "settings.h"
using writeLock = std::unique_lock<std::shared_mutex>;
using readLock = std::shared_lock<std::shared_mutex>;

void Utils::queueMessageBox(RE::BSFixedString a_message)
{
	auto factoryManager = RE::MessageDataFactoryManager::GetSingleton();
	auto uiStrHolder = RE::InterfaceStrings::GetSingleton();
	auto factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStrHolder->messageBoxData);
	auto messageBox = factory->Create();
	messageBox->unk4C = 4;
	messageBox->unk38 = 10;
	messageBox->bodyText = a_message;
	auto gameSettings = RE::GameSettingCollection::GetSingleton();
	auto sOk = gameSettings->GetSetting("sOk");
	messageBox->buttonText.push_back(sOk->GetString());
	messageBox->QueueMessage();
}

void Utils::playSound(RE::Actor* a, RE::BGSSoundDescriptorForm* a_descriptor, float a_volumeOverride)
{
	RE::BSSoundHandle handle;
	handle.soundID = static_cast<uint32_t>(-1);
	handle.assumeSuccess = false;
	*(uint32_t*)&handle.state = 0;

	RE::Offset::soundHelper_a(RE::BSAudioManager::GetSingleton(), &handle, a_descriptor->GetFormID(), 16);
	if (RE::Offset::set_sound_position(&handle, a->data.location.x, a->data.location.y, a->data.location.z)) {
		handle.SetVolume(a_volumeOverride);
		RE::Offset::soundHelper_b(&handle, a->Get3D());
		RE::Offset::soundHelper_c(&handle);
	}
}

float Utils::get_angle_he_me(RE::Actor *me, RE::Actor *he, RE::BGSAttackData *attackdata)
{
	auto he_me = PolarAngle(me->GetPosition() - he->GetPosition());
	auto head = PolarAngle(he->GetHeading(false) * 180.0f / PI);
	if (attackdata)
		head = head.add(attackdata->data.attackAngle);
	auto angle = he_me.sub(head).to_normangle();
	return angle;
}

RE::BGSAttackData *Utils::get_attackData(RE::Actor *a)
{
	if (!a->GetActorRuntimeData().currentProcess || !a->GetActorRuntimeData().currentProcess->high)
		return nullptr;
	return a->GetActorRuntimeData().currentProcess->high->attackData.get();
}

/*Get the absolute position of a point in world, given a relative position to an actor.*/
RE::NiPoint3 Utils::get_abs_pos(RE::Actor* a_actor, RE::NiPoint3 a_relative_pos)  // Hacked this together in a very short time; there might be better solutions.
{
	float len = sqrt(a_relative_pos.x * a_relative_pos.x + a_relative_pos.y * a_relative_pos.y);
	float actorAngleZ = a_actor->GetAngleZ();
	actorAngleZ -= 2 * actorAngleZ; /*Invert angle across axis*/

	float vecAngle = atan2(a_relative_pos.y, a_relative_pos.x);
	float vecAngleZ = actorAngleZ + vecAngle;

	float x_displacement = len * cos(vecAngleZ);
	float y_displacement = len * sin(vecAngleZ);
	float z_displacement = a_relative_pos.z;

	RE::NiPoint3 abs_pos = a_actor->GetPosition();
	abs_pos.x += x_displacement;
	abs_pos.y += y_displacement;
	abs_pos.z += z_displacement;

	return abs_pos;
}

/*Send the target flying based on causer's location.
	@param magnitude: strength of a push.*/
void Utils::PushActorAway(RE::Actor* causer, RE::Actor* target, float magnitude)
{
	auto targetPoint = causer->GetNodeByName(causer->GetActorRuntimeData().race->bodyPartData->parts[0]->targetName.c_str());
	RE::NiPoint3 vec = targetPoint->world.translate;
	//RE::NiPoint3 vec = causer->GetPosition();
	RE::Offset::pushActorAway(causer->GetActorRuntimeData().currentProcess, target, vec, magnitude);
}

void Utils::SetRotationMatrix(RE::NiMatrix3& a_matrix, float sacb, float cacb, float sb)
{
	float cb = std::sqrtf(1 - sb * sb);
	float ca = cacb / cb;
	float sa = sacb / cb;
	a_matrix.entry[0][0] = ca;
	a_matrix.entry[0][1] = -sacb;
	a_matrix.entry[0][2] = sa * sb;
	a_matrix.entry[1][0] = sa;
	a_matrix.entry[1][1] = cacb;
	a_matrix.entry[1][2] = -ca * sb;
	a_matrix.entry[2][0] = 0.0;
	a_matrix.entry[2][1] = sb;
	a_matrix.entry[2][2] = cb;
}

/*Slow down game time for a set period.
	@param a_duration: duration of the slow time.
	@param a_percentage: relative time speed to normal time(1).*/
void Utils::slowTime(float a_duration, float a_percentage)
{
	int duration_milisec = static_cast<int>(a_duration * 1000);
	RE::Offset::SGTM(a_percentage);
	/*Reset time here*/
	auto resetSlowTime = [](int a_duration) {
		std::this_thread::sleep_for(std::chrono::milliseconds(a_duration));
		RE::Offset::SGTM(1);
	};
	std::jthread resetThread(resetSlowTime, duration_milisec);
	resetThread.detach();
}

RE::TESObjectWEAP* Utils::Actor::getWieldingWeapon(RE::Actor* a_actor)
{
	auto weapon = a_actor->GetAttackingWeapon();
	if (weapon) {
		return weapon->object->As<RE::TESObjectWEAP>();
	}
	auto rhs = a_actor->GetEquippedObject(false);
	if (rhs && rhs->IsWeapon()) {
		return rhs->As<RE::TESObjectWEAP>();
	}
	auto lhs = a_actor->GetEquippedObject(true);
	if (lhs && lhs->IsWeapon()) {
		return lhs->As<RE::TESObjectWEAP>();
	}
	return nullptr;
}

bool Utils::Actor::isDualWielding(RE::Actor* a_actor)
{
	auto lhs = a_actor->GetEquippedObject(true);
	auto rhs = a_actor->GetEquippedObject(false);
	if (lhs && rhs && lhs->IsWeapon() && rhs->IsWeapon()) {
		auto weaponType = rhs->As<RE::TESObjectWEAP>()->GetWeaponType();
		return weaponType != RE::WEAPON_TYPE::kTwoHandAxe && weaponType != RE::WEAPON_TYPE::kTwoHandSword;  //can't be two hand sword.
	}
	return false;
}

bool Utils::Actor::isEquippedShield(RE::Actor* a_actor)
{
	return RE::Offset::getEquippedShield(a_actor);
}

bool Utils::Actor::isPowerAttacking(RE::Actor* a_actor)
{
	auto currentProcess = a_actor->GetActorRuntimeData().currentProcess;
	if (currentProcess) {
		auto highProcess = currentProcess->high;
		if (highProcess) {
			auto attackData = highProcess->attackData;
			if (attackData) {
				auto flags = attackData->data.flags;
				return flags.any(RE::AttackData::AttackFlag::kPowerAttack);
			}
		}
	}
	return false;
}

bool Utils::Actor::isBashing(RE::Actor* a_actor)
{
	if (a_actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) {
		return true;
	}
	auto currentProcess = a_actor->GetActorRuntimeData().currentProcess;
	if (currentProcess) {
		auto highProcess = currentProcess->high;
		if (highProcess) {
			auto attackData = highProcess->attackData;
			if (attackData) {
				auto flags = attackData->data.flags;
				return flags.any(RE::AttackData::AttackFlag::kBashAttack);
			}
		}
	}
	return false;
}

bool Utils::Actor::isHumanoid(RE::Actor* a_actor)
{
	auto bodyPartData = a_actor->GetRace() ? a_actor->GetRace()->bodyPartData : nullptr;
	return bodyPartData && bodyPartData->GetFormID() == 0x1d;
}

void Utils::Actor::getBodyPos(RE::Actor* a_actor, RE::NiPoint3& pos)
{
	if (!a_actor->GetActorRuntimeData().race) {
		return;
	}
	RE::BGSBodyPart* bodyPart = a_actor->GetActorRuntimeData().race->bodyPartData->parts[0];
	if (!bodyPart) {
		return;
	}
	auto targetPoint = a_actor->GetNodeByName(bodyPart->targetName.c_str());
	if (!targetPoint) {
		return;
	}
	pos = targetPoint->world.translate;
}

void Utils::Actor::getHeadPos(RE::Actor* a_actor, RE::NiPoint3& pos) 
{
	RE::NiAVObject* targetPoint = nullptr;
	getHeadPos(a_actor, targetPoint);
	if (!targetPoint) {
		return;
	}
	pos = targetPoint->world.translate;
}

/* Get A_ACTOR's head position, storing it in POS.*/
void Utils::Actor::getHeadPos(RE::Actor* a_actor, RE::NiAVObject*& pos)
{
	if (!a_actor->GetActorRuntimeData().race) {
		return;
	}
	RE::BGSBodyPart* headPart = a_actor->GetActorRuntimeData().race->bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kHead];
	if (!headPart) {
		return;
	}
	auto targetPoint = a_actor->GetNodeByName(headPart->targetName.c_str());
	if (!targetPoint) {
		return;
	}
	pos = targetPoint;
}

void Utils::Actor::dropShield(RE::Actor* a_actor)
{
	auto shd = RE::Offset::getEquippedShield(a_actor);
	if (shd) {
		auto shield = shd->As<RE::TESObjectARMO>();
		if (shield) {
			auto invShield = a_actor->GetInventory([](RE::TESBoundObject& a_obj) { return a_obj.IsArmor(); });
			for (auto& pair : invShield) {
				auto boundObj = pair.first;
				if (boundObj && boundObj->GetFormID() == shield->GetFormID()) {
					logger::info("dropping {}", boundObj->GetName());
					const auto pos = a_actor->GetNodeByName("SHIELD")->world.translate;
					a_actor->DropObject(boundObj, nullptr, 1, &pos, 0);
				}
			}
		}
	}
}

bool ValhallaUtils::is_adversary(RE::Actor* actor1, RE::Actor* actor2)
{
	bool is_adversary = false;

	if (actor1->GetActorRuntimeData().currentCombatTarget == actor2->GetHandle() || actor2->GetActorRuntimeData().currentCombatTarget == actor1->GetHandle()) {
		is_adversary = true;
	}

	if (!is_adversary) {
		auto combatGroup = actor1->GetCombatGroup();
		if (combatGroup) {
			for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
				if (it->targetHandle && it->targetHandle.get().get() && it->targetHandle.get().get() == actor2) {
					is_adversary = true;
				}
			}
		}
	}

	if (!is_adversary) {
		auto combatGroup = actor2->GetCombatGroup();
		if (combatGroup) {
			for (auto it = combatGroup->targets.begin(); it != combatGroup->targets.end(); ++it) {
				if (it->targetHandle && it->targetHandle.get().get() && it->targetHandle.get().get() == actor1) {
					is_adversary = true;
				}
			}
		}
	}
	return is_adversary;
}

bool ValhallaUtils::isBackFacing(RE::Actor* actor1, RE::Actor* actor2)
{
	auto angle = actor1->GetHeadingAngle(actor2->GetPosition(), false);
	if (90 < angle || angle < -90) {
		return true;
	} else {
		return false;
	}
}

float Utils::math::NormalRelativeAngle(float a_angle)
{
	while (a_angle > PI)
		a_angle -= TWO_PI;
	while (a_angle < -PI)
		a_angle += TWO_PI;
	return a_angle;
}

/*Do a simple raycast at a singular point to check if anything exists there.
		If anything exists, update A_POS argument to the position where raycast is hit.*/

bool DtryUtils::rayCast::object_exists(RE::NiPoint3& a_pos, float a_range)
{
	RE::NiPoint3 rayStart = a_pos;
	RE::NiPoint3 rayEnd = a_pos;
	rayStart.z += a_range;
	rayEnd.z -= a_range;
	auto havokWorldScale = RE::bhkWorld::GetWorldScale();
	RE::bhkPickData pick_data;
	pick_data.rayInput.from = rayStart * havokWorldScale;
	pick_data.rayInput.to = rayEnd * havokWorldScale;
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (!pc) {
		return false;
	}
	if (!pc->GetParentCell() || !pc->GetParentCell()->GetbhkWorld()) {
		return false;
	}
	pc->GetParentCell()->GetbhkWorld()->PickObject(pick_data);
	if (pick_data.rayOutput.HasHit()) {
		RE::NiPoint3 hitpos = rayStart + (rayEnd - rayStart) * pick_data.rayOutput.hitFraction;
		a_pos = hitpos;  //update the position to the hit position
		return true;
	}
	return false;
}

/*Cast a ray from the center of the actor to a_rayEnd, return the first object encountered, or nullptr if nothing is hit.*/

RE::TESObjectREFR* DtryUtils::rayCast::cast_ray(RE::Actor* a_actor, RE::NiPoint3 a_rayEnd, float a_castPos, float* ret_rayDist)
{
	auto havokWorldScale = RE::bhkWorld::GetWorldScale();
	RE::bhkPickData pick_data;
	RE::NiPoint3 rayStart = a_actor->GetPosition();
	float castHeight = a_actor->GetHeight() * a_castPos;
	rayStart.z += castHeight;  //cast from center of actor
							   /*Setup ray*/
	pick_data.rayInput.from = rayStart * havokWorldScale;
	pick_data.rayInput.to = a_rayEnd * havokWorldScale;

	/*Setup collision filter, ignoring the actor.*/
	uint32_t collisionFilterInfo = 0;
	a_actor->GetCollisionFilterInfo(collisionFilterInfo);
	uint16_t collisionGroup = collisionFilterInfo >> 16;
	pick_data.rayInput.filterInfo = (static_cast<uint32_t>(collisionGroup) << 16) | static_cast<uint32_t>(RE::COL_LAYER::kCharController);
	/*Do*/

	/*if (settings::bDodgeAI_DebugDraw_Enable && API::TrueHUD_API_acquired) {
		API::_TrueHud_API->DrawArrow(rayStart, a_rayEnd, 10, 1);
	}*/
	a_actor->GetParentCell()->GetbhkWorld()->PickObject(pick_data);
	if (pick_data.rayOutput.HasHit()) {
		RE::NiPoint3 hitpos = rayStart + (a_rayEnd - rayStart) * pick_data.rayOutput.hitFraction;
		if (ret_rayDist) {
			*ret_rayDist = hitpos.GetDistance(rayStart);
		}
		/*if (settings::bDodgeAI_DebugDraw_Enable && API::TrueHUD_API_acquired) {
			API::_TrueHud_API->DrawPoint(hitpos, 10, 3);
		}*/
		auto collidable = pick_data.rayOutput.rootCollidable;
		if (collidable) {
			RE::TESObjectREFR* ref = RE::TESHavokUtilities::FindCollidableRef(*collidable);
			if (ref) {
				return ref;
			}
		}
	}
	return nullptr;
}
