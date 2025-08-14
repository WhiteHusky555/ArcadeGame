#include "MenuView.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <memory>

MenuView::MenuView() : newGameBtn(nullptr), aboutBtn(nullptr), exitBtn(nullptr) {
    // Конструктор создает только базовые объекты
    // Настройка UI происходит в setupUI()
}

MenuView::~MenuView() {
    // Очищаем callback'и перед удалением
    newGameCallbackFunc = nullptr;
    aboutCallbackFunc = nullptr;
    exitCallbackFunc = nullptr;
    
    // Удаляем виджеты
    if (window) {
        window->hide();
        window->clear();
    }
}

void MenuView::setupUI() {
    // Создаем окно
    window = std::make_unique<Fl_Double_Window>(WINDOW_WIDTH, WINDOW_HEIGHT, "Tanks Game - Меню");
    window->size_range(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Создаем кнопки
    createButtons();
    
    window->end();
}

void MenuView::createButtons() {
    int totalHeight = 3 * BUTTON_HEIGHT + 2 * BUTTON_SPACING;
    int startY = (WINDOW_HEIGHT - totalHeight) / 2;
    int startX = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;

    newGameBtn = new Fl_Button(startX, startY, BUTTON_WIDTH, BUTTON_HEIGHT, "Новая игра");
    newGameBtn->callback(newGameCallback, this);

    aboutBtn = new Fl_Button(startX, startY + BUTTON_HEIGHT + BUTTON_SPACING, 
                            BUTTON_WIDTH, BUTTON_HEIGHT, "О программе");
    aboutBtn->callback(aboutCallback, this);

    exitBtn = new Fl_Button(startX, startY + 2 * (BUTTON_HEIGHT + BUTTON_SPACING), 
                           BUTTON_WIDTH, BUTTON_HEIGHT, "Выход");
    exitBtn->callback(exitCallback, this);
}

void MenuView::show() {
    if (window) {
        window->show();
    }
}

void MenuView::hide() {
    if (window) {
        // Отключаем все обработчики событий
        if (newGameBtn) newGameBtn->callback(nullptr, nullptr);
        if (aboutBtn) aboutBtn->callback(nullptr, nullptr);
        if (exitBtn) exitBtn->callback(nullptr, nullptr);
        
        // Скрываем окно
        window->hide();
    }
}

void MenuView::scheduleCallback(CallbackFunc callback) {
    if (callback) {
        Fl::add_timeout(0.01, scheduledCallbackHandler, new CallbackFunc(callback));
    }
}

void MenuView::setNewGameCallback(CallbackFunc cb) {
    newGameCallbackFunc = std::move(cb);
}

void MenuView::setAboutCallback(CallbackFunc cb) {
    aboutCallbackFunc = std::move(cb);
}

void MenuView::setExitCallback(CallbackFunc cb) {
    exitCallbackFunc = std::move(cb);
}

void MenuView::newGameCallback(Fl_Widget*, void* data) {
    MenuView* view = static_cast<MenuView*>(data);
    if (view && view->newGameCallbackFunc) {
        view->scheduleCallback(view->newGameCallbackFunc);
    }
}

void MenuView::aboutCallback(Fl_Widget*, void* data) {
    MenuView* view = static_cast<MenuView*>(data);
    if (view && view->aboutCallbackFunc) {
        view->scheduleCallback(view->aboutCallbackFunc);
    }
}

void MenuView::exitCallback(Fl_Widget*, void* data) {
    MenuView* view = static_cast<MenuView*>(data);
    if (view && view->exitCallbackFunc) {
        view->scheduleCallback(view->exitCallbackFunc);
    }
}

void MenuView::scheduledCallbackHandler(void* data) {
    auto callback = static_cast<CallbackFunc*>(data);
    if (callback) {
        (*callback)();
        delete callback;
    }
}
