#pragma once

#include <DxLib.h>
#include <string>
#include <vector>
#include <map>
#include "FileManager.h"

/* ------------------ BGM ------------------ */


/* ------------------ SFX ------------------ */

enum class SFX {
    DUMMY,
    COUNT
};

const char* getSFXName(SFX type);

struct PlayingSFX {
    ~PlayingSFX();

    SFX type;
    int handle = -1;
    bool paused = false;
    LONGLONG pausedPosition = -1;

    void pause();
    void resume();
    void stop();
    
    bool isPlaying();

    void setVolume(int volume); // 0-255
    void setPan(int pan);       // -255 (left) to 255 (right)
};

class SFXPlayer {
public:
    ~SFXPlayer();

    static void init();
    static void update();

    static void load(const SFX type, const std::string& path);
    static void unload(const SFX type);
    static void unloadAll();

    static void stopAll();
    static void pauseAll();
    static void resumeAll();

    static std::shared_ptr<PlayingSFX> play(const SFX type, int volume = 255, int pan = 0);

    static const std::vector<std::shared_ptr<PlayingSFX>>& getPlayingSFXs() { return playingSFXs; }    

private:
    static std::map<SFX, std::shared_ptr<FileBuffer>> sfxs;
    static std::vector<std::shared_ptr<PlayingSFX>> playingSFXs;
};