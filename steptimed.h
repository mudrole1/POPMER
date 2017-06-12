#ifndef STEPTIMED
#define STEPTIMED

#include <memory>
#include <iostream>
#include <sstream>

#include "task.h"
//#include "vhpop/plans.h"


//#include "vhpop/parameters.h"

class StepTimed
{
    std::shared_ptr<MyGroundAction> step;
    std::shared_ptr<Task>  task; //normal pointer - this object doesnt own task pointer
    bool backtrack;
    double endTime; //if the time is -1 - it is the start of an action. If the time is set, it is the end of an action
    double startTime; //to save in what time this step is in the plan, used in backtracking
    double timeSlack;

public:
    StepTimed(std::shared_ptr<MyGroundAction> s, std::shared_ptr<Task> t);
    StepTimed();
    StepTimed(const StepTimed&);
    std::shared_ptr<MyGroundAction> getStep();
    std::shared_ptr<Task> getTask();
    void setBacktrack(bool);
    bool getBacktrack();
    double getEndTime();
    void setEndTime(double);
    double getStartTime()
    {
        return startTime;
    }
    void setStartTime(double s)
    {
        startTime = s;
    }

    double getTimeSlack()
    {
        return timeSlack;
    }
    void setTimeSlack(double ts)
    {
        timeSlack = ts;    }

    friend std::ostream& operator<<(std::ostream&, const StepTimed&);
};


#endif // STEPTIMED

