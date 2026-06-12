#include "auth.h" 
#include "database.h" 
#include <fstream> 
#include <iostream> 
#include <ctime> 
#include <cstring> 
#include <sstream>


// Layout constants ---> screen aur panel ki sizing hai 
const float LoginScreen::WINDOW_W = 600.f;
const float LoginScreen::WINDOW_H = 600.f;
const float LoginScreen::PANEL_W = 364.f;
const float LoginScreen::PANEL_H = 500.f;
const float LoginScreen::PANEL_X = (WINDOW_W - PANEL_W) / 2.f;
const float LoginScreen::PANEL_Y = (WINDOW_H - PANEL_H) / 2.f;
const float LoginScreen::LOGO_H = 150.f;
const float LoginScreen::TAB_H = 36.f;
const float LoginScreen::FIELD_H = 38.f;
const float LoginScreen::BTN_W = 144.f;
const float LoginScreen::BTN_H = 44.f;

//  static members for AuthManager
const char* AuthManager::USERS_FILE = "users.txt";
const char  AuthManager::FIELD_SEP = '|'; 

// Constructor
LoginScreen::LoginScreen(sf::RenderWindow& window)
    : m_window(window), m_panelBgLoaded(false), m_logoLoaded(false),
    m_mode(LoginScreenMode::LOGIN), m_focusedField(0),
    m_cursorVisible(true), m_buttonHovered(false),
    m_statusColor(sf::Color::Red)
{
    loadFont();
    loadBackground();
    loadPanelBackground();
    loadLogo();
    loadmusic_();
    buildUI();
    refreshMode();
}

LoginScreenResult LoginScreen::run() {
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);

    sf::Clock frameClock;
    LoginScreenResult result = LoginScreenResult::EXIT;

    while (m_window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        bool shouldExit = false;

        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
                return LoginScreenResult::EXIT;
            }
            if (handleEvent(event, result))
                shouldExit = true;
        }

        updateCursor(dt);
        render();

        if (shouldExit) return result;
    }
    return LoginScreenResult::EXIT;
}

void LoginScreen::loadBackground() {
    if (!m_bgTexture.loadFromFile("bg.png")) {
        std::cerr << "[LoginScreen] WARNING: login_bg.png not found."
            " Using solid colour fallback.\n";
        sf::Image img;
        img.create(1, 1, sf::Color(6, 14, 34));
        m_bgTexture.loadFromImage(img);
    }
    m_bgSprite.setTexture(m_bgTexture);
    sf::Vector2u sz = m_bgTexture.getSize();
    if (sz.x > 0 && sz.y > 0)
        m_bgSprite.setScale(WINDOW_W / (float)sz.x, WINDOW_H / (float)sz.y);
}

void LoginScreen::loadPanelBackground() {
    m_panelBgLoaded = false;

    float innerPad = 6.f;
    float innerW = PANEL_W - innerPad * 2.f;
    float innerH = PANEL_H - innerPad * 2.f;
    m_panelInner.setSize(sf::Vector2f(innerW, innerH));
    m_panelInner.setPosition(PANEL_X + innerPad, PANEL_Y + innerPad);
    m_panelInner.setOutlineColor(sf::Color(120, 200, 255, 60));
    m_panelInner.setOutlineThickness(1.f);

    m_panelGradient = sf::VertexArray(sf::Quads, 4);
    sf::Vector2f p0(PANEL_X + innerPad, PANEL_Y + innerPad);
    sf::Vector2f p1(PANEL_X + innerPad + innerW, PANEL_Y + innerPad);
    sf::Vector2f p2(PANEL_X + innerPad + innerW, PANEL_Y + innerPad + innerH);
    sf::Vector2f p3(PANEL_X + innerPad, PANEL_Y + innerPad + innerH);
    sf::Color topCol(18, 40, 70, 230);
    sf::Color botCol(6, 14, 28, 230);
    m_panelGradient[0].position = p0; m_panelGradient[0].color = topCol;
    m_panelGradient[1].position = p1; m_panelGradient[1].color = topCol;
    m_panelGradient[2].position = p2; m_panelGradient[2].color = botCol;
    m_panelGradient[3].position = p3; m_panelGradient[3].color = botCol;

    float cr = innerPad + 6.f;
    for (int i = 0; i < 4; ++i) {
        m_panelCorner[i].setRadius(cr);
        m_panelCorner[i].setPointCount(20);
        m_panelCorner[i].setFillColor(sf::Color(10, 20, 40, 230));
        m_panelCorner[i].setOutlineColor(sf::Color(140, 220, 255, 80));
        m_panelCorner[i].setOutlineThickness(0.6f);
    }
    m_panelCorner[0].setPosition(PANEL_X + innerPad - cr, PANEL_Y + innerPad - cr);
    m_panelCorner[1].setPosition(PANEL_X + innerPad + innerW - cr + cr * 2 - cr, PANEL_Y + innerPad - cr);
    m_panelCorner[2].setPosition(PANEL_X + innerPad - cr, PANEL_Y + innerPad + innerH - cr + cr * 2 - cr);
    m_panelCorner[3].setPosition(PANEL_X + innerPad + innerW - cr + cr * 2 - cr, PANEL_Y + innerPad + innerH - cr + cr * 2 - cr);

    m_panelShine.setSize(sf::Vector2f(innerW - 10.f, 6.f));
    m_panelShine.setPosition(PANEL_X + innerPad + 5.f, PANEL_Y + innerPad + 6.f);
    m_panelShine.setFillColor(sf::Color(200, 245, 255, 40));
}

