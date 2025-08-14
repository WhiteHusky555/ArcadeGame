#pragma once
#include <FL/Fl_Double_Window.H>
#include <functional>
#include <memory>

class BaseView {
public:
    using CallbackFunc = std::function<void()>;
    
    virtual ~BaseView() = default;
    virtual void setupUI() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void scheduleCallback(CallbackFunc callback) = 0;

protected:
    std::unique_ptr<Fl_Double_Window> window;
};
