#include "MenuModel.h"

MenuModel::MenuModel() {
    // Модель меню может содержать настройки, последние результаты игры и т.д.
    setupDefaults();
}

void MenuModel::setupDefaults() {
    // Инициализация значений по умолчанию
    lastScore = 0;
    gamesPlayed = 0;
}

void MenuModel::updateLastScore(int score) {
    lastScore = score;
    gamesPlayed++;
}

int MenuModel::getLastScore() const {
    return lastScore;
}

int MenuModel::getGamesPlayed() const {
    return gamesPlayed;
}
