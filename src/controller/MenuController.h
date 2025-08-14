#pragma once
#include "BaseController.h"
#include "../model/MenuModel.h"
#include "../view/MenuView.h"
#include <memory>

class MenuController : public BaseController {
public:
    MenuController();
    ~MenuController();
    
    void show() override;
    void hide() override;
    void scheduleCleanup(CleanupFunc cleanup) override;
    
    void setNewGameCallback(CallbackFunc cb);
    void setAboutCallback(CallbackFunc cb);
    void setExitCallback(CallbackFunc cb);

private:
    std::unique_ptr<MenuModel> model;
    std::unique_ptr<MenuView> view;
    
    CallbackFunc newGameCallback;
    CallbackFunc aboutCallback;
    CallbackFunc exitCallback;
};
