#include "state.h"

State::State(std::map<std::string, std::vector<std::string> > vl)
{
  valid_literals = vl;
}

bool State::isLitPresented(std::string domain, std::string literal)
{
    if(std::find(valid_literals[domain].begin(),valid_literals[domain].end(),literal) != valid_literals[domain].end()) //something was found
    {
        return true;
    }
    else
    {
        return false;
    }
}

