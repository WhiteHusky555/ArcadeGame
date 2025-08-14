#include "AboutView.h"
#include "../model/AboutModel.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <memory>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>

AboutView::AboutView() : model(nullptr), textBuffer(nullptr), textDisplay(nullptr), backBtn(nullptr) {
    // Конструктор создает только базовые объекты
}

AboutView::~AboutView() {
    // Сначала очищаем callback, чтобы избежать вызовов после удаления
    backCallbackFunc = nullptr;
    
    // Удаляем виджеты
    if (window) {
        window->hide();
        
        // Удаляем все виджеты
        window->clear();
        
        // Удаляем текстовый буфер отдельно, так как он не является виджетом
        if (textBuffer) {
            delete textBuffer;
            textBuffer = nullptr;
        }
    }
}

void AboutView::setupUI() {
    window = std::make_unique<Fl_Double_Window>(WINDOW_WIDTH, WINDOW_HEIGHT, "О программе");
    window->size_range(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
    window->callback(windowCallback, this);
    
    createContent();
    
    // Создаем модель и обновляем содержимое
    model = std::make_unique<AboutModel>();
    updateContent();
    
    window->end();
}

void AboutView::createContent() {
    // Создаем текстовый буфер и дисплей
    textBuffer = new Fl_Text_Buffer();
    
    textDisplay = new Fl_Text_Display(20, 20, WINDOW_WIDTH - 40, WINDOW_HEIGHT - 100);
    textDisplay->buffer(textBuffer);
    textDisplay->wrap_mode(1, 0);
    
    // Кнопка "Назад"
    backBtn = new Fl_Button(WINDOW_WIDTH/2 - BUTTON_WIDTH/2, WINDOW_HEIGHT - 60, 
                           BUTTON_WIDTH, BUTTON_HEIGHT, "Назад");
    backBtn->callback(backCallback, this);
}

void AboutView::updateContent() {
    if (!model || !textBuffer) return;
    
    std::string content = model->getAppName() + "\n";
    content += "Версия: " + model->getVersion() + "\n";
    content += "Автор: " + model->getAuthor() + "\n\n";
    content += model->getDescription();
    
    textBuffer->text(content.c_str());
}

void AboutView::show() {
    if (window) {
        window->show();
    }
}

void AboutView::hide() {
    if (window) {
        // Отключаем все обработчики событий
        if (backBtn) {
            backBtn->callback(nullptr, nullptr);
        }
        
        // Скрываем окно
        window->hide();
    }
}

void AboutView::scheduleCallback(CallbackFunc callback) {
    if (callback) {
        Fl::add_timeout(0.01, scheduledCallbackHandler, new CallbackFunc(callback));
    }
}

void AboutView::setBackCallback(CallbackFunc cb) {
    backCallbackFunc = std::move(cb);
}

void AboutView::backCallback(Fl_Widget*, void* data) {
    AboutView* view = static_cast<AboutView*>(data);
    if (view && view->backCallbackFunc) {
        view->scheduleCallback(view->backCallbackFunc);
    }
}

void AboutView::windowCallback(Fl_Widget* w, void* data) {
    AboutView* view = static_cast<AboutView*>(data);
    if (view && view->backCallbackFunc) {
        view->scheduleCallback(view->backCallbackFunc);
    }
}

void AboutView::scheduledCallbackHandler(void* data) {
    auto callback = static_cast<CallbackFunc*>(data);
    if (callback) {
        (*callback)();
        delete callback;
    }
}
