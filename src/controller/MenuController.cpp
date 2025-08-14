#include "MenuController.h"
#include "../view/MenuView.h"
#include "../model/MenuModel.h"

MenuController::MenuController() {
    model = std::make_unique<MenuModel>();
    view = std::make_unique<MenuView>();
    
    // Представление настраивает себя самостоятельно
    view->setupUI();
    
    // Устанавливаем обработчики событий
    view->setNewGameCallback([this]() {
        if (newGameCallback) {
            view->scheduleCallback(newGameCallback);
        }
    });
    
    view->setAboutCallback([this]() {
        if (aboutCallback) {
            view->scheduleCallback(aboutCallback);
        }
    });
    
    view->setExitCallback([this]() {
        if (exitCallback) {
            view->scheduleCallback(exitCallback);
        }
    });
}

MenuController::~MenuController() {
    // Очищаем callback'и перед удалением
    newGameCallback = nullptr;
    aboutCallback = nullptr;
    exitCallback = nullptr;
    
    // Скрываем представление
    if (view) {
        view->hide();
    }
}

void MenuController::show() {
    if (view) {
        view->show();
    }
}

void MenuController::hide() {
    if (view) {
        view->hide();
    }
}

void MenuController::scheduleCleanup(CleanupFunc cleanup) {
    if (view) {
        view->scheduleCallback(cleanup);
    }
}

void MenuController::setNewGameCallback(CallbackFunc cb) {
    newGameCallback = std::move(cb);
}

void MenuController::setAboutCallback(CallbackFunc cb) {
    aboutCallback = std::move(cb);
}

void MenuController::setExitCallback(CallbackFunc cb) {
    exitCallback = std::move(cb);
}
