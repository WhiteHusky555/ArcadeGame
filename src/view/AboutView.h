#pragma once
#include "BaseView.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <memory>

class AboutModel;
class Fl_Text_Buffer;

class AboutView : public BaseView {
public:
    AboutView();
    ~AboutView();
    
    void setupUI() override;
    void show() override;
    void hide() override;
    void scheduleCallback(CallbackFunc callback) override;
    
    void setBackCallback(CallbackFunc cb);

private:
    void createContent();
    void updateContent();
    
    static void backCallback(Fl_Widget*, void*);
    static void windowCallback(Fl_Widget*, void*);
    static void scheduledCallbackHandler(void* data);
    
    std::unique_ptr<AboutModel> model;
    
    Fl_Text_Display* textDisplay = nullptr;
    Fl_Text_Buffer* textBuffer = nullptr;
    Fl_Button* backBtn = nullptr;
    
    CallbackFunc backCallbackFunc;
    
    static constexpr int WINDOW_WIDTH = 500;
    static constexpr int WINDOW_HEIGHT = 400;
    static constexpr int BUTTON_WIDTH = 100;
    static constexpr int BUTTON_HEIGHT = 40;
};