void LoginScreen::loadLogo() {
    if (!m_logoTexture.loadFromFile("logo.png")) {
        std::cerr << "[LoginScreen] WARNING: logo.png not found."
            " Showing fallback placeholder.\n";
        m_logoLoaded = false;
        return;
    }
    m_logoLoaded = true;
    m_logoSprite.setTexture(m_logoTexture);

    float maxW = PANEL_W - 40.f;
    float maxH = LOGO_H - 20.f;
    sf::Vector2u sz = m_logoTexture.getSize();
    float scaleX = maxW / (float)sz.x;
    float scaleY = maxH / (float)sz.y;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    m_logoSprite.setScale(scale, scale);

    float logoW = sz.x * scale;
    float logoH = sz.y * scale;
    float logoX = PANEL_X + (PANEL_W - logoW) / 2.f;
    float logoY = PANEL_Y + (LOGO_H - logoH) / 2.f;
    m_logoSprite.setPosition(logoX, logoY);
}

void LoginScreen::loadmusic_() {
    if (!m_music.openFromFile("snow_bros_theme_01.ogg")) {
        std::cerr << "[LoginScreen] WARNING: login_music.ogg not found."
            " Running without music.\n";
        return;
    }
    m_music.setLoop(true);
    m_music.setVolume(60.f);
    m_music.play();
}

void LoginScreen::loadFont() {
    if (!m_font.loadFromFile("arial.ttf"))
        std::cerr << "[LoginScreen] WARNING: No font loaded. Text may not render.\n";
}

