#include "GameController.h"

GameController::GameController() {
    model = std::make_unique<GameModel>();
    view = std::make_unique<GameView>();
    
    // Инициализируем модель
    if (!model->init("../resources/map.txt")) {
        // Обработка ошибки загрузки карты
        return;
    }
    
    // Настраиваем представление
    view->setupUI();
    view->setModel(model.get());
    
    // Устанавливаем обработчики событий
    view->setKeyPressCallback([this](int key) -> bool {
        return handleKeyPress(key);
    });
    
    view->setGameOverCallback([this]() {
        if (backToMenuCallback) {
            view->scheduleCallback(backToMenuCallback);
        }
    });
}

GameController::~GameController() {
    // Очищаем callback перед удалением
    backToMenuCallback = nullptr;
    
    if (view) {
        view->stopGame();
        view->stopResultsTimer();
        view->hide();
    }
}

void GameController::show() {
    if (view && model) {
        model->reset();
        view->show();
        view->startGame();
    }
}

void GameController::hide() {
    if (view) {
        view->stopGame();
        view->stopResultsTimer();
        view->hide();
    }
}

void GameController::scheduleCleanup(CleanupFunc cleanup) {
    if (view) {
        view->scheduleCallback(cleanup);
    }
}

void GameController::setBackToMenuCallback(CallbackFunc cb) {
    backToMenuCallback = std::move(cb);
}

bool GameController::handleKeyPress(int key) {
    if (!model || model->getState() == GameState::GAME_OVER) {
        return false;
    }
    
    // Пауза
    if (key == 'p' || key == 'P') {
        if (model->getState() == GameState::PLAYING) {
            model->setState(GameState::PAUSED);
        } else if (model->getState() == GameState::PAUSED) {
            model->setState(GameState::PLAYING);
        }
        return true;
    }

    if (model->getState() != GameState::PLAYING) return false;

    switch (key) {
        case 65362: // FL_Up
            model->playerMove(Direction::UP);
            return true;
        case 65364: // FL_Down
            model->playerMove(Direction::DOWN);
            return true;
        case 65361: // FL_Left
            model->playerMove(Direction::LEFT);
            return true;
        case 65363: // FL_Right
            model->playerMove(Direction::RIGHT);
            return true;
        case ' ':
            model->playerFire();
            return true;
        default:
            return false;
    }
}
