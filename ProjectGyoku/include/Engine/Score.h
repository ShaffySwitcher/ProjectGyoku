#pragma once

#include "Engine/Global.h"
#include "Engine/Math/Time.h"
#include <string>

#define SCORE_VERSION ENGINE_VERSION

#define HIGHSCORE_COUNT 10
#define DEFAULT_HIGHSCORE 1000000

#define DEFAULT_NAME "Nameless"

#define SCORE_FILE_MAGIC "GYOK"

#define PSCD_MAGIC "PSCD"
#define CLRD_MAGIC "CLRD"
#define SPCD_MAGIC "SPCD"
#define HSCD_MAGIC "HSCD"
#define GPLD_MAGIC "GPLD"
#define PSTD_MAGIC "PSTD"

enum class HighscoreState : uint8_t {
    UNINITIALIZED,
    DEFAULT,
    USER
};

#pragma pack(push, 1)

// PGSB: Project Gyoku Score Base (Common Base)
struct PGSB {
	char magic[4];
	uint8_t version;

    static bool validate(const PGSB& base, const char* expectedMagic) {
        return (memcmp(base.magic, expectedMagic, 4) == 0) && (base.version <= SCORE_VERSION);
    }

    static PGSB createBase(const char* magic) {
        PGSB base;
        memcpy_s(base.magic, 4, magic, 4);
        base.version = SCORE_VERSION;
        return base;
    }
};

// PSCD: Player Stage Clear Data
// Best score in stage for character on difficulty
// Exemple: 3,356,125 in Stage 4 for Lloyd B on Illusory
struct PSCD {
    PGSB base;
    uint32_t score;
    uint8_t character;
    uint8_t difficulty;
    uint8_t stage;
};

// CLRD: Clear Data
struct CLRD {
    PGSB base;
    uint8_t stageCleared[DIFFICULTY_COUNT]; // if no continues were used
    uint8_t stageClearedWithContinues[DIFFICULTY_COUNT];
    uint8_t character;
};

// SPCD: Spellcard Data
struct SPCD {
    PGSB base;
    uint32_t score;
    uint16_t id;
    uint16_t attempts;
    uint16_t success;
    char name[64];
};

// HSCD: Highscore Data
struct HSCD {
    PGSB base;
    char name[8];
    DateTime time;
    uint8_t character;
    uint8_t difficulty;
    uint8_t stage;
    uint32_t score;
    uint8_t state;
};

// GPLD: Gameplay Data
struct GPLD {
    PGSB base;
    uint32_t plays[CHARACTER_COUNT];
    uint32_t clears;
    uint32_t continues;
    uint32_t practices;
};

// PSTD: Play Stats Data
struct PSTD {
    PGSB base;
    DateTime timePlayed;
    DateTime timePlayedGame;
    GPLD dataDifficulty[DIFFICULTY_COUNT];
    GPLD dataTotal;
};

struct Score {
    char magic[4]; // "GYOK"
    uint8_t seed[2];
    uint16_t checksum;

    PSCD playerStageClearData[CHARACTER_COUNT][DIFFICULTY_COUNT][STAGE_COUNT];
    CLRD clearData[CHARACTER_COUNT];
    HSCD highscores[CHARACTER_COUNT][DIFFICULTY_COUNT][HIGHSCORE_COUNT];
    SPCD spellcards[SPELLCARD_COUNT];
    PSTD playStats;

    void reset();
    uint16_t calculateChecksum() const;
    uint32_t getStageHighscore(uint8_t character, uint8_t difficulty, uint8_t stage) const;
    uint32_t getHighscore(uint8_t character, uint8_t difficulty) const;
    bool hasUnlockedExtra(uint8_t character) const;
};

#pragma pack(pop)

class ScoreManager {
public:
    static void load(const std::string& path);
    static void save(const std::string& path);
    static void updatePlaytime();
    static Score* getCurrentScore();

private:
    static void crypt(uint8_t* data, size_t size, uint8_t key);

    static Score* currentScore;
};