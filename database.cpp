#include "database.h" 
#include <fstream>    // for file streams
#include <sstream>    // for parsing lines
#include <cstring>    // for string operations
#include <ctime>      // for timestamp
#include <iostream>   // for error logs
#include <cstdio>     // for file operations
using namespace std;

const char* SaveLoadManager::PROGRESS_FILE = "player_progress.txt";
const char  SaveLoadManager::FIELD_SEP = '|'; //  field separator in file

SaveLoadManager::SaveLoadManager() {
}

SaveLoadManager& SaveLoadManager::getInstance() {
    static SaveLoadManager instance;
    return instance;
}

bool SaveLoadManager::rewriteProgressFile(int targetUserId,bool skipUser,bool replaceUser,const char* replacementLine,
    bool& targetFound)
{
    targetFound = false;
    const char* tmpPath = "player_progress.tmp";

    ifstream in(PROGRESS_FILE);
    ofstream out(tmpPath, ios::trunc);
    if (!out.is_open()) return false;

    if (in.is_open()) {
        string line;
        while (getline(in, line)) {
            if (line.empty() || line[0] == '#') {
                out << line << "\n";
                continue;
            }

            stringstream ss(line);
            string pidStr, uidStr;
            getline(ss, pidStr, FIELD_SEP); // read progressId
            getline(ss, uidStr, FIELD_SEP); // read userId

            int uid = std::stoi(uidStr);
            if (uid == targetUserId) {
                targetFound = true;
                if (skipUser) 
                    continue;
                if (replaceUser) {
                    out << replacementLine << "\n"; 
                    continue; 
                }
            }
            out << line << "\n";
        }
        in.close();
    }
    out.close();

    remove(PROGRESS_FILE);
    rename(tmpPath, PROGRESS_FILE);
    return true;
}

bool SaveLoadManager::saveProgress(const PlayerProgress& p) {
    char replacementLine[MAX_LINE];
    progressToLine(p, replacementLine, MAX_LINE);

    bool recordFound = false;
    bool ok = rewriteProgressFile(p.user_Id,  false, true, replacementLine,recordFound);
    if (!ok) {
        std::cerr << "[SaveLoadManager] ERROR: Cannot rewrite " << PROGRESS_FILE << "\n";
        return false;
    }

    if (!recordFound) {
        
        PlayerProgress newP = p;
        newP.progress_Id = generateProgressId();
        char newLine[MAX_LINE];
        progressToLine(newP, newLine, MAX_LINE);

        std::ofstream app(PROGRESS_FILE, std::ios::app);
        if (!app.is_open()) return false;
        app << newLine << "\n";
    }
    return true;
}


PlayerProgress SaveLoadManager::loadProgress(int userId) {
    PlayerProgress defaultP;
    defaultP.user_Id = userId;
    defaultP.current_Level = 1;
    defaultP.lives_Remaining = 2;
    defaultP.gem_Count = 0;
    defaultP.high_Score = 0;

    std::ifstream file(PROGRESS_FILE);
    if (!file.is_open()) return defaultP; //file nahi to default

    char buf[MAX_LINE];
    while (file.getline(buf, MAX_LINE)) {
        if (buf[0] == '#' || buf[0] == '\0') continue;

        PlayerProgress p;
        if (lineToProgress(buf, p) && p.user_Id == userId) {
            file.close();
            return p;
        }
    }
    file.close();
    return defaultP;
}

bool SaveLoadManager::deleteProgress(int userId) {
    bool recordFound = false;
    bool ok = rewriteProgressFile(userId,true,false,nullptr,recordFound);
    return ok && recordFound;
}


bool SaveLoadManager::manualSave(int userId, int currentLevel,
    int livesRemaining, int gemCount, int highScore)
{
    PlayerProgress p = loadProgress(userId);
    p.user_Id = userId;
    p.current_Level = currentLevel;
    p.lives_Remaining = livesRemaining;
    p.gem_Count = gemCount;
    p.high_Score = highScore;
    return saveProgress(p);
}


void SaveLoadManager::flushGemCount(int userId, int newGemCount) {
    PlayerProgress p = loadProgress(userId);
    p.gem_Count = newGemCount;
    saveProgress(p);
}

void SaveLoadManager::progressToLine(const PlayerProgress& p,char* outBuf, int bufSize)
{
    char timestamp[20];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    snprintf(outBuf, bufSize, "%d|%d|%d|%d|%d|%d|%s",
        p.progress_Id,
        p.user_Id,
        p.current_Level,
        p.lives_Remaining,
        p.gem_Count,
        p.high_Score,
        timestamp);
}

bool SaveLoadManager::lineToProgress(const char* line, PlayerProgress& outP) {
    stringstream ss(line);
    string pidStr, uidStr, lvlStr, livesStr, gemStr, scoreStr, dateStr;

    if (!getline(ss, pidStr, FIELD_SEP)) 
        return false;
    if (!getline(ss, uidStr, FIELD_SEP))
       return false;
    if (!getline(ss, lvlStr, FIELD_SEP)) 
       return false;
    if (!getline(ss, livesStr, FIELD_SEP))
        return false;
    if (!getline(ss, gemStr, FIELD_SEP)) 
        return false;
    if (!getline(ss, scoreStr, FIELD_SEP))
        return false;
    if (!getline(ss, dateStr, FIELD_SEP))
        return false;

    outP.progress_Id = std::stoi(pidStr);
    outP.user_Id = std::stoi(uidStr);
    outP.current_Level = std::stoi(lvlStr);
    outP.lives_Remaining = std::stoi(livesStr);
    outP.gem_Count = std::stoi(gemStr);
    outP.high_Score = std::stoi(scoreStr);

    strncpy_s(outP.last_Updated, sizeof(outP.last_Updated),
        dateStr.c_str(), _TRUNCATE);
    return true;
}

