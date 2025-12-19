#include "debug.h"
#include "sound.h"

#if SOUND_BACKEND_MINIAUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

class CSoundSystem_Miniaudio : public ISoundSystem
{
public:
	CSoundSystem_Miniaudio();
	~CSoundSystem_Miniaudio();

	void Init();
	void Shutdown();

	HSOUND LoadSound(const char* filename);

	void Play(HSOUND handle, bool bLoop);
	void Stop(HSOUND handle);

	bool IsPlaying(HSOUND handle);

	void SetMasterVolume(float volume);
	float GetMasterVolume();

private:
	ma_sound m_sounds[MAX_SOUNDS];
	ma_engine m_engine;
	int m_dwSoundsNum;
	bool m_bInited;
};

static CSoundSystem_Miniaudio s_SoundSystem;
ISoundSystem* g_pSoundSystem = &s_SoundSystem;

CSoundSystem_Miniaudio::CSoundSystem_Miniaudio()
{
	memset(&m_engine, 0, sizeof(m_engine));
	memset(&m_sounds, 0, sizeof(m_sounds));

	m_bInited = false;
	m_dwSoundsNum = 0;
}

CSoundSystem_Miniaudio::~CSoundSystem_Miniaudio()
{
}

void CSoundSystem_Miniaudio::Init()
{
	ma_result result;

	DPrintf("Sound initialization\n");

	result = ma_engine_init(NULL, &m_engine);
	if ( result != MA_SUCCESS )
	{
		//DPrintf("Failed to initialize the miniaudio engine. %s\n", ma_result_description(result));
		DError("Failed to initialize the miniaudio engine. %s\n", ma_result_description(result));
		return;  // Failed to initialize the engine.
	}

	ma_uint32 uiChannels = ma_engine_get_channels(&m_engine);
	DPrintf("%d channel(s)\n", uiChannels);

	ma_uint32 uiSampleRate = ma_engine_get_sample_rate(&m_engine);
	DPrintf("Sound sampling rate: %d\n", uiSampleRate);

	m_bInited = true;
}

void CSoundSystem_Miniaudio::Shutdown()
{
	if ( !m_bInited )
		return;

	for ( int i = 0; i < m_dwSoundsNum; i++ )
	{
		ma_sound_uninit(&m_sounds[i]);
	}

	memset(&m_sounds, 0, sizeof(m_sounds));

	ma_engine_uninit(&m_engine);
	memset(&m_engine, 0, sizeof(m_engine));
}

HSOUND CSoundSystem_Miniaudio::LoadSound(const char* filename)
{
	if ( !m_bInited )
		return -1;

	ma_result result;

	HSOUND handle = m_dwSoundsNum;

	result = ma_sound_init_from_file(&m_engine, filename, 0, NULL, NULL, &m_sounds[handle]);
	if ( result != MA_SUCCESS )
	{
		DPrintf("Failed to load sound %s\n", filename);
		return -1;
	}

	m_dwSoundsNum++;
	return handle;
}

void CSoundSystem_Miniaudio::Play(HSOUND handle, bool bLoop)
{
	if ( !m_bInited )
		return;

	assert(handle <= -1 || handle <= m_dwSoundsNum);

	ma_sound_start(&m_sounds[handle]);

	if ( bLoop )
		ma_sound_set_looping(&m_sounds[handle], true);
}

void CSoundSystem_Miniaudio::Stop(HSOUND handle)
{
	if ( !m_bInited )
		return;

	assert(handle <= -1 || handle <= m_dwSoundsNum);
	ma_sound_stop(&m_sounds[handle]);
}

bool CSoundSystem_Miniaudio::IsPlaying(HSOUND handle)
{
	if ( !m_bInited )
		return false;

	assert(handle <= -1 || handle <= m_dwSoundsNum);

	return ma_sound_is_playing(&m_sounds[handle]);
}

void CSoundSystem_Miniaudio::SetMasterVolume(float volume)
{
	if ( !m_bInited )
		return;

	ma_engine_set_volume(&m_engine, volume);
}

float CSoundSystem_Miniaudio::GetMasterVolume()
{
	return ma_engine_get_volume(&m_engine);
}
#endif

#if SOUND_BACKEND_NULL

class CSoundSystem_NULL : public ISoundSystem
{
public:
	CSoundSystem_NULL();
	~CSoundSystem_NULL();

	void Init();
	void Shutdown();

	HSOUND LoadSound(const char* filename);

	void Play(HSOUND handle, bool bLoop);
	void Stop(HSOUND handle);

	bool IsPlaying(HSOUND handle);

	void SetMasterVolume(float volume);
	float GetMasterVolume();

private:
	int m_dwSoundsNum;
};

static CSoundSystem_NULL s_SoundSystem;
ISoundSystem* g_pSoundSystem = &s_SoundSystem;

CSoundSystem_NULL::CSoundSystem_NULL()
{
	m_dwSoundsNum = 0;
}

CSoundSystem_NULL::~CSoundSystem_NULL()
{
}

void CSoundSystem_NULL::Init()
{
}

void CSoundSystem_NULL::Shutdown()
{
}

HSOUND CSoundSystem_NULL::LoadSound(const char* filename)
{
	HSOUND handle = m_dwSoundsNum;
	m_dwSoundsNum++;
	return handle;
}

void CSoundSystem_NULL::Play(HSOUND handle, bool bLoop)
{
}

void CSoundSystem_NULL::Stop(HSOUND handle)
{
}

bool CSoundSystem_NULL::IsPlaying(HSOUND handle)
{
	return false;
}

void CSoundSystem_NULL::SetMasterVolume(float volume)
{
}

float CSoundSystem_NULL::GetMasterVolume()
{
	return 0.0f;
}

#endif