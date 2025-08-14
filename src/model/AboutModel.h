#pragma once
#include <string>

class AboutModel {
public:
    AboutModel();
    
    const std::string& getAppName() const;
    const std::string& getVersion() const;
    const std::string& getAuthor() const;
    const std::string& getDescription() const;

private:
    void setupInfo();
    
    std::string appName;
    std::string version;
    std::string author;
    std::string description;
};