void LoginScreen::buildUI() {
    m_panel.setSize(sf::Vector2f(PANEL_W, PANEL_H));
    m_panel.setPosition(PANEL_X, PANEL_Y);
    if (!m_panelBgLoaded)
        m_panel.setFillColor(sf::Color(8, 14, 40, 215));
    else
        m_panel.setFillColor(sf::Color::Transparent);
    m_panel.setOutlineColor(sf::Color(80, 160, 255, 160));
    m_panel.setOutlineThickness(1.5f);

    m_logoSlot.setSize(sf::Vector2f(PANEL_W, LOGO_H));
    m_logoSlot.setPosition(PANEL_X, PANEL_Y);
    m_logoSlot.setFillColor(sf::Color(5, 10, 30, 200));

    m_logoFallbackText.setFont(m_font);
    m_logoFallbackText.setString("[ logo.png ]");
    m_logoFallbackText.setCharacterSize(14);
    m_logoFallbackText.setFillColor(sf::Color(80, 130, 180, 160));
    centreTextX(m_logoFallbackText, PANEL_X, PANEL_X + PANEL_W);
    m_logoFallbackText.setPosition(
        m_logoFallbackText.getPosition().x,
        PANEL_Y + (LOGO_H - 14.f) / 2.f
    );

    float tabW = PANEL_W / 2.f;
    float tabTopY = PANEL_Y + LOGO_H;

    m_loginTab.setSize(sf::Vector2f(tabW, TAB_H));
    m_loginTab.setPosition(PANEL_X, tabTopY);

    m_registerTab.setSize(sf::Vector2f(tabW, TAB_H));
    m_registerTab.setPosition(PANEL_X + tabW, tabTopY);

    auto makeTabText = [&](sf::Text& t, const char* s, float tabX) {
        t.setFont(m_font);
        t.setString(s);
        t.setCharacterSize(13);
        t.setStyle(sf::Text::Bold);
        centreTextX(t, tabX, tabX + tabW);
        t.setPosition(t.getPosition().x, tabTopY + 10.f);
        };
    makeTabText(m_loginTabText, "LOGIN", PANEL_X);
    makeTabText(m_registerTabText, "REGISTER", PANEL_X + tabW);

    m_tabUnderline.setSize(sf::Vector2f(tabW, 2.f));
    m_tabUnderline.setFillColor(sf::Color(100, 210, 255));

    float fieldX = PANEL_X + 26.f;
    float fieldW = PANEL_W - 52.f;
    float startY = tabTopY + TAB_H + 26.f;
    float fieldGap = 66.f;

    const char* labels[FIELD_COUNT] = {
        "USERNAME",
        "PASSWORD",
        "EMAIL  (optional)"
    };

    for (int i = 0; i < FIELD_COUNT; i++) {
        float fy = startY + i * fieldGap;

        m_fieldLabel[i].setFont(m_font);
        m_fieldLabel[i].setString(labels[i]);
        m_fieldLabel[i].setCharacterSize(11);
        m_fieldLabel[i].setFillColor(sf::Color(120, 170, 210));
        m_fieldLabel[i].setPosition(fieldX, fy - 17.f);

        m_fieldBox[i].setSize(sf::Vector2f(fieldW, FIELD_H));
        m_fieldBox[i].setPosition(fieldX, fy);
        m_fieldBox[i].setFillColor(sf::Color(10, 18, 52, 230));
        m_fieldBox[i].setOutlineThickness(1.5f);
        m_fieldBox[i].setOutlineColor(sf::Color(50, 110, 190));

        m_fieldText[i].setFont(m_font);
        m_fieldText[i].setCharacterSize(14);
        m_fieldText[i].setFillColor(sf::Color(220, 238, 255));
        m_fieldText[i].setPosition(fieldX + 10.f, fy + 10.f);
    }

    m_cursor.setSize(sf::Vector2f(2.f, 20.f));
    m_cursor.setFillColor(sf::Color(100, 200, 255));

    float btnX = PANEL_X + (PANEL_W - BTN_W) / 2.f;
    float btnY = startY + FIELD_COUNT * fieldGap + 8.f;

    m_buttonGlow.setSize(sf::Vector2f(BTN_W + 16.f, BTN_H + 16.f));
    m_buttonGlow.setPosition(btnX - 8.f, btnY - 8.f);
    m_buttonGlow.setFillColor(sf::Color(60, 140, 255, 55));

    m_actionButton.setSize(sf::Vector2f(BTN_W, BTN_H));
    m_actionButton.setPosition(btnX, btnY);
    m_actionButton.setFillColor(sf::Color(22, 88, 190));
    m_actionButton.setOutlineColor(sf::Color(80, 160, 255, 140));
    m_actionButton.setOutlineThickness(1.5f);

    m_actionButtonText.setFont(m_font);
    m_actionButtonText.setCharacterSize(14);
    m_actionButtonText.setStyle(sf::Text::Bold);
    m_actionButtonText.setFillColor(sf::Color::White);

    m_statusText.setFont(m_font);
    m_statusText.setCharacterSize(12);
    m_statusText.setPosition(fieldX, btnY - 20.f);
    m_statusText.setString("");

    m_switchHintText.setFont(m_font);
    m_switchHintText.setCharacterSize(12);
    m_switchHintText.setFillColor(sf::Color(100, 145, 185));
    m_switchHintText.setPosition(0.f, btnY + BTN_H + 12.f);
}

