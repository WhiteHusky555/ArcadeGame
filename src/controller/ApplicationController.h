#pragma once
#include <memory>
#include "BaseController.h"


class MenuController;
class GameController;
class AboutController;

class ApplicationController {
public:
    ApplicationController();
    ~ApplicationController();
    
    void run();
    void showMenu();
    void showGame();
    void showAbout();

private:
    void cleanup();
    
    std::unique_ptr<BaseController> currentController;
    std::unique_ptr<BaseController> previousController; // Добавляем это поле
};
