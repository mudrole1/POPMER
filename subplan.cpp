#include "subplan.h"


Subplan::Subplan(int id)
 : GroundAction("subplan"+std::to_string(id), true)
{
   FinalPlan();
   this->diff_gid = -1;
}

void Subplan::setLocation(std::string loc)
{
    location = loc;
}

std::string Subplan::getLocation()
{
    return location;
}

std::shared_ptr<Task> Subplan::getTask()
{
   return task;
}

void Subplan::setTask(std::shared_ptr<Task> t)
{
    task = t;
}

void Subplan::print()
{
    std::ostream& os = std::cout;
    os << "Name: " << this->name() << "\n";
    FinalPlan::print();
    os << "location " << location << "\n";
    const Expression * e;
    e = &this->min_duration();
    os << "duration: [" << this->min_duration() << "]\n";
}

void Subplan::complete() //make from this subplan action
{

    //void set_condition(const Formula& condition);

    /* Adds an effect to this action. */
    //void add_effect(const Effect& effect);

    /* Sets the duration for this action. */
    //do not delete the pointer, as it is moved to be in the action
    Value * v = new Value(this->duration());
    RCObject::ref(v);
    std::cout << *v << "\n";
    this->set_duration(*v);

}