void LoginScreen::refreshMode() {
    bool isLogin = (m_mode == LoginScreenMode::LOGIN);

    sf::Color activeTabFill(25, 70, 165, 200);
    sf::Color inactiveTabFill(8, 14, 40, 160);

    m_loginTab.setFillColor(isLogin ? activeTabFill : inactiveTabFill);
    m_registerTab.setFillColor(isLogin ? inactiveTabFill : activeTabFill);
    m_loginTabText.setFillColor(isLogin ? sf::Color(255, 255, 255) : sf::Color(100, 145, 180));
    m_registerTabText.setFillColor(isLogin ? sf::Color(100, 145, 180) : sf::Color(255, 255, 255));

    float tabW = PANEL_W / 2.f;
    float tabTopY = PANEL_Y + LOGO_H;
    m_tabUnderline.setPosition(
        isLogin ? PANEL_X : PANEL_X + tabW,
        tabTopY + TAB_H - 2.f
    );

    m_actionButtonText.setString(isLogin ? "LOGIN" : "REGISTER");
    centreTextX(m_actionButtonText,
        m_actionButton.getPosition().x,
        m_actionButton.getPosition().x + BTN_W);
    m_actionButtonText.setPosition(
        m_actionButtonText.getPosition().x,
        m_actionButton.getPosition().y + (BTN_H - 14.f) / 2.f - 2.f
    );

    std::string hint = isLogin
        ? "No account?  Click REGISTER above."
        : "Have an account?  Click LOGIN above.";
    m_switchHintText.setString(hint);
    centreTextX(m_switchHintText, PANEL_X, PANEL_X + PANEL_W);
    m_switchHintText.setPosition(
        m_switchHintText.getPosition().x,
        m_switchHintText.getPosition().y
    );

    m_inputUsername.clear();
    m_inputPassword.clear();
    m_inputEmail.clear();
    m_focusedField = 0;
    setStatus("", false);
    syncFieldTexts();
}

bool LoginScreen::handleEvent(const sf::Event& ev, LoginScreenResult& out) {
    if (ev.type == sf::Event::TextEntered) {
        handleTextInput(ev.text.unicode);
        return false;
    }
    if (ev.type == sf::Event::KeyPressed) {
        handleKeyPress(ev.key.code, out);
        if (out == LoginScreenResult::LOGGED_IN ||
            out == LoginScreenResult::REGISTERED) return true;
        return false;
    }
    if (ev.type == sf::Event::MouseButtonPressed &&
        ev.mouseButton.button == sf::Mouse::Left) {
        handleMouseClick(sf::Vector2f((float)ev.mouseButton.x,
            (float)ev.mouseButton.y), out);
        if (out == LoginScreenResult::LOGGED_IN ||
            out == LoginScreenResult::REGISTERED) return true;
        return false;
    }
    if (ev.type == sf::Event::MouseMoved)
        handleMouseMove(sf::Vector2f((float)ev.mouseMove.x,
            (float)ev.mouseMove.y));
    return false;
}

void LoginScreen::handleTextInput(sf::Uint32 unicode) {
    if (unicode < 32 || unicode == 127) return;
    std::string* target = fieldString(m_focusedField);
    if (!target) return;
    int cap = (m_focusedField == 1) ? 32 : 24;
    if ((int)target->size() >= cap) return;
    if (unicode <= 126)
        target->push_back((char)unicode);
    syncFieldTexts();
}

void LoginScreen::handleKeyPress(sf::Keyboard::Key key, LoginScreenResult& out) {
    if (key == sf::Keyboard::Tab) {
        int maxField = (m_mode == LoginScreenMode::REGISTER) ? FIELD_COUNT - 1 : 1;
        m_focusedField = (m_focusedField + 1) % (maxField + 1);
        return;
    }
    if (key == sf::Keyboard::Enter) { submitForm(out); return; }
    if (key == sf::Keyboard::BackSpace) {
        std::string* t = fieldString(m_focusedField);
        if (t && !t->empty()) t->pop_back();
        syncFieldTexts();
        return;
    }
    if (key == sf::Keyboard::Escape)
        setStatus("", false);
}

