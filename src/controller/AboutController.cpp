#include "AboutController.h"

AboutController::AboutController() {
    model = std::make_unique<AboutModel>();
    view = std::make_unique<AboutView>();
    
    // Представление настраивает себя самостоятельно
    view->setupUI();
    
    // Устанавливаем обработчики событий
    view->setBackCallback([this]() {
        if (backCallback) {
            view->scheduleCallback(backCallback);
        }
    });
}

AboutController::~AboutController() {
    // Очищаем callback перед удалением
    backCallback = nullptr;
    
    // Скрываем представление
    if (view) {
        view->hide();
    }
}

void AboutController::show() {
    if (view) {
        view->show();
    }
}

void AboutController::hide() {
    if (view) {
        view->hide();
    }
}

void AboutController::scheduleCleanup(CleanupFunc cleanup) {
    if (view) {
        view->scheduleCallback(cleanup);
    }
}

void AboutController::setBackCallback(CallbackFunc cb) {
    backCallback = std::move(cb);
}
