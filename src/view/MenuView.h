#pragma once
#include "BaseView.h"
#include <FL/Fl_Button.H>
#include <functional>
#include <memory>

class MenuView : public BaseView {
public:
    MenuView();
    ~MenuView();
    
    void setupUI() override;
    void show() override;
    void hide() override;
    void scheduleCallback(CallbackFunc callback) override;
    
    void setNewGameCallback(CallbackFunc cb);
    void setAboutCallback(CallbackFunc cb);
    void setExitCallback(CallbackFunc cb);

private:
    void createButtons();
    
    static void newGameCallback(Fl_Widget*, void*);
    static void aboutCallback(Fl_Widget*, void*);
    static void exitCallback(Fl_Widget*, void*);
    static void scheduledCallbackHandler(void* data);
    
    Fl_Button* newGameBtn = nullptr;
    Fl_Button* aboutBtn = nullptr;
    Fl_Button* exitBtn = nullptr;
    
    CallbackFunc newGameCallbackFunc;
    CallbackFunc aboutCallbackFunc;
    CallbackFunc exitCallbackFunc;
    
    static constexpr int WINDOW_WIDTH = 400;
    static constexpr int WINDOW_HEIGHT = 400;
    static constexpr int BUTTON_WIDTH = 200;
    static constexpr int BUTTON_HEIGHT = 50;
    static constexpr int BUTTON_SPACING = 20;
};