void LoginScreen::handleMouseClick(sf::Vector2f mp, LoginScreenResult& out) {
    if (m_loginTab.getGlobalBounds().contains(mp) ||
        m_loginTabText.getGlobalBounds().contains(mp)) {
        if (m_mode != LoginScreenMode::LOGIN) {
            m_mode = LoginScreenMode::LOGIN;
            refreshMode();
        }
        return;
    }
    if (m_registerTab.getGlobalBounds().contains(mp) ||
        m_registerTabText.getGlobalBounds().contains(mp)) {
        if (m_mode != LoginScreenMode::REGISTER) {
            m_mode = LoginScreenMode::REGISTER;
            refreshMode();
        }
        return;
    }
    if (m_switchHintText.getGlobalBounds().contains(mp)) {
        m_mode = (m_mode == LoginScreenMode::LOGIN)
            ? LoginScreenMode::REGISTER : LoginScreenMode::LOGIN;
        refreshMode();
        return;
    }
    int maxField = (m_mode == LoginScreenMode::REGISTER) ? FIELD_COUNT - 1 : 1;
    for (int i = 0; i <= maxField; i++) {
        if (m_fieldBox[i].getGlobalBounds().contains(mp)) {
            m_focusedField = i;
            return;
        }
    }
    if (m_actionButton.getGlobalBounds().contains(mp) ||
        m_actionButtonText.getGlobalBounds().contains(mp))
        submitForm(out);
}

void LoginScreen::handleMouseMove(sf::Vector2f mp) {
    m_buttonHovered = m_actionButton.getGlobalBounds().contains(mp);
}

void LoginScreen::submitForm(LoginScreenResult& out) {
    if (m_mode == LoginScreenMode::LOGIN) {
        if (m_inputUsername.empty() || m_inputPassword.empty()) {
            setStatus("Please fill in username and password.", true);
            return;
        }
        LoginResult res = AuthManager::getInstance()
            .loginUser(m_inputUsername, m_inputPassword);
        if (res == LoginResult::SUCCESS) {
            m_music.stop();
            setStatus("Login successful!  Loading...", false);
            render();
            out = LoginScreenResult::LOGGED_IN;
        }
        else {
            setStatus(AuthManager::loginResultToString(res), true);
        }
    }
    else {
        if (m_inputUsername.empty() || m_inputPassword.empty()) {
            setStatus("Username and password are required.", true);
            return;
        }
        RegisterResult res = AuthManager::getInstance()
            .registerUser(m_inputUsername, m_inputPassword, m_inputEmail);
        if (res == RegisterResult::SUCCESS) {
            AuthManager::getInstance().loginUser(m_inputUsername, m_inputPassword);
            m_music.stop();
            setStatus("Account created!  Welcome, " + m_inputUsername + "!", false);
            render();
            out = LoginScreenResult::REGISTERED;
        }
        else {
            setStatus(AuthManager::registerResultToString(res), true);
        }
    }
}

void LoginScreen::syncFieldTexts() {
    m_fieldText[0].setString(m_inputUsername);
    std::string masked(m_inputPassword.size(), '*');
    m_fieldText[1].setString(masked);
    m_fieldText[2].setString(m_inputEmail);
}

void LoginScreen::updateCursor(float dt) {
    if (m_cursorClock.getElapsedTime().asSeconds() >= 0.5f) {
        m_cursorVisible = !m_cursorVisible;
        m_cursorClock.restart();
    }
    const sf::Text& ft = m_fieldText[m_focusedField];
    float cx = ft.getPosition().x + ft.getLocalBounds().width + 2.f;
    float cy = m_fieldBox[m_focusedField].getPosition().y + 9.f;
    m_cursor.setPosition(cx, cy);
}

