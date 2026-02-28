#pragma once

#include "Audio/Core/Types.h"

#include "fmod/fmod_studio.hpp"

namespace PrAudio {

	inline bool CheckError(FMOD_RESULT result)
	{
		if (result == 0)
			return true;

		PRLOG_ERROR("Audio system returned error code: {} at line: {} file: {}", result, __LINE__, __FILE__);
		return false;
	}

	inline PrCore::Math::vec3 FromFmod(const FMOD_VECTOR& p_vec3)
	{
		return PrCore::Math::vec3{ p_vec3.x, p_vec3.y, p_vec3.z };
	}

	inline SpeakerMode FromFmod(FMOD_SPEAKERMODE speakerMode)
	{
		switch (speakerMode)
		{
		case FMOD_SPEAKERMODE_DEFAULT:
			return SpeakerMode::Default;
			break;
		case FMOD_SPEAKERMODE_RAW:
			return SpeakerMode::Raw;
			break;
		case FMOD_SPEAKERMODE_MONO:
			return SpeakerMode::Mono;
			break;
		case FMOD_SPEAKERMODE_STEREO:
			return SpeakerMode::Stereo;
			break;
		case FMOD_SPEAKERMODE_QUAD:
			return SpeakerMode::Quad;
			break;
		case FMOD_SPEAKERMODE_SURROUND:
			return SpeakerMode::Surround;
			break;
		case FMOD_SPEAKERMODE_5POINT1:
			return SpeakerMode::_5Point1;
			break;
		case FMOD_SPEAKERMODE_7POINT1:
			return SpeakerMode::_7Point1;
			break;
		case FMOD_SPEAKERMODE_7POINT1POINT4:
		case FMOD_SPEAKERMODE_MAX:
		case FMOD_SPEAKERMODE_FORCEINT:
			return SpeakerMode::Unsupported;
		default:
			return SpeakerMode::Unsupported;
			break;
		}
	}

	inline Attributes3D FromFmod(const FMOD_3D_ATTRIBUTES& p_attributes)
	{
		Attributes3D attributes{
			   FromFmod(p_attributes.position),
			   FromFmod(p_attributes.velocity),
			   FromFmod(p_attributes.forward),
			   FromFmod(p_attributes.up)};

		return attributes;
	}


	inline FMOD_VECTOR ToFmod(PrCore::Math::vec3 p_vec3)
	{
		return FMOD_VECTOR{ p_vec3.x, p_vec3.y, p_vec3.z };
	}

	inline FMOD_3D_ATTRIBUTES ToFmod(Attributes3D& p_attributes)
	{
		return FMOD_3D_ATTRIBUTES{
			ToFmod(p_attributes.position),
			ToFmod(p_attributes.velocity),
			ToFmod(p_attributes.forwardVec),
			ToFmod(p_attributes.upVec)
		};
	}

	inline FMOD_STUDIO_STOP_MODE ToFmod(StopMode mode)
	{
		if (mode == StopMode::AllowFadeOut)
			return FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_ALLOWFADEOUT;
		else
			return FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE;
	}
}