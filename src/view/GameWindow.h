#pragma once
#include "BaseView.h"
#include <FL/Fl_Double_Window.H>
#include <functional>

class GameModel;

class GameView : public BaseView {
public:
    using KeyCallbackFunc = std::function<bool(int)>;
    
    GameView();
    ~GameView();
    
    void setupUI() override;
    void show() override;
    void hide() override;
    
    void setModel(GameModel* model);
    void startGame();
    void stopGame();
    
    void setKeyPressCallback(KeyCallbackFunc cb);
    void setGameOverCallback(CallbackFunc cb);
    
    bool handleKeyPress(int key);
    void draw();

private:
    // Вложенный класс для окна игры
    class GameWindow : public Fl_Double_Window {
    public:
        GameWindow(int w, int h, const char* title);
        void setView(GameView* view);
        int handle(int event) override;
        void draw() override;
        
    private:
        GameView* view;
    };
    
    void drawHUD();
    void drawGameStateMessages();
    void drawResultsScreen();
    
    static void gameLoopCallback(void* data);
    static void resultsCallback(void* data);
    
    GameModel* gameModel;
    bool gameLoopRunning;
    
    KeyCallbackFunc keyPressCallbackFunc;
    CallbackFunc gameOverCallbackFunc;
    
    // Экран результатов
    bool showResults = false;
    int playerFinalScore = 0;
    double resultsDisplayTime = 0.0;
    
    static constexpr int HUD_AREA_HEIGHT = 40;
};
