#include "myplan.h"

MyPlan::MyPlan(std::string domain, std::string problem, std::string output)
{
   domain_actions_ = ParsePDDLfiles::parseDomain(domain);
   orderings_.reset(new TemporalOrderings());
   achievers_ = ParsePDDLfiles::parseProblem(problem,&goals_);
   links_ = std::vector<std::pair<std::shared_ptr<MyLink>, int > >();
   steps_ = ParsePDDLfiles::parseGroundActions(&achievers_, &links_, output, domain_actions_, &orderings_, &duration_, &vir_memory_, &phys_memory_);
   if(steps_.size()==0)
       complete_ = false;
   else
       complete_ = true;



}

void MyPlan::print()
{
    std::ostream &osx1 = std::cout;
    for(unsigned int i=0; i < steps_.size(); i++)
    {
        steps_.at(i).first->printName(osx1);
        std::cout << "\n";
    }
}

MyPlan::MyPlan(MyPlan & p)
 :domain_actions_(p.domain_actions_), orderings_(p.orderings_), achievers_(p.achievers_), links_(p.links_), steps_(p.steps_)
{

}

MyPlan::~MyPlan()
{
    steps_.clear();
    domain_actions_.clear();
    orderings_.reset();
}


