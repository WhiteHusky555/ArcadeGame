#include "AboutModel.h"

AboutModel::AboutModel() {
    setupInfo();
}

void AboutModel::setupInfo() {
    appName = "FLTK OF TANKS";
    version = "1.0.0";
    author = "Непрокин Иван";
    description = "Классическая игра в танки\n\n"
                  "Управление:\n"
                  "Стрелки - движение\n"
                  "Пробел - выстрел\n"
                  "P - пауза\n\n"
                  "Цель: Уничтожить все вражеские танки!";
}

const std::string& AboutModel::getAppName() const {
    return appName;
}

const std::string& AboutModel::getVersion() const {
    return version;
}

const std::string& AboutModel::getAuthor() const {
    return author;
}

const std::string& AboutModel::getDescription() const {
    return description;
}
