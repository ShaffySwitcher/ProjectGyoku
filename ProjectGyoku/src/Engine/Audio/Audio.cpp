#include "Engine/Audio/Audio.h"
#include "Engine/Log.h"
#include "Engine/Supervisor.h"
#include "Engine/Utils.h"

/* ------------------ BGM ------------------ */

int BGMPlayer::handle = -1;
BGMLoopPoint BGMPlayer::loopPoint = { 0, 0 };
LONGLONG BGMPlayer::position = 0;
uint8_t BGMPlayer::volume = 255;
std::shared_ptr<Interpolator<uint8_t>> BGMPlayer::fadeInterpolator = nullptr;

BGMPlayer::~BGMPlayer()
{
    if (handle != -1) {
        StopSoundMem(handle);
        DeleteSoundMem(handle);
        handle = -1;
    }
}

void BGMPlayer::update()
{
    if(BGMPlayer::fadeInterpolator) {
        BGMPlayer::fadeInterpolator->update(gSupervisor.currentFrame);
        volume = BGMPlayer::fadeInterpolator->getValue();

        if (BGMPlayer::fadeInterpolator->isFinished()) { BGMPlayer::fadeInterpolator.reset(); }
    }

    BGMPlayer::setVolume(static_cast<uint8_t>(volume * (static_cast<float>(gSupervisor.config.bgmVolume) / 100.0f)));
    BGMPlayer::position = GetCurrentPositionSoundMem(BGMPlayer::handle);
}

void BGMPlayer::play(const std::string &name, uint8_t volume)
{
    BGMPlayer::stop();

    switch(gSupervisor.config.bgmType) {
        case static_cast<uint8_t>(GameConfigMusicMode::WAV): {
            std::string path = "wav/" + name + ".wav";

            std::shared_ptr<FileBuffer> bgmFile = FileManager::loadFile(path);
            if (!bgmFile) {
                Log::error("BGMPlayer::play(): Failed to load BGM file: %s", path.c_str());
                return;
            }

            BGMLoopPoint loopPoint = { 0, 0 };
            std::shared_ptr<FileBuffer> loopPointFile = FileManager::loadFile("data/music/" + name + ".pos");

            // File may not loop, so no crashes or warnings here
            if (loopPointFile) {
                loopPoint.start = loopPointFile->read<uint32_t>();
                loopPoint.end = loopPointFile->read<uint32_t>();
            }

            BGMPlayer::handle = LoadSoundMemByMemImage(bgmFile->data, static_cast<int>(bgmFile->size));
            if (BGMPlayer::handle == -1) {
                Log::error("BGMPlayer::play(): LoadSoundMemByMemImage failed for BGM: %s", path.c_str());
                return;
            }

            BGMPlayer::loopPoint = loopPoint;
            if(loopPoint.end > 0) {
                SetLoopAreaSamplePosSoundMem(loopPoint.start, loopPoint.end, BGMPlayer::handle);
            }

            PlaySoundMem(BGMPlayer::handle, DX_PLAYTYPE_LOOP);

            break;
        }
    }
}

void BGMPlayer::pause()
{
    StopSoundMem(BGMPlayer::handle);
}

void BGMPlayer::resume()
{
    PlaySoundMem(BGMPlayer::handle, DX_PLAYTYPE_LOOP);
    SetCurrentPositionSoundMem(BGMPlayer::position, BGMPlayer::handle);
}

void BGMPlayer::stop()
{
	StopSoundMem(BGMPlayer::handle);
	DeleteSoundMem(BGMPlayer::handle);
}

void BGMPlayer::fade(unsigned int duration, uint8_t target)
{
    BGMPlayer::fadeInterpolator = std::make_shared<Interpolator<uint8_t>>(volume, gSupervisor.currentFrame, target, gSupervisor.currentFrame + duration);
}

void BGMPlayer::setVolume(uint8_t volume)
{
    if (handle != -1) {
        ChangeVolumeSoundMem(volume, handle);
    }
}

/* ------------------ SFX ------------------ */

const char* getSFXName(SFX type)
{
    switch (type) {
    case SFX::DUMMY:
        return "DUMMY";
    case SFX::COUNT:
        return "COUNT";
    default:
        return "UNKNOWN";
    }
}

// ------ PlayingSFX ------

PlayingSFX::~PlayingSFX()
{
    if (handle != -1){
        StopSoundMem(handle);
        DeleteSoundMem(handle);
        handle = -1;
    }
}

