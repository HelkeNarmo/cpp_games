#pragma once
#include<SDL2/SDL.h>
#include<SDL2/SDL_mixer.h>

class AudioManager {
public:
    static AudioManager& getInstance();

    bool init();
    /**
     * 播放指定索引的BGM
     * @param idx BGM索引，范围[0, 5]
     */
    void playBGM(int idx, float volume01 = 1.0f);
    void stopBGM();
    void playHurt(float volume01 = 1.0f);

private:
    static const char *BGM_FILES[6];

    AudioManager() = default;
    ~AudioManager() = default;

    bool loadAudioAssets();

    Mix_Music *bgms_[6]{};
    Mix_Chunk *hurt_{};
};
