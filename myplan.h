#ifndef MYPLAN_H
#define MYPLAN_H
#include<myactionschema.h>
#include<mygroundaction.h>
#include<parsepddlfiles.h>
#include<vhpop/orderings.h>
#include<mylink.h>

#include<memory>
#include<vector>
#include<string>
#include<map>


class MyPlan
{
private:
    //name of the action, the action
    std::map<std::string, std::shared_ptr<MyActionSchema>> domain_actions_;
    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > steps_;
    std::shared_ptr<const TemporalOrderings> orderings_;
    std::map<std::string, std::pair<std::string, int> >  achievers_;
    std::vector<std::pair<std::shared_ptr<MyLink>, int > > links_;
    std::vector<std::string> goals_;
    bool complete_;
    double duration_;
    double vir_memory_;
    double phys_memory_;

    public:
    MyPlan(std::string domain, std::string problem, std::string output);
    MyPlan(MyPlan & p);
    ~MyPlan();
    int num_steps(){return steps_.size();}

    std::map<std::string, std::shared_ptr<MyActionSchema>> getDomainActions()
    {
        return domain_actions_;
    }

    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > getSteps()
    {
        return steps_;
    }

    void setSteps(std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > s)
    {
        steps_ = s;
    }

    std::shared_ptr<const TemporalOrderings> getOrderings()
    {
        return orderings_;
    }

    std::map<std::string, std::pair<std::string, int> > getAchievers()
    {
        return achievers_;
    }
    std::vector<std::pair<std::shared_ptr<MyLink>, int > > getLinks()
    {
        return links_;
    }

    void setLinks(std::vector<std::pair<std::shared_ptr<MyLink>, int > >  links)
    {
        links_ = links;
    }

    bool complete()
    {
        return complete_;
    }
    double getDuration()
    {
       return duration_;
    }
    void setDuration(double dur)
    {
        duration_ = dur;
    }

    std::vector<std::string> getGoals()
    {
        return goals_;
    }

    double getVirMemory()
    {
        return vir_memory_;
    }
    double getPhyMemory()
    {
        return phys_memory_;
    }

    void print();





};

#endif // MYPLAN_H
