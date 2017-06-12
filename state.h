#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

class State
{
private:
    std::map<std::string, std::vector<std::string> > valid_literals;
public:
    State(std::map<std::string, std::vector<std::string> > vl);
    bool isLitPresented(std::string domain, std::string literal);
};

#endif // STATE_H
