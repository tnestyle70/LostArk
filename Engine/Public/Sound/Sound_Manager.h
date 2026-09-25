#pragma once

#include "Engine_Defines.h"
#include "Sound/TrackedSoundChannel.h"

#include <mutex>

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	class ChannelGroup;
}

NS_BEGIN(Engine)

/* Mixer buses the system option window turns into user volume. MASTER is FMOD's own
master group, so it scales the other three; MUSIC is what Play_Music drives, INTERFACE is
every one-shot whose asset lives under Sound/UI/, and EFFECT is every other playback path. */
enum class SOUND_CATEGORY : uint32_t { MASTER, MUSIC, EFFECT, INTERFACE, END };

class CSound_Manager final
{
private:
	CSound_Manager();

public:
	~CSound_Manager();

public:
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
	uint64_t Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs = 0u, bool_t paused = false, f32_t playbackRate = 1.f);
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

	/* Bus volume in 0..1, applied to every channel already playing on that bus.
	Muting a category is fVolume 0: focus loss owns the master group's mute flag
	(see Update_ApplicationFocusMute) and user settings must not fight it. */
	HRESULT Apply_CategoryVolume(SOUND_CATEGORY eCategory, f32_t fVolume);
	f32_t Get_CategoryVolume(SOUND_CATEGORY eCategory) const;
	/* sound-in-background, inverted: true (the default) mutes the master group while
	another process owns the foreground window; false keeps playing. */
	void Set_MuteOnFocusLoss(bool_t bMute);

	void Update();

private:
	HRESULT Initialize();
	HRESULT Ready_CategoryGroups();
	bool_t Update_ApplicationFocusMute();
	FMOD::ChannelGroup* Find_CategoryGroup(SOUND_CATEGORY eCategory) const;
	/* Bus for a one-shot asset: Sound/UI/... is the interface bus, anything else effects. */
	FMOD::ChannelGroup* Pick_OneShotGroup(const wstring_t& strSoundFilePath) const;
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop);
	HRESULT Play_TrackedSound(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop, CTrackedSoundChannel<FMOD::Channel>& channel,
		FMOD::ChannelGroup* pGroup);

private:
	FMOD::System* m_pSystem = { nullptr };
	/* Children of FMOD's master group, created once in Initialize. Playback passes the
	matching group so a later volume change reaches sounds that are already running. */
	FMOD::ChannelGroup* m_pMusicGroup = { nullptr };
	FMOD::ChannelGroup* m_pEffectGroup = { nullptr };
	FMOD::ChannelGroup* m_pInterfaceGroup = { nullptr };
	f32_t m_CategoryVolumes[ETOUI(SOUND_CATEGORY::END)] = { 1.f, 1.f, 1.f, 1.f };
	bool_t m_bMuteOnFocusLoss = true;
	bool_t m_bFocusMuteInitialized = false;
	bool_t m_bFocusMuted = true;
	/* FMOD loop mode belongs to the Sound, not the Channel. Keep one cached
	instance per (asset, mode) so an SFX and a music cue cannot mutate each
	other when they happen to reference the same WAV. */
	map<pair<wstring_t, bool_t>, FMOD::Sound*> m_Sounds;
	mutex m_SoundsMutex;
	map<uint64_t, FMOD::Channel*> m_CueChannels;
	uint64_t m_iNextCueHandle = 1u;
	CTrackedSoundChannel<FMOD::Channel> m_MusicChannel;
	CTrackedSoundChannel<FMOD::Channel> m_LoopingSoundChannel;

public:
	static unique_ptr<CSound_Manager> Create();
};

NS_END
