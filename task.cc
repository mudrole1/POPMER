/**
  definition of a task

  @author Lenka Mudrova
  @version 1.0 09/12/2015
*/

#include "task.h"


/* Name of current file. */
/*std::string current_file;
int warning_level;
int verbosity;*/

//to order schedule
bool compareSF(std::pair<std::pair<size_t, float>,int> a, std::pair<std::pair<size_t, float>, int> b)
{
  return a.first.second < b.first.second;
}

/**
  constructor
  @param ID of the task, release time, deadline, domain file, problem file
  @return nan, it is constructor
*/
Task::Task(int ID, double r, double d, std::string d_file, std::string p_file)
{
  id = ID;
  release_date = r;
  deadline = d;
  domain_file = d_file;
  problem_file = p_file;
  name_problem = "";

  plan_available = false;
  duration = -1;
  used = false;

  v_callable = Vhpop_callable();

  parent_task = -1;
  parent_step = -1;

}

Task::Task(int ID, double r, double d, std::string d_file, std::string p_file, std::string name_prob)
{
  id = ID;
  release_date = r;
  deadline = d;
  domain_file = d_file;
  problem_file = p_file;
  name_problem = name_prob;

  plan_available = false;
  duration = -1;
  used = false;

  v_callable = Vhpop_callable();

  parent_task = -1;
  parent_step = -1;

}

Task::Task(const Task & t)
    :id(t.id),release_date(t.release_date),deadline(t.deadline), domain_file(t.domain_file),
      problem_file(t.problem_file), name_problem(t.name_problem),plan_available(t.plan_available),
      duration(t.duration), used(t.used), start_vector(t.start_vector), v_callable(t.v_callable),
      parent_task(t.parent_task), parent_step(t.parent_step)
{
    if (t.plan != 0)
    {
        plan = std::shared_ptr<MyPlan>(new MyPlan(*t.plan.get()));
    }
    else
        plan = 0;

    std::vector<std::pair<std::shared_ptr<const Link> ,bool> > links_vector; //to save links

    for(int i=0;i<t.links_vector.size();i++)
    {
        std::shared_ptr<const Link> ptr(new Link(*t.links_vector.at(i).first.get()));
        std::pair<std::shared_ptr<const Link> ,bool> pair(ptr,t.links_vector.at(i).second);
        this->links_vector.push_back(pair);
    }
}

/*Task::~Task()
{

    for(int i=0;i<links_vector.size();i++)
    {
        //links_vector.at(i).first.release(); //release ownership of the pointer
    }
    //everything else can be done automatically

}*/

int Task::getID()
{
    return id;
}

double Task::getReleaseDate()
{
    return release_date;
}

double Task::getDeadline()
{
    return deadline;
}

double Task::getDuration()
{
    return duration;
}


std::shared_ptr<MyPlan> Task::getPlan()
{
    //if (plan_available)
    //{
        return plan;
    //}
    //else
   //     return NULL;
}

std::string Task::getDomain()
{
    return domain_file;
}

std::string Task::getProblem()
{
    return problem_file;
}

void Task::setUsed(bool u)
{
    used = u;
}

bool Task::getUsed()
{
    return used;
}

void Task::setNameProblem(std::string name)
{
    name_problem = name;
}

std::string Task::getNameProblem()
{
    return name_problem;
}

int Task::getParentTask()
{
    return parent_task;
}

void Task::setParentTask(int id)
{
    if(id>-1) //only original task can be parent
    {
        parent_task = id;
    }
    else
    {
        std::cerr << "task::ERROR:only oroginal task can be parent!\n";
        throw;
    }
}

int Task::getParentStepId()
{
    return parent_step;
}

void Task::setParentStepId(int id)
{
    parent_step = id;
}


int Task::callPlanner(int argc, char* argv[])
{

    std::string all = "ulimit -t 10;";
    for (unsigned int i=0; i < argc-3; i++)
    {
        all+= std::string(argv[i]);
        all+=" ";
    }

    std::string domain = std::string(argv[argc-3]);
    std::string problem = std::string(argv[argc-2]);
    std::string output = std::string(argv[argc-1]);

    all+= domain + " ";
    all+=problem+ " ";
    all+=" > ";
    all+= output;
    all+= " 2>&1";


    system(all.c_str());

    all = "ulimit -t 1800;";

    system(all.c_str());

    std::ifstream file_in;
    bool success = false;

    file_in.open(output.c_str());
    if (file_in.is_open())
        success = true;
    else
        throw; //it should alway create file -> with "no solution";

    file_in.close();
    if (success)
    {

        plan.reset(new MyPlan(domain, problem, output));
        if(plan->complete())
        {
          plan_available = true;
          std::string command = "rm "+output;
          system(command.c_str());
        }
        else
        {
          std::string command = "rm "+output;
          system(command.c_str());
          plan_available = false;

        }
    }
    else
    {
        plan_available = false;
        plan.reset();
    }

    return 0;
}

std::ostream& operator<<(std::ostream& os, const Task& t)
{
   os << "[" << t.id <<"," << t.release_date << "," <<t.deadline << "," << t.domain_file << "," << t.problem_file << t.name_problem << "]\n";
   return os;
}

bool operator== (Task &t1, Task &t2)
{
    if((t1.id == t2.id)&&(t1.release_date == t2.release_date)&&
      (t1.deadline == t2.deadline)&&(t1.duration == t2.duration)&&
      (t1.domain_file == t2.domain_file)&&(t1.name_problem == t2.name_problem))
    {
        return true;
    }
    else
        return false;

}




