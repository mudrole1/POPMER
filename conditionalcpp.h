#ifndef CONDITIONALCPP_H
#define CONDITIONALCPP_H

#include <memory>
#include "finalplan.h"

class Conditional
{
public:
    static bool mainConditions(std::shared_ptr<FinalPlan> realPlan, std::map<std::string, std::vector<std::string> > execution_state,
                               int argc,  char ** argv);
};

#endif // CONDITIONALCPP_H
