#ifndef FINALPLAN_H
#define FINALPLAN_H

#include "vhpop/chain.h"
#include "vhpop/link.h"
#include <memory>
#include <vector>
#include "steptimed.h"
#include <iterator>
#include <cmath>
#include <iomanip>

class FinalPlan
{
private:
    std::shared_ptr<std::vector<std::shared_ptr<MyLink> > > links_;
    std::shared_ptr<const TemporalOrderings> orderings_;
    //pair of step and "start" or "end" of step
    std::shared_ptr<std::vector<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > > steps_;
    std::vector<std::vector<int> > steps_ordering;
    int num_main_steps;
    std::vector<bool> canComeFirst;

public:

    std::map<std::shared_ptr<StepTimed>, size_t> unfinished_actions;
    //the item on the last poistion in the vector is the last achieving condition
    std::map<std::string, std::vector<std::pair<std::string, int> > > achievers;
    std::map<std::string, std::vector<std::pair<std::string, int> > > deleters;

    void addAchiever(std::string condition, std::string when, int id);
    void addDeleter(std::string condition, std::string when, int id);

    FinalPlan();

    void addLink(std::shared_ptr<MyLink>);
    void changeLink(int, bool);
    std::shared_ptr<const TemporalOrderings> getOrderings()
    {
        return orderings_;
    }
    void setOrderings(std::shared_ptr<const TemporalOrderings> ord)
    {
        orderings_ = ord;
    }

    std::shared_ptr<std::vector<std::shared_ptr<MyLink> > > getLinks()
    {
        return links_;
    }

    size_t getUnfishedId(std::shared_ptr<StepTimed> step);

    void addStep(std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >);

    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > removeLastStep(bool test);

    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > getLastStep();

    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > getStepwithGIndex(int index);


    //return step with the id
    std::shared_ptr<StepTimed> getStep(size_t id);

    std::shared_ptr<std::vector<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > >  getSteps()
    {
        return steps_;
    }

    std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > getFullStep(size_t id);

    std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > getSeqStep(size_t index);

    int amountOfSteps()
    {
        return steps_->size();
    }

    long double duration();

    void print();

    void updateAchiDel(std::pair<std::string, size_t> step);

    void initStepsOrderings(int num_steps);

    void setStepsOrderings(int from_id, int to_id, int type, bool overwrite);

    int getStepsOrderings(int ind1, int ind2);

    int getNumMainSteps()
    {
        return num_main_steps;
    }

    void printStepsOrderings();

    void setCanComeFirst(int ind, bool value)
    {
        canComeFirst.at(ind) = value;
    }

    bool getCanComeFirst(int ind)
    {
        return canComeFirst.at(ind);
    }


    //TODO probably I will need remove step, remove link etc...
};

#endif // FINALPLAN_H
