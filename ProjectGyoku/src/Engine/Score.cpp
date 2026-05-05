#include "Engine/Score.h"
#include "Engine/Log.h"
#include "Engine/FileManager.h"
#include "Engine/Math/GyokuMath.h"
#include "Engine/Supervisor.h"
#include <vector>

Score *ScoreManager::currentScore = nullptr;

void Score::reset()
{
    memset(this, 0, sizeof(Score));
    memcpy_s(this->magic, 4, SCORE_FILE_MAGIC, 4);

    // PSCD
    for (auto& characterData : this->playerStageClearData) {
        for (auto& difficultyData : characterData) {
            for (auto& stageData : difficultyData) {
                stageData.base = PGSB::createBase(PSCD_MAGIC);
            }
        }
    }

    // CLRD
    for (auto& clearData : this->clearData) {
        clearData.base = PGSB::createBase(CLRD_MAGIC);
    }

    // HSCD
    for (auto& characterData : this->highscores) {
        for (auto& difficultyData : characterData) {
            for (size_t i = 0; i < HIGHSCORE_COUNT; ++i) {
                difficultyData[i].base = PGSB::createBase(HSCD_MAGIC);

                difficultyData[i].score = DEFAULT_HIGHSCORE - (i * 100000);
                difficultyData[i].state = static_cast<uint8_t>(HighscoreState::DEFAULT);
                memcpy_s(difficultyData[i].name, 8, DEFAULT_NAME, 8);
                difficultyData[i].time = DateTime::now();
            }
        }
    }

    // SPCD
    for (auto& spellcardData : this->spellcards) {
        spellcardData.base = PGSB::createBase(SPCD_MAGIC);
    }

    // PSTD
    this->playStats.base = PGSB::createBase(PSTD_MAGIC);
}

uint16_t Score::calculateChecksum() const
{
    uint16_t checksum = 0;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
    for (size_t i = 8; i < sizeof(Score); ++i) {
        checksum += data[i];
    }
    return checksum;
}

uint32_t Score::getHighscore(uint8_t character, uint8_t difficulty) const
{
    uint32_t highscore = DEFAULT_HIGHSCORE;
    for(auto& hs : highscores[character][difficulty]) {
        if(hs.score > highscore) {
            highscore = hs.score;
        }
    }
    return highscore;
}

bool Score::hasUnlockedExtra(uint8_t character) const
{
    return (this->clearData[character].stageCleared[static_cast<size_t>(Difficulty::NORMAL)] > static_cast<uint8_t>(Stage::STAGE_6)
        || this->clearData[character].stageCleared[static_cast<size_t>(Difficulty::HARD)] > static_cast<uint8_t>(Stage::STAGE_6)
        || this->clearData[character].stageCleared[static_cast<size_t>(Difficulty::ILLUSORY)] > static_cast<uint8_t>(Stage::STAGE_6));
}

void ScoreManager::load(const std::string &path)
{
    Score *score = new Score();
    std::shared_ptr<FileBuffer> scoreFile = FileManager::loadFile(path, true, true);
    std::vector<uint8_t> buffer;

    if(!scoreFile) {
        currentScore = score;
        return;
    }

    // Read file into a buffer
    buffer.resize(scoreFile->size);
    scoreFile->readBytes(buffer.data(), buffer.size());

    // Decrypt the buffer using the TWO seed (two pass)
    crypt(buffer.data(), buffer.size(), buffer[5]);
    crypt(buffer.data(), buffer.size(), buffer[4]);

    // Verify the checksum
    memcpy_s(score, sizeof(Score), buffer.data(), sizeof(Score));
    if(score->checksum != score->calculateChecksum() || memcmp(score->magic, SCORE_FILE_MAGIC, 4) != 0) {
        score->reset();
    }

    currentScore = score;
}

void ScoreManager::save(const std::string &path)
{
    // Calculate the checksum
    currentScore->checksum = currentScore->calculateChecksum();

    // Set the TWO seed (two pass) using random bytes
    currentScore->seed[0] = gRng.getIntRange(0, 255);
    currentScore->seed[1] = gRng.getIntRange(0, 255);

    // Encrypt the buffer using the TWO seed (two pass)
    std::vector<uint8_t> buffer(sizeof(Score));
    memcpy_s(buffer.data(), buffer.size(), currentScore, sizeof(Score));
    crypt(buffer.data(), buffer.size(), currentScore->seed[1]);
    crypt(buffer.data(), buffer.size(), currentScore->seed[0]);

    // Write the buffer to a file
    FileManager::saveFile(path, buffer.data(), buffer.size(), true);
}

void ScoreManager::crypt(uint8_t *data, size_t size, uint8_t key)
{
    for(size_t i = 6; i < size; ++i) {
        key = (key & 0xE0) >> 5 | (key & 0x1F) << 3; // Rotate key left by 3
        data[i] ^= key;
        key += static_cast<uint8_t>(i);
    }
} 

void ScoreManager::updatePlaytime()
{
    static int lastUpdateMS = GetNowCount();
    static uint32_t accumulatedMS = 0;

    if (!currentScore) {
        return;
    }

    const int now = GetNowCount();
    const int elapsedMS = now - lastUpdateMS;
    lastUpdateMS = now;

    if (elapsedMS > 0) {
        accumulatedMS += static_cast<uint32_t>(elapsedMS);
    }

    while (accumulatedMS >= 1000) {
        accumulatedMS -= 1000;
        currentScore->playStats.timePlayed += 1;

        if (gSupervisor.isInGame) {
            currentScore->playStats.timePlayedGame += 1;
        }
    }
}

Score *ScoreManager::getCurrentScore()
{
    return currentScore;
}