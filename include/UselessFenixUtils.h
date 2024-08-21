#pragma once
#include "include/Utils.h"

template <int id, typename T, typename... Args>
T _generic_foo(Args... args)
{
	using func_t = T(Args...);
	REL::Relocation<func_t> func{ REL::ID(id) };
	return func(std::forward<Args>(args)...);
}

// static const float PI = 3.141592653f;

namespace Utils
{
	RE::BGSAttackData* get_attackData(RE::Actor* a);

	struct PolarAngle
	{
		float alpha;
		operator float() const { return alpha; }
		PolarAngle(float x = 0.0f) : alpha(x)
		{
			while (alpha > 360.0f)
				alpha -= 360.0f;
			while (alpha < 0.0f)
				alpha += 360.0f;
		}
		PolarAngle(const RE::NiPoint3& p) {
			const float y = p.y;
			if (y == 0.0) {
				if (p.x <= 0.0)
					alpha = PI * 1.5f;
				else
					alpha = PI * 0.5f;
			} else {
				alpha = atanf(p.x / y);
				if (y < 0.0)
					alpha += PI;
			}
			alpha = alpha * 180.0f / PI;
		}
		PolarAngle add(const PolarAngle& r) const {
			float ans = alpha + r.alpha;
			if (ans >= 360.0f)
				return { ans - 360.0f };
			else
				return { ans };
		}
		PolarAngle sub(const PolarAngle& r) const {
			return this->add({ 360.0f - r.alpha });
		}
		float to_normangle() const {
			if (alpha > 180.0f)
				return alpha - 360.0f;
			else
				return alpha;
		}
		float to_normangle_abs() const {
			return abs(to_normangle());
		}
		static bool ordered(PolarAngle alpha, PolarAngle beta, PolarAngle gamma)
		{
			gamma = gamma.sub(alpha);
			beta = beta.sub(alpha);
			return gamma >= beta;
		}
		static float dist(float r, PolarAngle alpha)
		{
			auto ans = r * (alpha) / 180.0f * PI;
			return ans;
		}
	};
}

RE::Character* CombatAI__get_he();
RE::Character* CombatAI__get_me();
float get_total_av(RE::Actor* a, RE::ActorValue av);
