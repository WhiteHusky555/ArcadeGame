#pragma once

class MenuModel {
public:
    MenuModel();
    
    void updateLastScore(int score);
    int getLastScore() const;
    int getGamesPlayed() const;

private:
    void setupDefaults();
    
    int lastScore;
    int gamesPlayed;
};
