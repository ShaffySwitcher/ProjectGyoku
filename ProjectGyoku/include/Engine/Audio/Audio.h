#pragma once

#include <DxLib.h>
#include <string>
#include <vector>
#include <map>
#include "Engine/FileManager.h"
#include "Engine/Math/Interpolation.h"

/* ------------------ BGM ------------------ */

struct BGMLoopPoint {
    uint32_t start;
    uint32_t end;
};

class BGMPlayer {
public:
    ~BGMPlayer();

    static void update();

    static void play(const std::string& name, uint8_t volume = 255);
    
    static void pause();
    static void resume();
    static void stop();
    
    static void fade(unsigned int duration, uint8_t target = 0);

    static void setVolume(uint8_t volume);

private:
    static int handle;
    static BGMLoopPoint loopPoint;
    static LONGLONG position;
    static uint8_t volume;
    static std::shared_ptr<Interpolator<uint8_t>> fadeInterpolator;
};

/* ------------------ SFX ------------------ */

enum class SFX {
    DUMMY,
    PAUSE,
    DEATH,
    SHOT,
    EXTEND,
    COUNT
};

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