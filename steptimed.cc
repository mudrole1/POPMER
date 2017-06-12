#include <memory>
#include <iostream>

#include "task.h"
#include "vhpop/plans.h"
#include "steptimed.h"
#include "vhpop/bindings.h"


StepTimed::StepTimed(std::shared_ptr<MyGroundAction> s, std::shared_ptr<Task> t)
{
    step = s;
    task = t;
    backtrack = false;
    endTime = -1;
    startTime = -1;
    timeSlack = -1;
}


StepTimed::StepTimed()
{
    step = nullptr;
    task = nullptr;
    backtrack = false;
    endTime = -1;
    startTime = -1;
    timeSlack = -1;
}

StepTimed::StepTimed(const StepTimed & o)
    :backtrack(o.backtrack), endTime(o.endTime), startTime(o.startTime), timeSlack(o.timeSlack)
{
    step.reset(new MyGroundAction(*o.step.get()));
    task.reset(new Task(*o.task.get()));
}

std::shared_ptr<MyGroundAction> StepTimed::getStep()
{
    return step;
}

std::shared_ptr<Task> StepTimed::getTask()
{
    return task;
}

void StepTimed::setBacktrack(bool m)
{
    backtrack= m;
}

bool StepTimed::getBacktrack()
{
    return backtrack;
}

double StepTimed::getEndTime()
{
   return endTime;
}
void StepTimed::setEndTime(double et)
{
    endTime = et;
}

std::ostream& operator<<(std::ostream& os, const StepTimed& s)
{

   os << *s.step.get();

   return os;
}
