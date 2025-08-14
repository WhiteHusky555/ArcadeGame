#include "GameView.h"
#include "../model/GameModel.h"
#include "../model/Tank.h"
#include "../model/Bullet.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <memory>
#include "../model/TileType.h"

GameView::GameView() : gameModel(nullptr), gameLoopRunning(false), showResults(false), 
                       playerFinalScore(0), resultsDisplayTime(0.0) {
    // Конструктор создает только базовые объекты
}

GameView::~GameView() {
    // Останавливаем все таймеры и игровой цикл
    stopGame();
    stopResultsTimer();
    
    // Очищаем callback'и
    keyPressCallbackFunc = nullptr;
    gameOverCallbackFunc = nullptr;
    
    // Скрываем окно
    if (window) {
        window->hide();
    }
}

void GameView::setupUI() {
    // Размеры окна будут установлены после загрузки модели
    window = std::make_unique<GameWindow>(1200, 900, "Tanks Game");
    
    // Настраиваем обработку событий
    static_cast<GameWindow*>(window.get())->setView(this);
}

void GameView::show() {
    if (window) {
        window->show();
    }
}

void GameView::hide() {
    stopGame();
    stopResultsTimer();
    if (window) {
        window->hide();
    }
}

void GameView::scheduleCallback(CallbackFunc callback) {
    if (callback) {
        Fl::add_timeout(0.01, scheduledCallbackHandler, new CallbackFunc(callback));
    }
}

void GameView::setModel(GameModel* model) {
    gameModel = model;
    
    if (gameModel && window) {
        // Подгоняем размер окна под карту
        const GameMap& map = gameModel->getMap();
        int gamePixelWidth = static_cast<int>(map.getWidth() * GameModel::TILE_SIZE);
        int gamePixelHeight = static_cast<int>(map.getHeight() * GameModel::TILE_SIZE);
        
        int newWidth = gamePixelWidth;
        int newHeight = gamePixelHeight + HUD_AREA_HEIGHT;
        
        // Устанавливаем минимальные размеры окна
        int minWidth = std::max(newWidth, 800);
        int minHeight = std::max(newHeight, 600);
        
        window->resize(window->x(), window->y(), minWidth, minHeight);
        window->size_range(minWidth, minHeight, minWidth, minHeight);
    }
}

void GameView::startGame() {
    if (!gameLoopRunning && gameModel) {
        gameLoopRunning = true;
        showResults = false;
        playerFinalScore = 0;
        resultsDisplayTime = 0.0;
        Fl::add_timeout(0.016, gameLoopCallback, this);
    }
}

void GameView::stopGame() {
    if (gameLoopRunning) {
        gameLoopRunning = false;
        Fl::remove_timeout(gameLoopCallback, this);
    }
}

void GameView::stopResultsTimer() {
    if (showResults) {
        Fl::remove_timeout(resultsCallback, this);
        showResults = false;
    }
}

void GameView::setKeyPressCallback(KeyCallbackFunc cb) {
    keyPressCallbackFunc = std::move(cb);
}

void GameView::setGameOverCallback(CallbackFunc cb) {
    gameOverCallbackFunc = std::move(cb);
}

bool GameView::handleKeyPress(int key) {
    if (keyPressCallbackFunc) {
        return keyPressCallbackFunc(key);
    }
    return false;
}

void GameView::draw() {
    if (!gameModel) return;
    
    // Очищаем фон
    fl_color(FL_BLACK);
    fl_rectf(0, 0, window->w(), window->h());

    // Рисуем карту
    const GameMap& map = gameModel->getMap();
    for (int r = 0; r < map.getHeight(); ++r) {
        for (int c = 0; c < map.getWidth(); ++c) {
            if (map.getTile(c, r) == TileType::Wall) {
                fl_color(FL_BLUE);
                fl_rectf(static_cast<int>(c * GameModel::TILE_SIZE), 
                        static_cast<int>(r * GameModel::TILE_SIZE), 
                        static_cast<int>(GameModel::TILE_SIZE), 
                        static_cast<int>(GameModel::TILE_SIZE));
            }
        }
    }

    // Рисуем игровые объекты
    for (const auto& obj : gameModel->getObjects()) {
        if (!obj->isDestroyed()) {
            // Проверяем тип объекта и рисуем соответственно
            if (Tank* tank = dynamic_cast<Tank*>(obj.get())) {
                drawTank(tank);
            } else if (Bullet* bullet = dynamic_cast<Bullet*>(obj.get())) {
                drawBullet(bullet);
            }
        }
    }

    // Рисуем HUD
    drawHUD();
    
    // Рисуем сообщения состояния игры
    drawGameStateMessages();
    
    // Рисуем экран результатов
    if (showResults) {
        drawResultsScreen();
    }
}

