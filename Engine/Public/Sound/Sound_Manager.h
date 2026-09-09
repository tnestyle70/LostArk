#pragma once

#include "Engine_Defines.h"
#include "Sound/TrackedSoundChannel.h"

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
}

NS_BEGIN(Engine)

class CSound_Manager final
{
private:
	CSound_Manager();

public:
	~CSound_Manager();

public:
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
	uint64_t Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs = 0u, bool_t paused = false);
	bool_t Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs);
	bool_t Is_SoundCueActive(uint64_t handle) const;
	void Pause_SoundCue(uint64_t handle, bool_t paused);
	void Seek_SoundCue(uint64_t handle, uint32_t ageMs);
	void Stop_SoundCue(uint64_t handle);

	/* A separately owned looping SFX never replaces level/encounter music. */
	HRESULT Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Stop_LoopingSound();

	/* Unlike Play_Sound (fire-and-forget SFX), music keeps one tracked channel
	that a level or encounter can replace and stop. Cutscene music is one-shot;
	ambient/combat music can opt into an infinite loop. */
	HRESULT Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop = true);
	void Stop_Music();

	void Update();

private:
	HRESULT Initialize();
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop);
	HRESULT Play_TrackedSound(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop, CTrackedSoundChannel<FMOD::Channel>& channel);

private:
	FMOD::System* m_pSystem = { nullptr };
	/* FMOD loop mode belongs to the Sound, not the Channel. Keep one cached
	instance per (asset, mode) so an SFX and a music cue cannot mutate each
	other when they happen to reference the same WAV. */
	map<pair<wstring_t, bool_t>, FMOD::Sound*> m_Sounds;
	map<uint64_t, FMOD::Channel*> m_CueChannels;
	uint64_t m_iNextCueHandle = 1u;
	CTrackedSoundChannel<FMOD::Channel> m_MusicChannel;
	CTrackedSoundChannel<FMOD::Channel> m_LoopingSoundChannel;

public:
	static unique_ptr<CSound_Manager> Create();
};

NS_END
