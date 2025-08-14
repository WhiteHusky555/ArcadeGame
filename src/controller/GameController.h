#pragma once
#include "BaseController.h"
#include "../model/GameModel.h"
#include "../view/GameView.h"
#include <memory>

class GameController : public BaseController {
public:
    GameController();
    ~GameController();
    
    void show() override;
    void hide() override;
    void scheduleCleanup(CleanupFunc cleanup) override;
    
    void setBackToMenuCallback(CallbackFunc cb);

private:
    bool handleKeyPress(int key);
    
    std::unique_ptr<GameModel> model;
    std::unique_ptr<GameView> view;
    
    CallbackFunc backToMenuCallback;
};