void GameView::drawTank(const Tank* tank) {
    if (!tank) return;
    
    float x = tank->getX();
    float y = tank->getY();
    Direction direction = tank->getDirection();
    bool isPlayer = tank->isPlayer();
    
    // Корпус танка
    fl_color(isPlayer ? FL_GREEN : FL_RED);
    fl_rectf(static_cast<int>(x), static_cast<int>(y), 
             static_cast<int>(GameModel::TANK_SIZE), static_cast<int>(GameModel::TANK_SIZE));

    // Рисуем башню (простая линия, указывающая направление)
    float centerX = x + GameModel::TANK_SIZE / 2.0f;
    float centerY = y + GameModel::TANK_SIZE / 2.0f;
    float turretLength = GameModel::TANK_SIZE * 0.75f;

    fl_color(FL_BLACK);
    fl_line_style(FL_SOLID, 4);

    switch(direction) {
        case Direction::UP:    fl_line(static_cast<int>(centerX), static_cast<int>(centerY), 
                                      static_cast<int>(centerX), static_cast<int>(centerY - turretLength)); break;
        case Direction::DOWN:  fl_line(static_cast<int>(centerX), static_cast<int>(centerY), 
                                      static_cast<int>(centerX), static_cast<int>(centerY + turretLength)); break;
        case Direction::LEFT:  fl_line(static_cast<int>(centerX), static_cast<int>(centerY), 
                                      static_cast<int>(centerX - turretLength), static_cast<int>(centerY)); break;
        case Direction::RIGHT: fl_line(static_cast<int>(centerX), static_cast<int>(centerY), 
                                      static_cast<int>(centerX + turretLength), static_cast<int>(centerY)); break;
    }
    fl_line_style(0);
    
    // Рисуем полоску здоровья для вражеских танков
    if (!isPlayer && tank->getHealth() < tank->getMaxHealth()) {
        drawHealthBar(tank);
    }
}

void GameView::drawBullet(const Bullet* bullet) {
    if (!bullet) return;
    
    fl_color(bullet->isFromPlayer() ? FL_YELLOW : FL_MAGENTA);
    fl_circle(static_cast<int>(bullet->getX()), static_cast<int>(bullet->getY()), 6);
}

void GameView::drawHealthBar(const Tank* tank) {
    if (!tank) return;
    
    float x = tank->getX();
    float y = tank->getY();
    int currentHealth = tank->getHealth();
    int maxHealth = tank->getMaxHealth();
    
    // Размеры полоски здоровья
    const float barWidth = GameModel::TANK_SIZE;
    const float barHeight = 6.0f;
    const float barOffsetY = -12.0f;
    
    float barX = x;
    float barY = y + barOffsetY;
    
    // Фон полоски (красный)
    fl_color(FL_RED);
    fl_rectf(static_cast<int>(barX), static_cast<int>(barY), 
             static_cast<int>(barWidth), static_cast<int>(barHeight));
    
    // Текущее здоровье (зеленый)
    if (currentHealth > 0) {
        float healthRatio = static_cast<float>(currentHealth) / static_cast<float>(maxHealth);
        float healthWidth = barWidth * healthRatio;
        
        // Цвет зависит от количества здоровья
        if (healthRatio > 0.6f) {
            fl_color(FL_GREEN);
        } else if (healthRatio > 0.3f) {
            fl_color(FL_YELLOW);
        } else {
            fl_color(FL_RED);
        }
        
        fl_rectf(static_cast<int>(barX), static_cast<int>(barY), 
                 static_cast<int>(healthWidth), static_cast<int>(barHeight));
    }
    
    // Рамка полоски
    fl_color(FL_WHITE);
    fl_rect(static_cast<int>(barX), static_cast<int>(barY), 
            static_cast<int>(barWidth), static_cast<int>(barHeight));
}

void GameView::drawHUD() {
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 20);
    
    // Счет
    std::string scoreText = "Очки: " + std::to_string(gameModel->getScore());
    fl_draw(scoreText.c_str(), 15, 35);
    
    // Здоровье
    std::string healthText = "Жизни: " + std::to_string(gameModel->getPlayerHealth());
    fl_draw(healthText.c_str(), 200, 35);
    
    // FPS
    std::ostringstream fpsStream;
    fpsStream << "FPS: " << std::fixed << std::setprecision(1) << gameModel->getFPS();
    fl_draw(fpsStream.str().c_str(), window->w() - 120, 35);
}

