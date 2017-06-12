#ifndef SUBPLAN_H
#define SUBPLAN_H

#include "finalplan.h"
#include "vhpop/actions.h"
#include "vhpop/expressions.h"

class Subplan: public FinalPlan, public GroundAction
{
private:
    std::shared_ptr<Task> task;
    std::string location;
public:
    int diff_gid; //the ids in the plan must start from 1,
    //this safes the difference to global_id
    std::vector<std::string> preconditions;
    std::vector<std::string> effects;
    std::vector<std::string> parameters;
    std::shared_ptr<Task> getTask();
    void setTask(std::shared_ptr<Task> t);

    Subplan(int id);
    void complete();
    void setLocation(std::string loc);
    std::string getLocation();
    void print();

};

#endif // SUBPLAN_H
