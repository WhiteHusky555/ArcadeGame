#pragma once
#include <functional>

class BaseController {
public:
    using CallbackFunc = std::function<void()>;
    using CleanupFunc = std::function<void()>;
    
    virtual ~BaseController() = default;
    virtual void show() = 0;
    virtual void hide() {}
    virtual void scheduleCleanup(CleanupFunc cleanup) {}
};
