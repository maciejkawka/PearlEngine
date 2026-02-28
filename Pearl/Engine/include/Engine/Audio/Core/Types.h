#pragma once

#include "Core/Math/Math.h"

namespace PrAudio {

	enum class SpeakerMode
	{
		Default,
		Raw,
		Mono,
		Stereo,
		Quad,
		Surround,
		_5Point1,
		_7Point1,
		Unsupported
	};

	enum class StopMode {
		AllowFadeOut,
		Immediate
	};

	struct Attributes3D
	{
		PrCore::Math::vec3 position{ 0.0f };
		PrCore::Math::vec3 velocity{ 0.0f };
		PrCore::Math::vec3 forwardVec{ 0.0f };
		PrCore::Math::vec3 upVec{ 0.0f };
	};

	struct DriverInfo
	{
		int           id;
		std::string   name;
		int           systemRate;
		SpeakerMode   speakerMode;
		int           speakerModeChannels;
	};

	struct SoundListener
	{
		int                index;
		PrCore::Math::vec3 position;
		PrCore::Math::vec3 velocity;
		PrCore::Math::vec3 forwardVec;
		PrCore::Math::vec3 upVec;
	};
}