void LoginScreen::render() {
    m_window.clear(sf::Color(6, 10, 28));
    m_window.draw(m_bgSprite);

    if (m_panelBgLoaded)
        m_window.draw(m_panelBgSprite);

    m_window.draw(m_panel);

    if (!m_panelBgLoaded) {
        m_window.draw(m_panelGradient);
        for (int i = 0; i < 4; ++i) m_window.draw(m_panelCorner[i]);
        m_window.draw(m_panelShine);
    }

    m_window.draw(m_logoSlot);
    if (m_logoLoaded)
        m_window.draw(m_logoSprite);
    else
        m_window.draw(m_logoFallbackText);

    m_window.draw(m_loginTab);
    m_window.draw(m_registerTab);
    m_window.draw(m_loginTabText);
    m_window.draw(m_registerTabText);
    m_window.draw(m_tabUnderline);

    int maxField = (m_mode == LoginScreenMode::REGISTER) ? FIELD_COUNT - 1 : 1;
    for (int i = 0; i <= maxField; i++) {
        if (i == m_focusedField) {
            m_fieldBox[i].setOutlineColor(sf::Color(100, 190, 255));
            m_fieldBox[i].setOutlineThickness(2.f);
        }
        else {
            m_fieldBox[i].setOutlineColor(sf::Color(45, 100, 175));
            m_fieldBox[i].setOutlineThickness(1.f);
        }
        m_window.draw(m_fieldLabel[i]);
        m_window.draw(m_fieldBox[i]);
        m_window.draw(m_fieldText[i]);
    }

    if (m_cursorVisible)
        m_window.draw(m_cursor);

    m_statusText.setFillColor(m_statusColor);
    m_window.draw(m_statusText);

    if (m_buttonHovered) {
        m_window.draw(m_buttonGlow);
        m_actionButton.setFillColor(sf::Color(40, 120, 240));
        m_actionButton.setOutlineColor(sf::Color(140, 200, 255, 200));
    }
    else {
        m_actionButton.setFillColor(sf::Color(22, 88, 190));
        m_actionButton.setOutlineColor(sf::Color(80, 160, 255, 140));
    }
    m_window.draw(m_actionButton);
    m_window.draw(m_actionButtonText);
    m_window.draw(m_switchHintText);
    m_window.display();
}

std::string* LoginScreen::fieldString(int idx) {
    if (idx == 0) return &m_inputUsername;
    if (idx == 1) return &m_inputPassword;
    if (idx == 2) return &m_inputEmail;
    return nullptr;
}

void LoginScreen::centreTextX(sf::Text& text, float left, float right) {
    sf::FloatRect b = text.getLocalBounds();
    float x = left + (right - left - b.width) / 2.f - b.left;
    text.setPosition(x, text.getPosition().y);
}

void LoginScreen::setStatus(const std::string& msg, bool isError) {
    m_statusText.setString(msg);
    m_statusColor = isError ? sf::Color(255, 90, 90) : sf::Color(90, 220, 140);
}

// =============================================================================
//  AuthManager
// =============================================================================

AuthManager::AuthManager() {
}

AuthManager& AuthManager::getInstance() {
    static AuthManager instance;
    return instance;
}

RegisterResult AuthManager::registerUser(const std::string& username,
    const std::string& password,
    const std::string& email)
{
    if (username.empty() || password.empty())
        return RegisterResult::INVALID_INPUT;

    if (usernameExists(username))
        return RegisterResult::USERNAME_TAKEN;

    unsigned long hashVal = PasswordHasher::hash(password);
    int newId = generateUserId();

    std::ofstream file(USERS_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[AuthManager] ERROR: Cannot open " << USERS_FILE << "\n";
        return RegisterResult::FILE_ERROR;
    }

    file << newId << FIELD_SEP
        << username << FIELD_SEP
        << hashVal << FIELD_SEP
        << email << FIELD_SEP
        << getCurrentDate() << "\n";

    file.close();
    return RegisterResult::SUCCESS;
}

LoginResult AuthManager::loginUser(const std::string& username,
    const std::string& password)
{
    bool found = false;
    UserRecord user = findUser(username, found);

    if (!found)
        return LoginResult::USER_NOT_FOUND;

    if (!PasswordHasher::verify(password, user.passwordHash))
        return LoginResult::BAD_PASSWORD;

    SessionManager::getInstance().setCurrentUser(user.userId, user.username);

    PlayerProgress progress =
        SaveLoadManager::getInstance().loadProgress(user.userId);
    SessionManager::getInstance().setLoadedProgress(progress);

    return LoginResult::SUCCESS;
}

