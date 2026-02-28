#include "Core/Common/pearl_pch.h"

#include "Audio/Resources/SoundBankLoader.h"

#include "Audio/Resources/ISoundBank.h"
#include "Audio/Core/IAudioSystem.h"

#include "Core/Utils/SystemProvider.h"

using namespace PrAudio;

PrCore::IResourceDataPtr SoundBankLoader::LoadResource(const std::string& p_path)
{
	auto bankPtr = PrSystems::Get<IAudioSystem>()->LoadSoundBank(p_path);
	return bankPtr;
}

void SoundBankLoader::UnloadResource(PrCore::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool SoundBankLoader::SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
