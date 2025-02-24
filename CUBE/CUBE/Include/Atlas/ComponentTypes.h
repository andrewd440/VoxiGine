#pragma once

#include <cstdint>


namespace Atlas
{
	struct EComponent
	{
		enum Type : uint32_t
		{
			DirectionalLight,
			PointLight,
			RigidBody,
			MeshRenderer,
			Collider,
			SoundListener,
			SoundEmitter,
			Count
		};
	};

	namespace ComponentTraits
	{
		template <EComponent::Type T>
		struct Object
		{
			using Type = void;
		};
	}
}