std::string AuthManager::loginResultToString(LoginResult result) {
    switch (result) {
    case LoginResult::SUCCESS:        return "";
    case LoginResult::BAD_PASSWORD:   return "Incorrect password. Please try again.";
    case LoginResult::USER_NOT_FOUND: return "Username not found. Please register first.";
    case LoginResult::FILE_ERROR:     return "Database error. Please contact support.";
    default:                          return "Unknown error.";
    }
}

std::string AuthManager::registerResultToString(RegisterResult result) {
    switch (result) {
    case RegisterResult::SUCCESS:        return "";
    case RegisterResult::USERNAME_TAKEN: return "Username already taken. Choose another.";
    case RegisterResult::INVALID_INPUT:  return "Username and password cannot be empty.";
    case RegisterResult::FILE_ERROR:     return "Database error. Could not save account.";
    default:                             return "Unknown error.";
    }
}

bool AuthManager::usernameExists(const std::string& username) {
    std::ifstream file(USERS_FILE);
    if (!file.is_open()) return false; // RU: file nahi to no user exists yet

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string field;
        std::getline(ss, field, FIELD_SEP); // user_id
        std::getline(ss, field, FIELD_SEP); // username

        if (field == username) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

UserRecord AuthManager::findUser(const std::string& username, bool& found) {
    UserRecord record;
    found = false;

    std::ifstream file(USERS_FILE);
    if (!file.is_open()) return record;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string idStr, uname, hashStr, email, date;

        std::getline(ss, idStr, FIELD_SEP);
        std::getline(ss, uname, FIELD_SEP);
        std::getline(ss, hashStr, FIELD_SEP);
        std::getline(ss, email, FIELD_SEP);
        std::getline(ss, date, FIELD_SEP);

        if (uname == username) {
            record.userId = std::stoi(idStr);
            record.username = uname;
            record.passwordHash = std::stoul(hashStr);
            record.email = email;
            record.createdAt = date;
            found = true;
            break;
        }
    }
    file.close();
    return record;
}

int AuthManager::generateUserId() {
    std::ifstream file(USERS_FILE);
    if (!file.is_open()) return 1; // RU: pehli baar — id 1 se shuru

    int maxId = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string idStr;
        std::getline(ss, idStr, FIELD_SEP);

        int id = std::stoi(idStr);
        if (id > maxId) maxId = id;
    }
    file.close();
    return maxId + 1;
}

std::string AuthManager::getCurrentDate() {
    time_t now = time(nullptr);
    tm t{};
    localtime_s(&t, &now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return std::string(buf);
}

// =============================================================================
//  PasswordHasher
// =============================================================================

unsigned long PasswordHasher::hash(const std::string& password) {
    unsigned long hashVal = 5381;
    for (char c : password)
        hashVal = ((hashVal << 5) + hashVal) + (unsigned char)c;
    return hashVal;
}

bool PasswordHasher::verify(const std::string& password,
    unsigned long storedHash)
{
    return hash(password) == storedHash;
}

// =============================================================================
//  SessionManager
// =============================================================================

SessionManager::SessionManager()
    : m_loggedIn(false), m_userId(-1), m_username(""), m_loadedProgress()
{
}

SessionManager& SessionManager::getInstance() {
    static SessionManager instance;
    return instance;
}

void SessionManager::setCurrentUser(int id, const std::string& username) {
    m_userId = id;
    m_username = username;
    m_loggedIn = true;
}

void SessionManager::setLoadedProgress(const PlayerProgress& p) {
    m_loadedProgress = p;
}

PlayerProgress SessionManager::getLoadedProgress() const {
    return m_loadedProgress;
}

bool SessionManager::isLoggedIn() const {
    return m_loggedIn;
}

int SessionManager::getCurrentUserId() const {
    return m_loggedIn ? m_userId : -1;
}

std::string SessionManager::getCurrentUsername() const {
    return m_loggedIn ? m_username : "";
}

void SessionManager::logout() {
    m_loggedIn = false;
    m_userId = -1;
    m_username = "";
    m_loadedProgress = PlayerProgress();
}
