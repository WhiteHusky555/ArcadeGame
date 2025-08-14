#pragma once
#include "BaseController.h"
#include "../model/AboutModel.h"
#include "../view/AboutView.h"
#include <memory>

class AboutController : public BaseController {
public:
    AboutController();
    ~AboutController();
    
    void show() override;
    void hide() override;
    void scheduleCleanup(CleanupFunc cleanup) override;
    
    void setBackCallback(CallbackFunc cb);

private:
    std::unique_ptr<AboutModel> model;
    std::unique_ptr<AboutView> view;
    
    CallbackFunc backCallback;
};
