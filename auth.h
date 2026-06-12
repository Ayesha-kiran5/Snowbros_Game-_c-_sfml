#pragma once
#include"database.h"
#include <SFML/Graphics.hpp> 
#include <SFML/Audio.hpp>    
#include <string>  
using namespace std;

struct PlayerProgress;
// login screen 
// screen se niklte waqt k results
enum class LoginScreenResult {
    LOGGED_IN,
    REGISTERED,
    EXIT
};

// knsa tab active hai --->  login ya rgster
enum class LoginScreenMode {
    LOGIN,
    REGISTER
};

class LoginScreen {
public:
    explicit LoginScreen(sf::RenderWindow& window);
    LoginScreenResult run();

private:
    // --- window ---
    sf::RenderWindow& m_window;

    // --- full-screen background ---
    sf::Texture m_bgTexture; //  background image texture
    sf::Sprite  m_bgSprite;  // sprite to draw the texture

    // --- panel background texture ---
    sf::Texture m_panelBgTexture;
    sf::Sprite  m_panelBgSprite;
    bool  m_panelBgLoaded;

    // Panel rounded inner shape + corner circles for curved edges
    sf::RectangleShape m_panelInner;
    sf::CircleShape m_panelCorner[4];
    sf::RectangleShape m_panelShine;
    sf::VertexArray m_panelGradient; // quads, vertical gradient

    // --- logo image  ---
    sf::Texture m_logoTexture;
    sf::Sprite m_logoSprite;
    bool m_logoLoaded;
    sf::Text m_logoFallbackText;   //agar logo missing ho to dikhane wala text

    // --- audio / font ---
    sf::Music m_music; // background music object
    sf::Font m_font;  //font for all UI text

    // --- panel border ---
    sf::RectangleShape m_panel; // outer panel rectangle (border)

    // --- logo slot bar (dark strip behind logo) ---
    sf::RectangleShape m_logoSlot;

    // --- tabs ---
    sf::RectangleShape m_loginTab;    //left tab rectangle
    sf::RectangleShape m_registerTab; // right tab rectangle
    sf::Text m_loginTabText;    // "LOGIN" label
    sf::Text m_registerTabText; // REGISTER" label
    sf::RectangleShape m_tabUnderline;   // underline under active tab

    // --- input fields (0=user, 1=pass, 2=email) ---
    static const int   FIELD_COUNT = 3; //  username,password,email
    sf::RectangleShape m_fieldBox[FIELD_COUNT]; //  input box shapes
    sf::Text  m_fieldLabel[FIELD_COUNT]; //  labels above boxes
    sf::Text  m_fieldText[FIELD_COUNT]; // displayed text ===> masked for pass
    std::string m_inputUsername; // store input username
    std::string  m_inputPassword; // store input password (plain in memory)
    std::string  m_inputEmail;    //optional email
    int  m_focusedField; 

    // --- blinking cursor ---
    sf::RectangleShape m_cursor; // ===> tiny rect as caret
    bool  m_cursorVisible; //---> visible/invisible for blink
    sf::Clock m_cursorClock; // -->  timing for blink

    // --- action button ==> square +++ hover glow ---
    sf::RectangleShape m_actionButton; // ==> main button rect
    sf::RectangleShape m_buttonGlow;   // ==> glow rectangle drawn when hovered
    sf::Text m_actionButtonText; // ==> button label text
    bool m_buttonHovered; // ==> hover state

    // --- hint + status ---
    sf::Text  m_switchHintText; // ==> small hint to switch tabs
    sf::Text m_statusText; // ==> error/success messages
    sf::Color m_statusColor; // ==> color for status text (red/green)

    // --- state ---
    LoginScreenMode    m_mode;
    void loadBackground();
    void loadPanelBackground();
    void loadLogo();
    void loadmusic_();
    void loadFont();
    void buildUI();
    void refreshMode();

    bool handleEvent(const sf::Event& ev, LoginScreenResult& out);
    void handleTextInput(sf::Uint32 unicode);
    void handleKeyPress(sf::Keyboard::Key key, LoginScreenResult& out);
    void handleMouseClick(sf::Vector2f mp, LoginScreenResult& out);
    void handleMouseMove(sf::Vector2f mp);
    void submitForm(LoginScreenResult& out);
    void syncFieldTexts();
    void updateCursor(float dt);
    void render();

    string* fieldString(int idx);
    void centreTextX(sf::Text& t, float left, float right);
    void setStatus(const std::string& msg, bool isError);

    // --- layout constants ---
    static const float WINDOW_W;  // 600
    static const float WINDOW_H;  // 600
    static const float PANEL_W;   // 364
    static const float PANEL_H;   // 460
    static const float PANEL_X;
    static const float PANEL_Y;
    static const float LOGO_H;    // 110
    static const float TAB_H;     // 36
    static const float FIELD_H;   // 38
    static const float BTN_W;     // 144
    static const float BTN_H;     // 44
};

// =============================================================================
//   PASSWORD HASHER
// =============================================================================

class PasswordHasher {
public:
    static unsigned long hash(const std::string& password);
    static bool verify(const std::string& password, unsigned long storedHash);
};

// =============================================================================
//   AUTH MANAGER
// =============================================================================

enum class LoginResult { SUCCESS, BAD_PASSWORD, USER_NOT_FOUND, FILE_ERROR };
enum class RegisterResult { SUCCESS, USERNAME_TAKEN, INVALID_INPUT, FILE_ERROR };

struct UserRecord {
    int userId;
    std::string username;
    unsigned long passwordHash;
    std::string email;
    std::string createdAt;
};

class AuthManager {
public:
    static AuthManager& getInstance();

    RegisterResult registerUser(const std::string& username,
        const std::string& password,
        const std::string& email = "");

    LoginResult    loginUser(const std::string& username,
        const std::string& password);

    static string loginResultToString(LoginResult result);
    static string registerResultToString(RegisterResult result);

private:
    AuthManager();
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;

    bool usernameExists(const std::string& username);
    UserRecord  findUser(const std::string& username, bool& found);
    int  generateUserId();
    string getCurrentDate();

    static const char* USERS_FILE;   // "users.txt"
    static const char  FIELD_SEP;    
};

// =============================================================================
//  SESSION MANAGER
// =============================================================================

class SessionManager {
public:
    static SessionManager& getInstance();
    void setCurrentUser(int id, const std::string& username);
    bool isLoggedIn() const;
    int getCurrentUserId() const;
    std::string getCurrentUsername() const;

    void setLoadedProgress(const PlayerProgress& p);
    PlayerProgress getLoadedProgress() const;
    void logout();

private:
    SessionManager();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    bool           m_loggedIn;
    int            m_userId;
    std::string    m_username;
    PlayerProgress m_loadedProgress;
};