void PlayingSFX::pause()
{
    if (handle != -1 && !paused) {
        pausedPosition = GetCurrentPositionSoundMem(handle);
        StopSoundMem(handle);
        paused = true;
    }
}

void PlayingSFX::resume()
{
    if (handle != -1 && paused) {
        PlaySoundMem(handle, DX_PLAYTYPE_BACK);
        if (pausedPosition > 0) {
            SetCurrentPositionSoundMem(pausedPosition, handle);
        }
        paused = false;
    }
}

void PlayingSFX::stop()
{
    if (handle != -1) {
        StopSoundMem(handle);
        paused = false;
        pausedPosition = -1;
    }
}

bool PlayingSFX::isPlaying() {
    if (handle == -1 || paused) return false;
    return CheckSoundMem(handle) == 1;
}

void PlayingSFX::setVolume(int volume)
{
    if (handle != -1) {
        ChangeVolumeSoundMem(volume, handle);
    }
}

void PlayingSFX::setPan(int pan)
{
    if (handle != -1) {
        ChangePanSoundMem(pan, handle);
    }
}

// ------ SFXPlayer ------

std::map<SFX, std::shared_ptr<FileBuffer>> SFXPlayer::sfxs;
std::vector<std::shared_ptr<PlayingSFX>> SFXPlayer::playingSFXs;

SFXPlayer::~SFXPlayer()
{
    unloadAll();
}

void SFXPlayer::init()
{
    load(SFX::DUMMY, "data/sound/dummy.wav");
    load(SFX::PAUSE, "data/sound/pause.wav");
}

void SFXPlayer::load(const SFX type, const std::string &path)
{
    if (sfxs.find(type) != sfxs.end()) {
        Log::print("SFXPlayer::load(): SFX of type %d is already loaded, skipping", static_cast<int>(type));
        return;
    }

    std::shared_ptr<FileBuffer> soundFile = FileManager::loadFile(path);
    if (!soundFile) {
        Log::error("SFXPlayer::load(): Failed to load sound effect file: %s", path.c_str());
        SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMPRESS);
        return;
    }

    sfxs[type] = soundFile;
}

void SFXPlayer::update() {
    for(auto it = playingSFXs.begin(); it != playingSFXs.end(); ) {
        if (!(*it)->isPlaying() && !(*it)->paused) {
            it = playingSFXs.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SFXPlayer::unload(const SFX type)
{
    auto it = sfxs.find(type);
    if (it != sfxs.end()) {
        sfxs.erase(it);
    }
}

void SFXPlayer::unloadAll()
{
    stopAll();
    sfxs.clear();
}

void SFXPlayer::stopAll()
{
    for (const auto& sfx : playingSFXs) {
        sfx->stop();
    }
}

void SFXPlayer::pauseAll()
{
    for (const auto& sfx : playingSFXs) {
        sfx->pause();
    }
}

void SFXPlayer::resumeAll()
{
    for (const auto& sfx : playingSFXs) {
        sfx->resume();
    }
}

std::shared_ptr<PlayingSFX> SFXPlayer::play(const SFX type, int volume, int pan)
{
    if (!gSupervisor.config.useSfx) {
        return nullptr;
    }

    auto it = sfxs.find(type);
    if (it == sfxs.end()) {
        Log::print("SFXPlayer::play(): SFX of type %d is not loaded, cannot play", static_cast<int>(type));
        return nullptr;
    }

    int handle = LoadSoundMemByMemImage(it->second->data, static_cast<int>(it->second->size));
    if (handle == -1) {
        Log::error("SFXPlayer::play(): LoadSoundMemByMemImage failed for type %d", static_cast<int>(type));
        return nullptr;
    }

    const int clampedRequestedVolume = clamp(volume, 0, 255);
    const int clampedConfigVolume = clamp(static_cast<int>(gSupervisor.config.sfxVolume), 0, 100);
    const int scaledVolume = (clampedRequestedVolume * clampedConfigVolume + 50) / 100;

    ChangeVolumeSoundMem(scaledVolume, handle);
    ChangePanSoundMem(pan, handle);

    if (PlaySoundMem(handle, DX_PLAYTYPE_BACK) == -1) {
        Log::error("SFXPlayer::play(): PlaySoundMem failed for type %d", static_cast<int>(type));
        DeleteSoundMem(handle);
        return nullptr;
    }

    auto instance = std::make_shared<PlayingSFX>();
    instance->type = type;
    instance->handle = handle;
    instance->paused = false;
    instance->pausedPosition = -1;

    playingSFXs.push_back(instance);
    return instance;
}