#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#define INVALID_SOUND_HANDLE -1

#define MAX_SOUNDS 64

// Sound backend selection
#define SOUND_BACKEND_MINIAUDIO 1
#define SOUND_BACKEND_NULL 0

typedef int HSOUND;

// Sound system interface
class ISoundSystem
{
public:
	virtual ~ISoundSystem() {}

	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	virtual HSOUND LoadSound(const char* filename) = 0;

	virtual void Play(HSOUND handle, bool bLoop) = 0;
	virtual void Stop(HSOUND handle) = 0;

	virtual bool IsPlaying(HSOUND handle) = 0;

	virtual void SetMasterVolume(float volume) = 0;
	virtual float GetMasterVolume() = 0;
};

extern ISoundSystem* g_pSoundSystem;

#endif // !SOUNDSYSTEM_H
