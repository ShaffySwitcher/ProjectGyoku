#include "Engine/Format/SHT.h"
#include "Engine/FileManager.h"
#include "Engine/Log.h"

SHT SHT::load(const std::string& path) {
    auto file = FileManager::loadFile(path);
    if(!file) {
        Log::error("SHT::load(): Failed to load file: %s", path.c_str());
    }

    SHT sht;
    sht.shotCount = file->read<int16_t>();
    sht.bombs = file->read<uint8_t>();
    sht.deathBombWindow = file->read<uint32_t>();
    sht.hitbox = file->read<float>();
    sht.grazebox = file->read<float>();
    sht.itemMagnetSpeed = file->read<float>();
    sht.itemBox = file->read<float>();
    sht.pocLine = file->read<float>();
    sht.playerSpeed = file->read<PlayerSpeed>();

    const size_t headerEnd = file->tell();

    for (int i = 0; i < sht.shotCount; i++) {
        file->seek(headerEnd + i * 8);
        const int offset = file->read<int>();
        const int power  = file->read<int>();

        file->seek(offset);
        std::vector<Shot> powerShots;

        while(true) {
            Shot shot;
            shot.interval = file->read<int16_t>();
            shot.delay = file->read<int16_t>();

            if(shot.interval == -1 && shot.delay == -1) {
                break;
            }

            shot.position = Vector(file->read<float>(), file->read<float>(), 0);
            shot.hitbox = Vector(file->read<float>(), file->read<float>(), 0);
            shot.angle = file->read<float>();
            shot.speed = file->read<float>();
            shot.damage = file->read<uint16_t>();
            shot.orb = file->read<uint8_t>();
            shot.type = file->read<uint8_t>();
            shot.sprite = file->read<uint16_t>();
            shot.sfx = file->read<uint16_t>();

            powerShots.push_back(shot);
        }

        sht.shots[power] = powerShots;
    }

    return sht;
}