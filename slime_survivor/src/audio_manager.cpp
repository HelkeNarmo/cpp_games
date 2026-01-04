#include "audio_manager.h"
#include <bitset>
using namespace std;

const char *AudioManager::BGM_FILES[6] = {
    "../aud/bgm_menu.mp3",
    "../aud/bgm_action_1.mp3",
    "../aud/bgm_action_2.mp3",
    "../aud/bgm_action_3.mp3",
    "../aud/bgm_action_4.mp3",
    "../aud/bgm_action_5.mp3"
};

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

bool AudioManager::init() {
    int flags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        SDL_Log("Mix_Init failed: %s", Mix_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) != 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }
    Mix_AllocateChannels(32);
    return loadAudioAssets();
}

bool AudioManager::loadAudioAssets() {
    for (int i = 0; i < 6; ++i) {
        bgms_[i] = Mix_LoadMUS(BGM_FILES[i]);
        if (bgms_[i] == nullptr) {
            SDL_Log("Mix_LoadMUS failed: %s", Mix_GetError());
            return false;
        }
    }
    hurt_ = Mix_LoadWAV("../aud/hurt.wav");
    if (hurt_ == nullptr) {
        SDL_Log("Mix_LoadWAV failed: %s", Mix_GetError());
        return false;
    }
    return true;
}

void AudioManager::playBGM(int idx, float volume01) {
    Mix_VolumeMusic((int)(volume01 * MIX_MAX_VOLUME));
    if (Mix_PlayMusic(bgms_[idx], -1) != 0) {
        SDL_Log("Mix_PlayMusic failed: %s", Mix_GetError());
    }   
}

void AudioManager::stopBGM() {
    Mix_FadeOutMusic(1000);
}

void AudioManager::playHurt(float volume01) {
    Mix_VolumeChunk(hurt_, (int)(volume01 * MIX_MAX_VOLUME));
    if (Mix_PlayChannel(-1, hurt_, 0) == -1) {
        SDL_Log("Mix_PlayChannel failed: %s", Mix_GetError());
    }
}
