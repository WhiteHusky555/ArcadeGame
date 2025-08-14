#include "GameView.h"
#include "../model/GameModel.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <sstream>
#include <iomanip>
#include <cmath>

GameView::GameView() : gameModel(nullptr), gameLoopRunning(false) {
    // Конструктор создает только базовые объекты
}

GameView::~GameView() {
    stopGame();
}

void GameView::setupUI() {
    // Размеры окна будут установлены после загрузки модели
    window = std::make_unique<GameWindow>(800, 600, "Tanks Game");
    
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
    if (window) {
        window->hide();
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
        
        window->resize(window->x(), window->y(), newWidth, newHeight);
        window->size_range(newWidth, newHeight, newWidth, newHeight);
    }
}

void GameView::startGame() {
    if (!gameLoopRunning && gameModel) {
        gameLoopRunning = true;
        Fl::add_timeout(0.016, gameLoopCallback, this);
    }
}

void GameView::stopGame() {
    if (gameLoopRunning) {
        gameLoopRunning = false;
        Fl::remove_timeout(gameLoopCallback, this);
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
            obj->draw();
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

void GameView::drawHUD() {
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 16);
    
    // Счет
    std::string scoreText = "Очки: " + std::to_string(gameModel->getScore());
    fl_draw(scoreText.c_str(), 10, 25);
    
    // Здоровье
    std::string healthText = "Жизни: " + std::to_string(gameModel->getPlayerHealth());
    fl_draw(healthText.c_str(), 150, 25);
    
    // FPS
    std::ostringstream fpsStream;
    fpsStream << "FPS: " << std::fixed << std::setprecision(1) << gameModel->getFPS();
    fl_draw(fpsStream.str().c_str(), window->w() - 100, 25);
}

void GameView::drawGameStateMessages() {
    if (gameModel->getState() == GameState::PAUSED) {
        fl_color(FL_YELLOW);
        fl_font(FL_HELVETICA_BOLD, 36);
        const char* pauseMsg = "ПАУЗА";
        int msgW = static_cast<int>(fl_width(pauseMsg));
        fl_draw(pauseMsg, (window->w() - msgW)/2, window->h()/2);
    } else if (gameModel->getState() == GameState::GAME_OVER && !showResults) {
        fl_color(FL_RED);
        fl_font(FL_HELVETICA_BOLD, 36);
        const char* gameOverMsg = "ИГРА ОКОНЧЕНА";
        int msgW_go = static_cast<int>(fl_width(gameOverMsg));
        fl_draw(gameOverMsg, (window->w() - msgW_go)/2, window->h()/2 - 25);
        
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 24);
        std::string finalScore = "Ваш счет: " + std::to_string(gameModel->getScore());
        int msgW_fs = static_cast<int>(fl_width(finalScore.c_str()));
        fl_draw(finalScore.c_str(), (window->w() - msgW_fs)/2, window->h()/2 + 25);
    }
}

void GameView::drawResultsScreen() {
    // Черный фон
    fl_color(FL_BLACK);
    fl_rectf(0, 0, window->w(), window->h());
    
    // Текст результатов
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 36);
    
    std::string gameOverText = "ИГРА ОКОНЧЕНА";
    int textWidth_go = static_cast<int>(fl_width(gameOverText.c_str()));
    fl_draw(gameOverText.c_str(), (window->w() - textWidth_go) / 2, window->h() / 2 - 60);
    
    std::string scoreMsgText = "Счет: " + std::to_string(playerFinalScore);
    int textWidth_score = static_cast<int>(fl_width(scoreMsgText.c_str()));
    fl_draw(scoreMsgText.c_str(), (window->w() - textWidth_score) / 2, window->h() / 2 - 10);
    
    // Таймер обратного отсчета
    int secondsLeft = static_cast<int>(std::floor(3.0 - resultsDisplayTime)) + 1;
    if (secondsLeft < 1) secondsLeft = 1;
    std::string timerText = "Возврат в меню через: " + std::to_string(secondsLeft);
    fl_font(FL_HELVETICA, 18);
    int textWidth_timer = static_cast<int>(fl_width(timerText.c_str()));
    fl_draw(timerText.c_str(), (window->w() - textWidth_timer) / 2, window->h() / 2 + 40);
}

void GameView::gameLoopCallback(void* data) {
    GameView* view = static_cast<GameView*>(data);
    if (!view || !view->gameLoopRunning || !view->gameModel) return;

    // Обновляем модель только если игра идет
    if (view->gameModel->getState() == GameState::PLAYING) {
        view->gameModel->update();
    }
    
    view->window->redraw();

    // Проверяем окончание игры
    if (view->gameModel->getState() == GameState::GAME_OVER && !view->showResults) {
        view->showResults = true;
        view->playerFinalScore = view->gameModel->getPlayerScore();
        view->resultsDisplayTime = 0.0;
        Fl::add_timeout(0.1, resultsCallback, view);
    }

    if (view->gameLoopRunning) {
        Fl::repeat_timeout(0.016, gameLoopCallback, data);
    }
}

void GameView::resultsCallback(void* data) {
    GameView* view = static_cast<GameView*>(data);
    if (!view || !view->showResults) return;

    view->resultsDisplayTime += 0.1;
    
    if (view->resultsDisplayTime < 3.0) {
        view->window->redraw();
        Fl::repeat_timeout(0.1, resultsCallback, data);
    } else {
        view->showResults = false;
        if (view->gameOverCallbackFunc) {
            view->gameOverCallbackFunc();
        }
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