void GameView::drawGameStateMessages() {
    if (gameModel->getState() == GameState::PAUSED) {
        fl_color(FL_YELLOW);
        fl_font(FL_HELVETICA_BOLD, 48);
        const char* pauseMsg = "ПАУЗА";
        int msgW = static_cast<int>(fl_width(pauseMsg));
        fl_draw(pauseMsg, (window->w() - msgW)/2, window->h()/2);
    } else if (gameModel->getState() == GameState::GAME_OVER && !showResults) {
        fl_color(FL_RED);
        fl_font(FL_HELVETICA_BOLD, 48);
        const char* gameOverMsg = "ИГРА ОКОНЧЕНА";
        int msgW_go = static_cast<int>(fl_width(gameOverMsg));
        fl_draw(gameOverMsg, (window->w() - msgW_go)/2, window->h()/2 - 30);
        
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 32);
        std::string finalScore = "Ваш счет: " + std::to_string(gameModel->getScore());
        int msgW_fs = static_cast<int>(fl_width(finalScore.c_str()));
        fl_draw(finalScore.c_str(), (window->w() - msgW_fs)/2, window->h()/2 + 35);
    }
}

void GameView::drawResultsScreen() {
    // Черный фон
    fl_color(FL_BLACK);
    fl_rectf(0, 0, window->w(), window->h());
    
    // Текст результатов
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 48);
    
    std::string gameOverText = "ИГРА ОКОНЧЕНА";
    int textWidth_go = static_cast<int>(fl_width(gameOverText.c_str()));
    fl_draw(gameOverText.c_str(), (window->w() - textWidth_go) / 2, window->h() / 2 - 80);
    
    std::string scoreMsgText = "Счет: " + std::to_string(playerFinalScore);
    int textWidth_score = static_cast<int>(fl_width(scoreMsgText.c_str()));
    fl_draw(scoreMsgText.c_str(), (window->w() - textWidth_score) / 2, window->h() / 2 - 15);
    
    // Таймер обратного отсчета
    int secondsLeft = static_cast<int>(std::floor(3.0 - resultsDisplayTime)) + 1;
    if (secondsLeft < 1) secondsLeft = 1;
    std::string timerText = "Возврат в меню через: " + std::to_string(secondsLeft);
    fl_font(FL_HELVETICA, 24);
    int textWidth_timer = static_cast<int>(fl_width(timerText.c_str()));
    fl_draw(timerText.c_str(), (window->w() - textWidth_timer) / 2, window->h() / 2 + 50);
}

void GameView::gameLoopCallback(void* data) {
    GameView* view = static_cast<GameView*>(data);
    if (!view || !view->gameLoopRunning || !view->gameModel) {
        return;
    }

    // Обновляем модель только если игра идет
    if (view->gameModel->getState() == GameState::PLAYING) {
        view->gameModel->update();
    }
    
    // Проверяем, что окно еще существует перед перерисовкой
    if (view->window) {
        view->window->redraw();
    }

    // Проверяем окончание игры
    if (view->gameModel->getState() == GameState::GAME_OVER && !view->showResults) {
        view->showResults = true;
        view->playerFinalScore = view->gameModel->getPlayerScore();
        view->resultsDisplayTime = 0.0;
        
        // Останавливаем игровой цикл
        view->gameLoopRunning = false;
        
        // Запускаем таймер результатов
        Fl::add_timeout(0.1, resultsCallback, view);
        return;
    }

    // Продолжаем игровой цикл только если он все еще должен работать
    if (view->gameLoopRunning) {
        Fl::repeat_timeout(0.016, gameLoopCallback, data);
    }
}

void GameView::resultsCallback(void* data) {
    GameView* view = static_cast<GameView*>(data);
    if (!view || !view->showResults) {
        return;
    }

    view->resultsDisplayTime += 0.1;
    
    // Проверяем, что окно еще существует перед перерисовкой
    if (view->window) {
        view->window->redraw();
    }
    
    if (view->resultsDisplayTime < 3.0) {
        Fl::repeat_timeout(0.1, resultsCallback, data);
    } else {
        // Останавливаем показ результатов
        view->showResults = false;
        
        // Вызываем callback для возврата в меню
        if (view->gameOverCallbackFunc) {
            view->scheduleCallback(view->gameOverCallbackFunc);
        }
    }
}

void GameView::scheduledCallbackHandler(void* data) {
    auto callback = static_cast<CallbackFunc*>(data);
    if (callback) {
        (*callback)();
        delete callback;
    }
}

// Вложенный класс GameWindow
GameView::GameWindow::GameWindow(int w, int h, const char* title)
    : Fl_Double_Window(w, h, title), view(nullptr) {}

void GameView::GameWindow::setView(GameView* v) {
    view = v;
}

int GameView::GameWindow::handle(int event) {
    if (view && event == FL_KEYDOWN) {
        int key = Fl::event_key();
        if (view->handleKeyPress(key)) {
            return 1;
        }
    }
    return Fl_Double_Window::handle(event);
}

void GameView::GameWindow::draw() {
    if (view) {
        view->draw();
    } else {
        Fl_Double_Window::draw();
    }
}