int SaveLoadManager::generateProgressId() {
    std::ifstream file(PROGRESS_FILE);
    if (!file.is_open()) return 1;

    int maxId = 0;
    char buf[MAX_LINE];
    while (file.getline(buf, MAX_LINE)) {
        if (buf[0] == '#' || buf[0] == '\0') 
            continue;
        stringstream ss(buf);
        string idStr;
        getline(ss, idStr, FIELD_SEP);
        int id = stoi(idStr);
        if (id > maxId) maxId = id;
    }
    file.close();
    return maxId + 1;
}

void SaveLoadManager::getCurrentTimestamp(char* outBuf, int bufSize) {
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    strftime(outBuf, bufSize, "%Y-%m-%d %H:%M:%S", &t);
}
// =============================================================
//  LeaderboardManager
// =============================================================

const char* LeaderboardManager::LEADERBOARD_FILE = "leaderboard.txt";
const char  LeaderboardManager::FIELD_SEP = '|';

LeaderboardManager::LeaderboardManager() {}

LeaderboardManager& LeaderboardManager::getInstance() {
    static LeaderboardManager instance;
    return instance;
}

bool LeaderboardManager::addEntry(int userId,
    const std::string& username, int score, int levelReached)
{
    LeaderboardEntry e;
    e.entry_Id = generateEntryId();
    e.user_Id = userId;
    e.score = score;
    e.level_Reached = levelReached;

    // copy username safely
    int i = 0;
    while (username[i] && i < 31) { e.username[i] = username[i]; ++i; }
    e.username[i] = '\0';

    getCurrentTimestamp(e.recorded_At, sizeof(e.recorded_At));

    char line[MAX_LINE];
    entryToLine(e, line, MAX_LINE);

    std::ofstream file(LEADERBOARD_FILE, std::ios::app);
    if (!file.is_open()) return false;
    file << line << "\n";
    return true;
}

int LeaderboardManager::loadTopN(LeaderboardEntry* outEntries, int maxCount)
{
    // Read ALL entries from file
    static const int BIG = 500;
    LeaderboardEntry all[BIG];
    int total = 0;

    std::ifstream file(LEADERBOARD_FILE);
    if (!file.is_open()) return 0;

    char buf[MAX_LINE];
    while (file.getline(buf, MAX_LINE) && total < BIG) {
        if (buf[0] == '#' || buf[0] == '\0') continue;
        LeaderboardEntry e;
        if (lineToEntry(buf, e))
            all[total++] = e;
    }
    file.close();

    // Simple insertion sort — highest score first
    for (int i = 1; i < total; ++i) {
        LeaderboardEntry key = all[i];
        int j = i - 1;
        while (j >= 0 && all[j].score < key.score) {
            all[j + 1] = all[j];
            --j;
        }
        all[j + 1] = key;
    }

    // Copy top maxCount into output array
    int count = (total < maxCount) ? total : maxCount;
    for (int i = 0; i < count; ++i)
        outEntries[i] = all[i];
    return count;
}

void LeaderboardManager::entryToLine(const LeaderboardEntry& e,
    char* outBuf, int bufSize)
{
    snprintf(outBuf, bufSize, "%d|%d|%s|%d|%d|%s",
        e.entry_Id,
        e.user_Id,
        e.username,
        e.score,
        e.level_Reached,
        e.recorded_At);
}

bool LeaderboardManager::lineToEntry(const char* line, LeaderboardEntry& outE)
{
    std::stringstream ss(line);
    std::string idStr, uidStr, uname, scoreStr, lvlStr, dateStr;

    if (!std::getline(ss, idStr, FIELD_SEP)) return false;
    if (!std::getline(ss, uidStr, FIELD_SEP)) return false;
    if (!std::getline(ss, uname, FIELD_SEP)) return false;
    if (!std::getline(ss, scoreStr, FIELD_SEP)) return false;
    if (!std::getline(ss, lvlStr, FIELD_SEP)) return false;
    if (!std::getline(ss, dateStr, FIELD_SEP)) return false;

    outE.entry_Id = std::stoi(idStr);
    outE.user_Id = std::stoi(uidStr);
    outE.score = std::stoi(scoreStr);
    outE.level_Reached = std::stoi(lvlStr);

    int i = 0;
    while (uname[i] && i < 31) { outE.username[i] = uname[i]; ++i; }
    outE.username[i] = '\0';

    strncpy_s(outE.recorded_At, sizeof(outE.recorded_At),
        dateStr.c_str(), _TRUNCATE);
    return true;
}

int LeaderboardManager::generateEntryId()
{
    std::ifstream file(LEADERBOARD_FILE);
    if (!file.is_open()) return 1;

    int maxId = 0;
    char buf[MAX_LINE];
    while (file.getline(buf, MAX_LINE)) {
        if (buf[0] == '#' || buf[0] == '\0') continue;
        std::stringstream ss(buf);
        std::string idStr;
        std::getline(ss, idStr, FIELD_SEP);
        int id = std::stoi(idStr);
        if (id > maxId) maxId = id;
    }
    file.close();
    return maxId + 1;
}

void LeaderboardManager::getCurrentTimestamp(char* outBuf, int bufSize)
{
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    strftime(outBuf, bufSize, "%Y-%m-%d %H:%M:%S", &t);
}