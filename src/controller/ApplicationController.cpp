#include "ApplicationController.h"
#include "MenuController.h"
#include "GameController.h"
#include "AboutController.h"
#include <FL/Fl.H>
#include <cstdlib>

ApplicationController::ApplicationController() 
    : currentController(nullptr), previousController(nullptr) {
    showMenu();
}

ApplicationController::~ApplicationController() {
    cleanup();
}

void ApplicationController::run() {
    Fl::run();
}

void ApplicationController::showMenu() {
    // Создаем новый контроллер меню
    auto menuController = std::make_unique<MenuController>();
    
    // Настраиваем обработчики
    menuController->setNewGameCallback([this]() { 
        showGame();
    });
    
    menuController->setAboutCallback([this]() { 
        showAbout();
    });
    
    menuController->setExitCallback([this]() { 
        std::exit(0); 
    });
    
    // Показываем новый контроллер
    menuController->show();
    
    // Сохраняем текущий контроллер как предыдущий
    previousController = std::move(currentController);
    
    // Устанавливаем новый контроллер как текущий
    currentController = std::move(menuController);
    
    // Удаляем предыдущий контроллер через отложенный вызов
    Fl::add_timeout(0.1, [](void* data) {
        ApplicationController* app = static_cast<ApplicationController*>(data);
        if (app) {
            app->previousController.reset();
        }
    }, this);
}

void ApplicationController::showGame() {
    // Создаем новый контроллер игры
    auto gameController = std::make_unique<GameController>();
    
    // Настраиваем обработчики
    gameController->setBackToMenuCallback([this]() { 
        showMenu();
    });
    
    // Показываем новый контроллер
    gameController->show();
    
    // Сохраняем текущий контроллер как предыдущий
    previousController = std::move(currentController);
    
    // Устанавливаем новый контроллер как текущий
    currentController = std::move(gameController);
    
    // Удаляем предыдущий контроллер через отложенный вызов
    Fl::add_timeout(0.1, [](void* data) {
        ApplicationController* app = static_cast<ApplicationController*>(data);
        if (app) {
            app->previousController.reset();
        }
    }, this);
}

void ApplicationController::showAbout() {
    // Создаем новый контроллер "О программе"
    auto aboutController = std::make_unique<AboutController>();
    
    // Настраиваем обработчики
    aboutController->setBackCallback([this]() { 
        showMenu();
    });
    
    // Показываем новый контроллер
    aboutController->show();
    
    // Сохраняем текущий контроллер как предыдущий
    previousController = std::move(currentController);
    
    // Устанавливаем новый контроллер как текущий
    currentController = std::move(aboutController);
    
    // Удаляем предыдущий контроллер через отложенный вызов
    Fl::add_timeout(0.1, [](void* data) {
        ApplicationController* app = static_cast<ApplicationController*>(data);
        if (app) {
            app->previousController.reset();
        }
    }, this);
}

void ApplicationController::cleanup() {
    // Очищаем все контроллеры
    currentController.reset();
    previousController.reset();
}
