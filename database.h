#pragma once
#include <string>

struct PlayerProgress {
    int progress_Id = 0;      //==> unique id for the saved row
    int user_Id = 0;          //==> user id owner
    int current_Level = 1;    //==> current level
    int lives_Remaining = 2;  //==> remaining lives
    int gem_Count = 0;        //==> gem count
    int high_Score = 0;       //==> high score
    char last_Updated[20] = ""; //==> timestamp string (YYYY-MM-DD HH:MM:SS)
};

//==> SaveLoadManager --> responsible for reading/writing progress file
class SaveLoadManager {
public:
    static SaveLoadManager& getInstance(); //==> singleton getter

    bool saveProgress(const PlayerProgress& p); // yh progress ko save or update krta 
    PlayerProgress loadProgress(int userId); // it loads progress for user
	bool deleteProgress(int userId); // yeh user's progress delete krta hai
    bool manualSave(int userId, int currentLevel, int livesRemaining, int gemCount, int highScore); 
    void flushGemCount(int userId, int newGemCount); //it updates gem count only

private:
    SaveLoadManager();
    SaveLoadManager(const SaveLoadManager&) = delete;
    SaveLoadManager& operator=(const SaveLoadManager&) = delete;

    void progressToLine(const PlayerProgress& p, char* outBuf, int bufSize);
    bool lineToProgress(const char* line, PlayerProgress& outP);
    int  generateProgressId();
    void getCurrentTimestamp(char* outBuf, int bufSize);

    bool rewriteProgressFile(int targetUserId,
        bool skipUser,
        bool replaceUser,
        const char* replacementLine,
        bool& targetFound);

    static const char* PROGRESS_FILE; // path to progress file
    static const char  FIELD_SEP;     //  field separator '|'
    static const int   MAX_USERS = 200; 
    static const int   MAX_LINE = 256;  // max line length buffer
};
// =============================================================
//  LEADERBOARD ENTRY
// =============================================================
struct LeaderboardEntry {
    int  entry_Id = 0;
    int  user_Id = 0;
    char username[32] = "";
    int  score = 0;
    int  level_Reached = 1;
    char recorded_At[20] = "";
};

// =============================================================
//  LeaderboardManager
//  Appends new score entries and loads top-N sorted by score.
// =============================================================
class LeaderboardManager {
public:
    static LeaderboardManager& getInstance();

    // Call this when a game session ends (level complete / game over)
    bool addEntry(int userId, const std::string& username,
        int score, int levelReached);

    // Fills outEntries[] with up to maxCount entries, sorted
    // highest score first. Returns how many were filled.
    int  loadTopN(LeaderboardEntry* outEntries, int maxCount);

private:
    LeaderboardManager();
    LeaderboardManager(const LeaderboardManager&) = delete;
    LeaderboardManager& operator=(const LeaderboardManager&) = delete;

    void entryToLine(const LeaderboardEntry& e, char* outBuf, int bufSize);
    bool lineToEntry(const char* line, LeaderboardEntry& outE);
    int  generateEntryId();
    void getCurrentTimestamp(char* outBuf, int bufSize);

    static const char* LEADERBOARD_FILE;  // "leaderboard.txt"
    static const char  FIELD_SEP;
    static const int   MAX_LINE = 256;
};