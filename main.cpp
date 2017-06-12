#include "merging.h"
#include "vhpop/vhpop_callable.h"
#include "vhpop/plans.h"
#include "vhpop/domains.h"
#include "vhpop/problems.h"
#include "vhpop/formulas.h"
#include "task.h"
#include "steptimed.h"
#include "vhpop/bindings.h"
#include "finalplan.h"
#include "vhpop/effects.h"
#include "tree_four.h"
//#include "conditionalcpp.h"
#include "subplan.h"
#include "vhpop/effects.h"
#include "vhpop/orderings.h"
#include "parsepddlfiles.h"
#include "myplan.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <exception>
#include <algorithm>

#include "stdio.h"
#include "string.h"

#include "sys/times.h"
#include "sys/vtimes.h"

#include <ctime>
#include <ratio>
#include <chrono>

#define EXECUTION 0
#define DEBUG 0
#define DEBUG1 0
#define DEBUG2 0

static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;


int parseLine(char* line){
         int i = strlen(line);
         while (*line < '0' || *line > '9') line++;
         line[i-3] = '\0';
         i = atoi(line);
         return i;
}


int getVirtualMemory(){ //Note: this value is in KB!
         FILE* file = fopen("/proc/self/status", "r");
         int result = -1;
         char line[128];


         while (fgets(line, 128, file) != NULL){
             if (strncmp(line, "VmSize:", 7) == 0){
                 result = parseLine(line);
                 break;
             }
         }
         fclose(file);
         return result;
}

int getPhysicalMemory(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

/*to get cpu time*/
void init(){
        FILE* file;
        struct tms timeSample;
        char line[128];


        lastCPU = times(&timeSample);
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;


        file = fopen("/proc/cpuinfo", "r");
        numProcessors = 0;
        while(fgets(line, 128, file) != NULL){
            if (strncmp(line, "processor", 9) == 0) numProcessors++;
        }
        fclose(file);
}

double getCurrentValueCPU(){
        struct tms timeSample;
        clock_t now;
        double percent;


        now = times(&timeSample);
        if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
            timeSample.tms_utime < lastUserCPU){
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else{
            percent = (timeSample.tms_stime - lastSysCPU) +
                (timeSample.tms_utime - lastUserCPU);
            percent /= (now - lastCPU);
            percent /= numProcessors;
            percent *= 100;
        }
        lastCPU = now;
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;


        return percent;
}

/*class myexception: public exception
{
  virtual const char* what() const throw()
  {
    return "My exception happened";
  }
} myex;*/


//key is domain, value is a vector of valid predicates
std::map<std::string, std::vector<std::string> > current_state;
std::map<std::string, std::vector<std::string> > execution_state;

int g_argc;
char ** g_argv;
int g_temp_prob = 0;

/* Name of current file. */
//std::string current_file;
/* Level of warnings. */
//int warning_level = 1;
/* Verbosity level. */
//int verbosity =1;

/* Default planning parameters. */
//Parameters params;

int g_id = 0; //to rename steps ID in the finalplan

std::shared_ptr<std::vector< std::shared_ptr<Task> > > tasks(new std::vector< std::shared_ptr<Task> >() );
std::shared_ptr< std::vector< std::shared_ptr<Task> > > temp_tasks(new std::vector<std::shared_ptr<Task>>());

std::shared_ptr< std::vector< std::shared_ptr<Task> > > temp_tasks_used(new std::vector<std::shared_ptr<Task>>());

//task_id, action_id of the original, task_id, action_id of the similar one
std::map< std::pair<int,int>, std::pair<int,int> > similar_actions;

std::vector<std::shared_ptr<Subplan> > subplans;

std::string move_action;
std::string move_preposition;
int move_waypoint_pos;

std::vector<int> previously_not_useful;

double incr = 0.01;

int number_of_backtracks = 0;

//constraints
std::vector< std::vector<int> > constraints;
/*adding all steps from a task to the activeList*/

double g_vir_memory = 0;
double g_phy_memory = 0;



bool findUsed(std::pair<std::shared_ptr<MyGroundAction>, int >  a)
{
    //1 means used
    //0 being merged
    //-1 not used
    if(a.second==-1)
        return true;
    else
        return false;
}

bool findHalfUsed(std::pair<std::shared_ptr<MyGroundAction>, int > a)
{
    //1 means used
    //0 being merged
    //-1 not used
    if((a.second==-1)||(a.second ==0))
        return true;
    else
        return false;
}


bool zerosPrec(int v)
{
    return (v == 0);
}

bool fourPrec(int v)
{
    return ((v == 4)||(v == -4));
}

bool twosPrec(int v)
{
    return (v == 2);
}

int op_decrease (int i) { return i--; }

bool sortRelease(std::pair<double, int>  a, std::pair<double, int>  b)
{

    return a.first < b.first;
}






bool taskUnused(std::shared_ptr<Task> task)
{
    return !task->getUsed();
}

bool compareDeadlines(const std::pair<double, int>& firstElem, const std::pair<double, int>& secondElem) {
  return firstElem.first < secondElem.first;

}

bool canBeThreaten(std::shared_ptr<MyLink> link)
{
    return link->getBeingThreaten();
}

/*05/09 checked
 * this method splits conjuction of literals to single literals, time separately
 * the output is pair of the string literal (can include not) with string time
 * condition = the literal to be split
 */
static std::vector<std::pair<std::string,std::string> > getSubConditions(std::string condition)
{
    if(condition.find("?")!=std::string::npos)
    {
        std::cerr << "main::getSubConditions:ERROR: wrong bindings\n";
        throw;
    }
    std::vector<std::pair<std::string,std::string> > sub_conditions;
    std::string sub_condition;
    int skip_index;
    size_t index;
    std::string when;

    int index2 = -1;
    size_t index3 = std::string::npos;

    if(condition.find("and") != std::string::npos) //and statement
    {
        skip_index = 1; //skip the first bracket
    }
    else
    {
        skip_index = 0;
    }
        index = condition.find("(",skip_index);
        //find all predicates
        while( index != std::string::npos) //something found
        {

            skip_index = condition.find(")",skip_index+1);
            while(true)
            {

                if(condition.substr(skip_index+1,1)==")")
                {
                    if(condition.find("and") != std::string::npos) //we need to skip last bracket
                    {
                      if((skip_index+1) < (condition.size()-1))
                        skip_index++;
                      else
                        break;
                    }
                    else
                    {
                      skip_index++;
                    }

                }
                else
                  break;
            }
            sub_condition = condition.substr(index,skip_index-index+1); //those predicates can have "not"
            index = condition.find("(",skip_index);
            //subcondition contains "at end" or over all

            std::pair<std::string,std::string> condition_time_pair;
            if(sub_condition.find("at end")!=std::string::npos)
            {
                when = "at end";
                index2 = sub_condition.find(when);
                index3 = sub_condition.find(")");
                if(sub_condition.find("not")!=std::string::npos)
                    index3++;
                condition_time_pair.first = sub_condition.substr(index2+when.size()+1,index3-index2-when.size());
                condition_time_pair.second = when;
                sub_conditions.push_back(condition_time_pair);
            }
            else if(sub_condition.find("over all")!=std::string::npos)
            {
                when = "over all";
                index2 = sub_condition.find(when);
                index3 = sub_condition.find(")");
                if(sub_condition.find("not")!=std::string::npos)
                    index3++;
                condition_time_pair.first = sub_condition.substr(index2+when.size()+1,index3-index2-when.size());
                condition_time_pair.second = when;
                sub_conditions.push_back(condition_time_pair);
            }
            else if(sub_condition.find("at start")!=std::string::npos)
            {
                when = "at start";
                index2 = sub_condition.find(when);
                index3 = sub_condition.find(")");
                if(sub_condition.find("not")!=std::string::npos)
                    index3++;
                condition_time_pair.first = sub_condition.substr(index2+when.size()+1,index3-index2-when.size());
                condition_time_pair.second = when;
                sub_conditions.push_back(condition_time_pair);
            }
            else
            {
                //TODO this might cause some issues, as I change that literals have "at start"
                when = ""; //at start
                index2 = -1;
                condition_time_pair.first = sub_condition.substr(index2+when.size()+1,sub_condition.size()-1-index2-when.size());
                condition_time_pair.second = when;
                sub_conditions.push_back(condition_time_pair);
            }
            index3=0;
            index3 = condition_time_pair.first.find("(",index3);
            int num_brackets=0;
            while(index3!=std::string::npos)
            {
                num_brackets++;
                index3 = condition_time_pair.first.find("(",index3+1);
            }
            index3=0;
            index3 = condition_time_pair.first.find(")",index3);
            while(index3!=std::string::npos)
            {
                num_brackets--;
                index3 = condition_time_pair.first.find(")",index3+1);
            }
            if(num_brackets!=0)
            {
                std::cerr << "getSubConditions:ERROR: wrong number of brackets\n";
                throw;
            }
            if((condition_time_pair.first.substr(0,1)!="(")||(condition_time_pair.first.substr(condition_time_pair.first.size()-1,1)!=")"))
            {
                std::cerr << "getSubConditions:ERROR: wrong extracted conditions in problem\n";
                throw;
            }
        }
    return sub_conditions;
}

/* 05/09 checked
 * checking if the condition literals holds in the current state
 * condition =  the condition to be checked
 * when = reffers to "" (at start), "at_end", "over all", or "all" - in this case, time is ignored
 * domain is domain where to test predicate
 */
static int areConditionsSatisfiedNow(std::string condition, std::string when, std::string domain)
{
    std::vector<std::pair<std::string,std::string> > sub_condis = getSubConditions(condition);

    int predicateSatisfied = 1;
    for(unsigned int j=0;j<sub_condis.size();j++)
    {
        if(when != "all")
        {
            if(when != sub_condis.at(j).second)
            {
                continue;
            }
        }

        unsigned int index_c = std::distance(current_state[domain].begin(),std::find(current_state[domain].begin(),current_state[domain].end(),sub_condis.at(j).first));

        if (index_c == current_state[domain].size()) //nothing found
        {
          predicateSatisfied*= 0; //this goal is not yet satisfied
          break; //we do not have to check others
        }
    }
    return predicateSatisfied;
}

/* 05/09 checked
 * this method add the causal link into set of active_links
 * if the link isnt there already
 * link = a link to add
 * active_links = the set of links to be merged
 * task = the task to which the link belongs
 * return true - when a link was added, otherwise false
*/
static bool addLink(std::shared_ptr<MyLink> link, std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links,
                    std::shared_ptr<Task> task)
{
    //we need to check if the link is not in the active list
    bool sameLink=false;
    bool conditionSat = false;
    for(unsigned int k=0; k<active_links->size();k++)
    {
        if(active_links->at(k)->first == link)
        {
            sameLink = true;
            break;
        }
    }
    if(areConditionsSatisfiedNow(link->getCondition(),"all",task->getDomain())==1)
    {
        conditionSat = true;

    }
    if((!sameLink)&&(!conditionSat))
    {
      std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> >> tl(new std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> >(link,task));
      active_links->push_back(tl);
      return true;

    }
    return false;
}

/*
 * back - backtrack value
 * active_t_id - id of task from active_list
 * active_s_id - id of step from active_list
 * adding_t_id - id of adding taks
 * adding_s_id - id of adding step
 */

static int isBacktracked(bool back, int active_t_id, int active_s_id, int adding_t_id, int adding_s_id)
{
    int ret;
    if(back)
    {
        similar_actions[std::pair<int,int>(adding_t_id,adding_s_id)] = std::pair<int, int>(active_t_id,active_s_id);
        ret=0; //we want to add the new action to the list
    }
    else
    {
       similar_actions[std::pair<int,int>(active_t_id,active_s_id)] = std::pair<int, int>(adding_t_id,adding_s_id);
       ret=1; //dont add it
    }
    return ret;
}






//to correctly test if effects of action are already merged, ie, we can skip that action
static bool removeOppositeEffects(std::vector<std::pair<std::string,std::string> > sub_condis, std::string domain)
{

    std::vector<std::string > not_act_start;
    std::vector<std::string > act_start;
    std::vector<std::string > not_act_end;
    std::vector<std::string > act_end;

    for (unsigned int i=0; i < sub_condis.size(); i++)
    {
        if(sub_condis.at(i).first.find("not")!=std::string::npos)
        {
           if(sub_condis.at(i).second=="")
           {
             not_act_start.push_back(sub_condis.at(i).first.substr(5,sub_condis.at(i).first.length()-5-1));
           }
           else
           {
               not_act_end.push_back(sub_condis.at(i).first.substr(5,sub_condis.at(i).first.length()-5-1));
           }
        }
        else
        {
            if(sub_condis.at(i).second=="")
              act_start.push_back(sub_condis.at(i).first);
            else
              act_end.push_back(sub_condis.at(i).first);
        }
    }
    //sort(not_act.begin(), not_act.end());
    //sort(act.begin(),act.end());

    //this could be done smarter by comparing the beggining of strings using the sort

    bool negFound = false;
    bool satisfied = true;
    for (unsigned int i=0; i < act_start.size();i++)
    {
        for (unsigned int j=0; j < not_act_end.size(); j++)
        {
          if(act_start.at(i) == not_act_end.at(j))
          {
              negFound = true;
              break;
          }
        }
        if(negFound)//use only the end effect
        {
          //erase start effect
          act_start.erase(act_start.begin()+i);
          i--; //as we changed the size
        }
        else
        {
            if(areConditionsSatisfiedNow(act_start.at(i), "", domain)==0)
            {
                satisfied = false;
                return satisfied;
                break;
            }

        }
    }
    //test end effects
    for (unsigned int j=0; j <not_act_end.size();j++)
    {
        if(areConditionsSatisfiedNow("(not " + not_act_end.at(j)+")", "", domain)==0)
        {
            satisfied = false;
            return satisfied;
            break;
        }
    }

    negFound = false;
    for (unsigned int i=0; i < not_act_start.size();i++)
    {
        for (unsigned int j=0; j < act_end.size(); j++)
        {
          if(act_end.at(j) == not_act_start.at(i))
          {
              negFound = true;
              break;
          }
        }
        if(negFound)//use only the end effect
        {
          //erase start effect
          not_act_start.erase(not_act_start.begin()+i);
          i--; //as we changed the size
        }
        else
        {
            if(areConditionsSatisfiedNow("(not " + not_act_start.at(i)+")", "", domain)==0)
            {
                satisfied = false;
                return satisfied;
                break;
            }

        }
    }
    //test end effects
    for (unsigned int j=0; j <act_end.size();j++)
    {
        if(areConditionsSatisfiedNow(act_end.at(j), "", domain)==0)
        {
            satisfied = false;
            return satisfied;
            break;
        }
    }

    return satisfied;

}


/* 05/09 checked
 * this method check if the tested step is already in the active_list
 * moreover it checks if the newly added action does not have same effects as one already in the active_list
 * or which effects are already satisfied
 * active_list = list of steps already queueing for merging
 * step = a tested new step if it is contained in active_list or not
 *
 * return = 0 steps are different
 * return = 1 step is already presented in the activelist
 * return = -1 steps effects are satisfied in the current state
 */
int compareSteps(std::shared_ptr<std::vector< std::shared_ptr<StepTimed> > > active_list, std::pair<std::shared_ptr<MyGroundAction>, int > step, std::shared_ptr<Task> task)
{
    int ret = 0;
    int  taskId = task->getID();

    std::stringstream ss;
    std::ostream &os(ss);
    step.first->printName(os);
    std::string action = ss.str();

    for(unsigned int i=0;i<active_list->size();i++)
    {
        std::stringstream ss1;
        std::ostream &os1(ss1);
        active_list->at(i)->getStep()->printName(os1);
        std::string action2 =ss1.str();

        if(action == action2) //if actions are same test if the check came from same task, ie it is the same action
            //or from other task, it is just similar action
        {
            if(taskId != active_list->at(i)->getTask()->getID())
            {
                //if it is from other task, which action should go to similar actions?
                //if the active_list action has backtrack flag, we want to overwrite it with the new one
                ret = isBacktracked(active_list->at(i)->getBacktrack(),active_list->at(i)->getTask()->getID(), active_list->at(i)->getStep()->getId(), taskId, step.first->getId());
                if(ret==0)
                {
                    active_list->erase(active_list->begin()+i);
                    i--;
                }
            }
            else //if they are same
            {
                //I need to check the temporary task, the ids can be same by coincidence (as Iam deleting the temporary tasks)
                if(taskId < 0)
                {
                    if(task->getParentTask()==active_list->at(i)->getTask()->getParentTask()) //it is action from the same task
                    {
                        if(task->getParentStepId()==active_list->at(i)->getTask()->getParentStepId())
                        {
                            //it is even from the same step
                            ret =1; //dont add
                        }
                        else //different step
                        {
                            ret = isBacktracked(active_list->at(i)->getBacktrack(),active_list->at(i)->getTask()->getID(), active_list->at(i)->getStep()->getId(), taskId, step.first->getId());
                            if(ret==0)
                            {
                                active_list->erase(active_list->begin()+i);
                                i--;

                            }
                        }
                    }
                    else //different tasks
                    {
                        ret = isBacktracked(active_list->at(i)->getBacktrack(),active_list->at(i)->getTask()->getID(), active_list->at(i)->getStep()->getId(), taskId, step.first->getId());
                        if(ret==0)
                        {
                            active_list->erase(active_list->begin()+i);
                            i--;

                        }
                    }
                }
                ret = 1;
            }
            break;
        }
    }

    //check if Step needs to be added, maybe its effects are already satisfied

         std::string domain = task->getDomain();
         //time, predicate parameters end brackets
         std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effL = step.first->getEffects();

         std::vector<std::pair<std::string,std::string> > actions;
         for(unsigned int j=0; j<effL.size();j++)
         {
             std::string literal;
             literal = effL.at(j).first.first.second;
             for(unsigned int k=0; k< effL.at(j).first.second.size();k++)
             {
                 literal += " ";
                 literal += step.first->getParameters().at(effL.at(j).first.second.at(k));
             }
             literal += effL.at(j).second;
             std::string action = literal;

             if(effL.at(j).first.first.first=="at start")
             {
                 std::pair<std::string,std::string> pair;
                 pair.first = action;
                 pair.second = ""; //start
                 actions.push_back(pair);
             }
             else
             {
                 std::pair<std::string,std::string> pair;
                 pair.first = action;
                 pair.second = "end"; //start
                 actions.push_back(pair);
             }

         }
         //all effects of action must hold, unless some effects are opposite each other, then end effects must hold
         if(removeOppositeEffects(actions,domain)) //we can remove it
         {
            //dont add Step, all effects are valid
            ret = -1;
            //mark in startVector as satisfied
            step.second = 1;
         }



    return ret;
}

/*05/09 checked */
/* This method reasons over the structure of the input plan for a task
 * and adds actions with satisfied producing links into active_list
 * or add the not satisfied links into active_links
 * task = a task which plan is being tested
 * active_list = the queue for step which can be merged
 * active_links = the queue for links which can be merged
 */
static void addSteps(std::shared_ptr<Task> task, std::shared_ptr<std::vector< std::shared_ptr<StepTimed> > > active_list,
                     std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links)
{

    std::vector<size_t> ids; //ids to add this time

    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > >::iterator it;
    //find such an action which has not been yet used
    std::shared_ptr<MyPlan> plan = task->getPlan();
    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > steps = plan->getSteps();
    std::vector<std::pair<std::shared_ptr<MyLink>, int > >  links = task->getPlan()->getLinks();


    it = std::find_if(steps.begin(),steps.end(),findUsed);

    size_t previousStep = std::numeric_limits<std::size_t>::max();

    while(it != steps.end()) //we found some step
    {

        size_t id = it->first->getId();


        /*------------------------------------------------------*/
        /* checking orderings */
        /*------------------------------------------------------*/

        //this part here is making sure, that only actions which can coexists in parallel are chosen
        //if there are two action with no link between them, they still might need to be ordered
        if(previousStep != std::numeric_limits<std::size_t>::max())
        {
           //check if this step can be before or in parallel with the previous step
           if(plan->getOrderings()->possibly_not_after(id, StepTime::AT_START,previousStep, StepTime::AT_END))
           //if yes - check with the action before, etc...
           {
               //int indexB = start_map[previousStep];
               int indexB = previousStep;

               if(indexB == 0) //it is the first action, there is nothing else before it
               {
                   previousStep = std::numeric_limits<std::size_t>::max();
               }
               else
               {
                 for(int q=indexB;q>=0;q--)
                 {
                    //if it cant be before, save the step
                    if(!(plan->getOrderings()->possibly_not_after(id, StepTime::AT_START,steps.at(q).first->getId(), StepTime::AT_END)))
                    {
                      previousStep = steps.at(q).first->getId();
                      break;
                    }
                 }
               }

           }

           //is the previous step (which must be ordered before this) merged? if yes, continue normally, if not, I cannot add this step or any link with it
          if(previousStep != std::numeric_limits<std::size_t>::max()) //if the previous step is different that total start
          {
               std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > >::iterator it2;
               //TODO should be here findUsed or findHalfUsed
               it2 = std::find_if(steps.begin(),steps.end(),findUsed);
               bool found = false;
               while(it2 != steps.end())
               {
                   //we found the step but it hasnt been merged yet, hence I cannot add this step or any link with it
                   if(it2->first->getId() == previousStep)
                   {
                       found = true;
                       break;
                   }
                   it2 = std::find_if(it2+1,steps.end(),findUsed);
               }
               if(found)
               {
                   previousStep = id;
                   it = std::find_if(it+1,steps.end(),findUsed);
                   continue; //loop the main while loop
               }
               //if this end is reached it means that the previousStep id is merged, we can continue in adding this step
           }
        }

        /*--------------------------------------------------------------*/
        /* checking the links */
        /*--------------------------------------------------------------*/

        //if the step is not in to_id link, we want to add it
        //else if the from_id  not used, skip it

        bool linkFound = false;

        for(unsigned int j=0; j < links.size(); j++)
        {
            size_t toID = links.at(j).first->getToId();
            size_t fromID = links.at(j).first->getFromId();


            if(( toID == id)&&(links.at(j).second==-1))
            { //if the id is in link which is not yet satisfied

                if(fromID==0) //the first init action
                {
                    //the link is not satisfied yet, the producer is the init state
                    //hence something deleted the satisfied link and we need to
                    //satisfy it again =>
                    //add the Link

                    linkFound=addLink(links.at(j).first,active_links, task);
                    if(!linkFound)
                    {
                        //either it was already in active links or it is satisfied
                        if(areConditionsSatisfiedNow(links.at(j).first->getCondition(),"all",task->getDomain())==1)
                        {
                           links.at(j).second = 1;
                        }
                    }
                }
                else
                {
                    //is the action fromID satisfied? - all of it
                    //int indexL = start_map[fromID];
                    int indexL = fromID-1;
                    if(steps.at(indexL).second== 1) //if the action FROM is satisfied add the link
                    {

                        linkFound = addLink(links.at(j).first,active_links,task);
                        if(!linkFound)
                        {
                            //either it was already in active links or it is satisfied
                            if(areConditionsSatisfiedNow(links.at(j).first->getCondition(),"all",task->getDomain())==1)
                            {
                               links.at(j).second = 1;
                            }
                        }
                    }
                    else
                    {
                        //start has been merged
                        if(steps.at(indexL).second== 0)
                        {
                           //if the link is from AT_START, I can add the link, as the FROM is merged
                           if(links.at(j).first->getEffectTime()=="at start")
                           {

                              linkFound = addLink(links.at(j).first,active_links,task);
                              if(!linkFound)
                              {
                                  //either it was already in active links or it is satisfied
                                  if(areConditionsSatisfiedNow(links.at(j).first->getCondition(),"all",task->getDomain())==1)
                                  {
                                     links.at(j).second = 1;
                                  }
                              }
                           }
                           else //it is to the end,
                           {
                              //actually test, if how is the TO action involved by start, or end or both?
                              if(links.at(j).first->getConditionTime()=="at end")
                              {
                                  //check the start vector if the TO action is start, or end now
                                   //int indexTO = start_map[toID];
                                   int indexTO = toID;
                                   //nothing has been merged
                                   if(steps.at(indexTO).second== -1)
                                   {
                                       //as this condition is limited to the end of this action
                                       //I can add start
                                       linkFound= false;
                                   }
                                   else //the start of TO has been merged
                                   {
                                       //in this case we know that it is link end->end
                                       //but end of FROM_id is not merged in this else
                                       //hence do not add link but do not add the action either
                                       linkFound = true;
                                   }
                              }
                              else //the TO action is involved by start or by overall
                              {
                                  //so we know it is link end -> start, but end is not merged here
                                  //do nothing
                                  linkFound =true;
                              }
                           }
                        }
                        else //nothing of the FROM action has been merged
                        {
                            //but this links requires the end
                            if(links.at(j).first->getEffectTime()=="at end")
                            {
                                linkFound = false;
                            }
                            else
                            {
                                linkFound= true;
                            }

                        }
                    }


                }

                if(linkFound)
                  break; //we found a link, we dont have to search more
            }
        }
        if(!linkFound) //no link is violating the action, we can add it
        {
          ids.push_back(id);
        }
        //search for another action
        previousStep = id;
        it = std::find_if(it+1,steps.end(),findUsed);

    }

    //set the set links back to the plan
    task->getPlan()->setLinks(links);

    /*-----------------------------------------------------*/
    /*This part here is related to the relaxation of the input plans
     * it might happen that some goal literal is in the initial state of the relaxed problem
     * hence the action is not included in the input plan
     * but for the final plan, we need to satisfy this goal literal too, so we add the link*/
    /*-----------------------------------------------------*/


    //check if all the actions are done, and links not leading to goal are done,
    //you want to add links leading to goal
    it = std::find_if(steps.begin(),steps.end(),findHalfUsed);
    if(it==steps.end())
    {
        for(unsigned int x=0;x<links.size();x++)
        {

            //if it is not satisfied and it is not leading to the goal state
           if((links.at(x).second==-1)&&(links.at(x).first->getToId()!=Plan::GOAL_ID))
           {
               //if we have some unsatisfied link which is "in the middle" of the plan
               //it shows on the malfunction of the algorithm
               std::cerr << "Unsatisfied links in the input plans when all actions are merged";
               throw;
           }
           else if ((links.at(x).second==-1)&&(links.at(x).first->getToId()==Plan::GOAL_ID))
           {
               //add this link that it can be finally satisfied
               addLink(links.at(x).first,active_links,task);
           }

        }

    }

    /*-----------------------------------------------*/
    /* add those actions which satisfied limits on orderings and links*/
    /*-----------------------------------------------*/

    //09/09 new
    //the algoritghm above test actions in the order of their schedule
    //hence the first id corelates to the first action which can appear
    //thus Id like to loop throw the ids and add them in the same order

    bool callAgain = false;
    for(unsigned int i = 0; i< ids.size(); i++)
    {
        std::pair<std::shared_ptr<MyGroundAction>, int > step = steps.at(ids.at(i)-1);

        if(step.first->getId() != ids.at(i))
        {
            std::cerr << "addsteps::ERROR: ids not matching\n";
            throw;
        }
        int ret = compareSteps(active_list,step, task);
        if(ret==0) //add the step and the step is not yet in the active_list
        {

            std::shared_ptr<StepTimed> s_p(new StepTimed(step.first, task));
            active_list->push_back(s_p);
        }
        else if(ret==-1) //we want to call addStep again as the action had its effects satisfied
        {
            callAgain=true;
        }

    }

    if(callAgain)
    {
        addSteps(task, active_list,active_links);
    }

    //deleting active_links - those whose from_id is not satisfied
    //or those who are satisfied in the current_state
    std::vector<int> different_tasks;
    //task_id, order_id in the active_links
    std::vector<std::pair<int, int> > links_pair;
    std::vector<int> links_to_delete;
    for(unsigned int i=0; i < active_links->size();i++)
    {
        //if there are more links from the same task, test if the to_ids can co-exists in parallel

        std::vector<int>::iterator it = std::find(different_tasks.begin(),different_tasks.end(),active_links->at(i)->second->getID());
        if(it !=different_tasks.end())
        {
            //id already found
            int l1 = links_pair.at(std::distance(different_tasks.begin(),it)).second;
            size_t to_id1= active_links->at(l1)->first->getToId();
            size_t to_id2 = active_links->at(i)->first->getToId();
            //can both to_ids coexist in parallel?
            // first step could be ordered after the second step
            //TODO what step times?
            if(active_links->at(i)->second->getPlan()->getOrderings()->possibly_after(to_id1, StepTime::AT_START,to_id2, StepTime::AT_START))
            {
                //swap ids
                //if not, delete a link
                if(!active_links->at(i)->second->getPlan()->getOrderings()->possibly_after(to_id2, StepTime::AT_START,to_id1, StepTime::AT_START))
                {
                    //the second link must go first, hence delete the first one
                    links_to_delete.push_back(l1);
                    //just replace the link_pair
                    links_pair.at(l1).second = i;
                }
            }
            else
            {
                //delete second one
                links_to_delete.push_back(i);
            }
        }
        else
        {
            different_tasks.push_back(active_links->at(i)->second->getID());
            links_pair.push_back(std::pair<int,int>(active_links->at(i)->second->getID(),i));
        }
    }


       if(links_to_delete.size()>0)
       {
           std::sort(links_to_delete.begin(),links_to_delete.end());
           for(unsigned int i = 0;i<active_links->size();i++)
           {
               if(links_to_delete.size()==0)
                  break;

               if(i == links_to_delete.at(0))
               {
                   //descrese indexed
                   std::transform (links_to_delete.begin(), links_to_delete.end(), links_to_delete.begin(), bind2nd(std::plus<int>(), -1));
                   //delete all indexes which are smaller than the current index
                   while(links_to_delete.at(0)<i)
                   {
                       links_to_delete.erase(links_to_delete.begin());
                       if(links_to_delete.size()==0)
                           break;
                   }

                   active_links->erase(active_links->begin()+i);
                   i--;
               }
           }
       }
}

/* 05/09 checked
 * this method tests if the task's time window is open
 * if yes, it calls addStep to add new actions and links
 * init_time = time now
 * active_list = list of steps to be merged
 * active_links = list of links to be merged
 */
static void checkActiveActions(double init_time, std::shared_ptr<std::vector< std::shared_ptr<StepTimed> > > active_list,
                               std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links)
{

    for (unsigned int i=0; i< tasks->size();i++)
    {
       if (tasks->at(i)->getReleaseDate() <= init_time) //the time window for the tasks is opened
       {

          addSteps(tasks->at(i),active_list,active_links);

       }
    }

}

/* 05/09 checked
 * this method provides one string with the current state
 * domain = a domain for which state should be obtained
 */
static std::string writeInitialState(std::string domain)
{
    std::string output="";
    for (unsigned int i=0;i<current_state[domain].size();i++)
    {
      output += "    ";
      output += current_state[domain].at(i);
      output += "\n";
    }
    output += ")\n";
    return output;
}

/* 05/09 checked
 * writing a temporal problem to the file
 * origin = an original task from which the action with unsatisfied condition came
 * action = the unsatisfied condition
 * output are two names of the temporal problems
*/
static std::vector<std::string> createTempProblem(std::shared_ptr<Task> origin, std::string action)
{

    std::string problemName = origin->getProblem();

    size_t index = problemName.find("problem.pddl");

    std::string line;
    std::ofstream file_out;
    std::ifstream file_in;
    std::string filename = problemName.substr(0,int(index)) + "temp_problem.pddl";//\0";
    std::string probname;

    //splitting action if and, getting at end, over all away
    std::vector<std::pair<std::string,std::string>> sub_condis = getSubConditions(action);

    file_out.open ((filename).c_str());

    //copying settings of the problem file
    file_in.open(problemName.c_str());
    int h = 0;
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
           if (h==0) //first line
           {
               h++;
               line = line.substr(0,line.length()-1)+"_temp"+std::to_string(g_temp_prob) + ")";
               g_temp_prob++;
               probname = line;
           }
           if(line.find("(:init") == std::numeric_limits<std::size_t>::max()) //copy
             file_out << line << "\n";
           else
             break; //stop
        }
    }
    file_in.close();
    file_out << "(:init\n";
    file_out << writeInitialState(origin->getDomain());
    file_out << "(:goal (and\n";


    for(unsigned int x=0;x<sub_condis.size();x++)
    {
      file_out<< "    "  << sub_condis.at(x).first << "\n";
    }

    file_out << ")))\n";
    file_out.close();

    probname = probname.substr(17,probname.length()-1);
    probname = probname.substr(0,probname.length()-1)+"\0";

    std::vector<std::string> names;
    names.push_back(filename);
    names.push_back(probname);

    return names;
}

//TODO move this method to tasks, when I will make current state accesible
//TODO why I use this method only at the beginning, why I dont reuse it?
/* 05/09 checked
 * this method is called in order to initialise which links are satisfied at the beginning
 * task = the task to make the analysis
 */
static void updateTaskLinks(std::shared_ptr<Task> task)
{
    //check is satisfied action hasnt satisfied also some link
    std::vector<std::pair<std::shared_ptr<MyLink> ,int> > links;
    links = task->getPlan()->getLinks();

    for(unsigned int k=0;k<links.size();k++)
    {
      std::string literal = links.at(k).first->getCondition();

      //check only those actions, which are from_id = 0(ie initial state)
      if(links.at(k).first->getFromId()==0)
      {
          //here changed on 3.10.
          if(areConditionsSatisfiedNow(literal, "all", task->getDomain())==1)
          {
             links.at(k).second = 1;
          }
       }
    }
    task->getPlan()->setLinks(links);

}

/* 05/09 checked
 * this method call the planner and creates the temporal task
 * origin = the original task for which unsatisfied precondition we plan
 * action = the not satisfied condition for which we need to plan
 * do NOT delete pointer to origin, you dont own it
 */
static void planTempTask(std::shared_ptr<Task> origin, std::string action, int step_id)
{
    std::vector<std::string> names = createTempProblem(origin, action);
    std::string problemName = names.at(0);


    std::shared_ptr<Task> temp(new Task(-(temp_tasks->size()+1),origin->getReleaseDate(),origin->getDeadline(),origin->getDomain(),problemName,names.at(1)));
    temp->setParentTask(origin->getID());
    temp->setParentStepId(step_id);
    char * word;

    std::string output = problemName+"out";
    word = new char[output.length()+1]; //+1 to copy also null char in the end
    std::strcpy(word,output.c_str());
    g_argv[g_argc-1] = word;

    word = new char[problemName.length()+1]; //+1 to copy also null char in the end
    std::strcpy(word,problemName.c_str());
    g_argv[g_argc-2] = word;


    word = new char[origin->getDomain().length()+1]; //+1 to copy also null char in the end
    std::strcpy(word,origin->getDomain().c_str());
    g_argv[g_argc-3] = word;

    temp->callPlanner(g_argc,g_argv);
    //05/09 added - to just check the links, if the plan exist
    if(temp->getPlanAvailable())
    {
      updateTaskLinks(temp);

    }
    if(temp->getPlan()->getVirMemory() > g_vir_memory )
       g_vir_memory = temp->getPlan()->getVirMemory();
    if(temp->getPlan()->getPhyMemory() > g_phy_memory)
       g_phy_memory = temp->getPlan()->getPhyMemory();


    temp_tasks->push_back(temp);

    std::string command = "rm "+problemName;
    system(command.c_str());
}


/* 05/09 checked
 * this method check preconditions of a step, if there are not satisfied it plans for it
 * step_p = step to check
 * when = "" (for at start) or "at end"
 * plan = shall we plan if the preconds are not satisfied?
 * return value:
 * 0 no extra things needed
 * 1 extra actions needed to satisfy condition
 * -1 plan not existing
*/
static int checkPreconditions(std::shared_ptr<StepTimed> step_p, std::string when, bool plan)
{

    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = step_p->getStep()->getConditions();

    std::string literal = "(and ";

    //in the literal are only those conditions for which I care depending on when -> hence, I can omit their time info such as "overall" etc
    for(unsigned int i=0;i < condi.size();i++)
    {
        if((when=="")&&(condi.at(i).first.first.first=="at end"))
            continue;

        if((when=="at end")&&(condi.at(i).first.first.first=="at start"))
            continue;


        literal += condi.at(i).first.first.second;
        for(unsigned int j=0; j< condi.at(i).first.second.size();j++)
        {
            literal += " ";
            literal += step_p->getStep()->getParameters().at(condi.at(i).first.second.at(j));

        }
        literal += condi.at(i).second;
    }
    literal+= ")";

        std::string domain = step_p->getTask()->getDomain();

        if ( areConditionsSatisfiedNow(literal,"all", domain) == 1)
        {
            temp_tasks->push_back(std::shared_ptr<Task>()); // empty task, to keep correct order
            return 0; //we do not need add anything
        }
        else //not satisfied, we need to plan
        {
            if(plan)
            {
                //step_p->getTask()->getID()
                planTempTask(step_p->getTask(),literal,step_p->getStep()->getId());
                if(temp_tasks->at(temp_tasks->size()-1)->getPlanAvailable())
                    return 1;
                else
                    return -1;
            }
            else
            {
                return 1; //there are not satisfied
            }


    }
 }
//TODO test this method! (probably dummy3 problem)

//returns those conditions, what are satisfied by test action
static std::vector<std::pair<std::string,std::string>> checkEffects(std::shared_ptr<StepTimed> step_p, std::string when, std::shared_ptr<StepTimed> test )
{

    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = step_p->getStep()->getConditions();

    std::string literal = "(and ";

    for(unsigned int i=0;i < condi.size();i++)
    {
        if((when=="")&&(condi.at(i).first.first.first=="at end"))
            continue;

        if((when=="at end")&&(condi.at(i).first.first.first=="at start"))
            continue;


        literal += condi.at(i).first.first.second;
        for(unsigned int j=0; j< condi.at(i).first.second.size();j++)
        {
            literal += " ";
            literal += step_p->getStep()->getParameters().at(condi.at(i).first.second.at(j));

        }
        literal += condi.at(i).second;
    }
    literal+= ")";

    std::vector<std::pair<std::string,std::string> > sub_condis = getSubConditions(literal);

    std::vector<std::pair<std::string,std::string>> unsatis_cond;

    for(unsigned int j=0;j<sub_condis.size();j++)
    {
        /*if(when != sub_condis.at(j).second)
        {
            continue;
        }*/

        std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effects = test->getStep()->getEffects();



        bool found =false;

        for (unsigned int i =0; i< effects.size();i ++)
        {
            //use only those effect which rely if we are merge start or end of the action
            if((test->getEndTime()==-1)&&(effects.at(i).first.first.first != "at start"))
            //we want start effects, but this is not one, do nothing
            {
                continue;
            }
            if((test->getEndTime()!=-1)&&(effects.at(i).first.first.first != "at end"))
            //we want end effects, but this is not one, do nothing
            {
                continue;
            }


            std::string action = effects.at(i).first.first.second;
            for(unsigned int j=0; j< effects.at(i).first.second.size();j++)
            {
                action += " ";
                action += step_p->getStep()->getParameters().at(effects.at(i).first.second.at(j));

            }
            action += effects.at(i).second;

            if(sub_condis.at(j).first == action)
            {
                found=true;
                break;
            }
       }
       if(!found)
       {
           unsatis_cond.push_back(sub_condis.at(j));
       }

   }
   return unsatis_cond;

}

/* 05/09 checked
 * this method check the conditions of the link, if it is not satisfied it plans for it
 * link_p - the link with the related task
 * return value:
 * 0 no extra things needed
 * 1 extra actions needed to satisfy condition
 * -1 plan not existing
*/
static int checkLinks(std::shared_ptr<std::pair<std::shared_ptr<MyLink>, std::shared_ptr<Task> > > link_p)
{


    std::string domain = link_p->second->getDomain();
    std::string condition = link_p->first->getCondition();


    //TODO if the links are those which condition has NOT been satisfied
    //why am I even trying to check it again, I should plan immediately
    if(std::find(current_state[domain].begin(),current_state[domain].end(),condition) != current_state[domain].end()) //something was found
    {
        temp_tasks->push_back(std::shared_ptr<Task>()); // empty task, to keep correct order
        return 0; //we do not need add anything
    }
    else
    {
        planTempTask(link_p->second,condition,link_p->first->getToId());
        //get the last temp tasks -> just added
        if(temp_tasks->at(temp_tasks->size()-1)->getPlanAvailable())
        {
            return 1;
        }
        else
            return -1; //plan was not found

    }
}

/* 05/09 checked
 * this method search for threaten links (those overall from unfinished actions in the final plan
 * and checks if the literal lit (effect of an action to be merged) can threaten it or not
 * realPlan = the final plan where we are merging
 * lit = the effect condition of an action - must be atom or negation already
 * active_id = the id of the action being checked
 * lit_id = id of the literal being checked - used to printout
 * bind = bindings - used to printout
 * return 0 if the literal threatens, 1 if not
 */
static int checkLiteral(std::shared_ptr<FinalPlan> realPlan, std::string effect, size_t active_id)
{
    std::shared_ptr<std::vector<std::shared_ptr<MyLink> > > vecL = realPlan->getLinks(); //dont delete thhis pointer as it is not owner

    std::vector<std::shared_ptr<MyLink> >::iterator it = std::find_if(vecL->begin(), vecL->end(),canBeThreaten);

    int notThreating = 1;
    while (it != vecL->end()) //we found some link which might be threaten
    {

        //TODO I need to test this, I think to include this if is correct, but I wonder
        //how it could work without it
        if((*it)->getToId() == active_id) //if active_id is different than -1, it is from not finished action
        //this makes sure that action doesnt threat itself :)
        {
           break; //TODO shouldnt I test the rest of the links too? ie, continue + change in iterator
        }

        //TODO do I need this step? It feels it will be always there
        //find the step in the realPlan with ID = to_id - but to_id is "global",
        std::shared_ptr<StepTimed> stepRealPlan = realPlan->getStep((*it)->getToId());
        if (stepRealPlan.get() == NULL) //step with such an id hasnt been found
        {
          std::cerr << "main::checkLiteral:ERROR: the step where the link leads was not found\n";
          throw;
        }

        bool cond_not = false;
        bool effe_not = false;
        std::string condition = (*it)->getCondition();
        //the effect and condition must be opposite, check for not

        size_t index = condition.find("not");
        if(index!= std::string::npos) //found
        {
           index = condition.find("(",index);//to skip not
           condition = condition.substr(index,condition.size()-index-1); //-1 not to take last bracket
           cond_not = true;
        }

        index = effect.find("not");
        if(index!= std::string::npos) //found
        {
            index = effect.find("(",index);//to skip not
            effect = effect.substr(index,effect.size()-index-1); //-1 not to take last bracket
            effe_not = true;
        }

        //only if they are opposite
        if(((cond_not)&&(!effe_not))||((!cond_not)&&(effe_not)))
        {
            if(effect == condition)
            {
                notThreating = 0;
                break;
            }
        }
        //check other links
        it = std::find_if(it+1, vecL->end(),canBeThreaten);
     }

    return notThreating;

}

/* 05/09 checked
 * this method gets the effect literal of the action which should be chosen to be merged
 * and test if the literals are not threatening any existing overall link
 * realPlan = the final plan
 * active_list_p = the queue of actions to be merged
 * index = the order of the action in the queue
 */
static bool threatsLinks(std::shared_ptr<FinalPlan> realPlan,
                         std::shared_ptr<std::vector< std::shared_ptr<StepTimed> > > active_list_p,
                         size_t index)
{

    size_t active_id;
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effL;

    if (index>=active_list_p->size()) //check comes from the links
    {
        return false; //link cannot threats another links, it is the action
    }
    else //check comes from action
    {

        effL = active_list_p->at(index)->getStep()->getEffects();

        if(active_list_p->at(index)->getEndTime()!=-1) //end action
        {
            std::map< std::shared_ptr<StepTimed>, size_t>::iterator it = realPlan->unfinished_actions.find(active_list_p->at(index));
            if (it != realPlan->unfinished_actions.end()) //found something
            {
               active_id = it->second;
            }
        }
        else
        {
            active_id = -1; //TODO which id here? //active_list_p->at(index)->getStep()->id();
        }
    }


    int isFormulaSaved = 1;

    for(unsigned int i =0; i< effL.size();i++)
    {
        std::string literal="";

        literal += effL.at(i).first.first.second;
        for(unsigned int j=0; j< effL.at(i).first.second.size();j++)
        {
            literal += " ";
            literal += active_list_p->at(index)->getStep()->getParameters().at(effL.at(i).first.second.at(j));

        }
        literal += effL.at(i).second;
        //TODO test if literal is correct

        isFormulaSaved *= checkLiteral(realPlan, literal,active_id);
    }

    if (isFormulaSaved == 1)//none literal breaks it
       return false;
    else
       return true;
}

/*09/09 check
 * needs to solve backtracking still
 * this method choose one action to be merged
 * realPlan = the final plan
 * active_list_p = the queue of actions
 * backtrack_a = the last step which failed
 * active_links = the queue of links
 * timeTaken = current times
 * result = a pointer on a step to be merged or NULL
 */
static std::shared_ptr<StepTimed> getAction(std::shared_ptr<FinalPlan> realPlan,
                                            std::shared_ptr<std::vector< std::shared_ptr<StepTimed> > > active_list_p,
                                              std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links,
                                            double * timeTaken, std::shared_ptr<std::vector<int> > back_ids )
{


    std::vector<float> time_requir;

    std::vector<int> precond_needed(active_list_p->size()+active_links->size());
    std::vector<int> choose_instead(active_list_p->size()+active_links->size(),-1);


    /* precond_needed can be
     * 4 - end action needs a plan to satisfy its precond, plan found
     * 3 - previously not useful, skip it
     * 2 - came from backtrack
     * 1 - plan found
     * 0 - nothing needed
     * -1 - plan NOT found
     * -4 - end action needs a plan to satisfy its precond, plan not found
     */
    //check preconditions of the active actions
    for(unsigned int i=0; i<active_list_p->size(); i++)
    {

        if(previously_not_useful.size()>0)
        {
            std::vector<int>::iterator notu_ind = std::find(previously_not_useful.begin(),previously_not_useful.end(),i);
            if(notu_ind!=previously_not_useful.end()) //index found
            {
                //this id lead to returned NULL previously, we dont want to choose it again
                precond_needed.at(i)= 3; //skip this one
                time_requir.push_back(INT_MAX);
                temp_tasks->push_back(std::shared_ptr<Task>());
                continue;
            }
        }

        if(active_list_p->at(i)->getBacktrack())
        {
            precond_needed.at(i) = 2; //as backtrack
            time_requir.push_back(INT_MAX);
            temp_tasks->push_back(std::shared_ptr<Task>());
            back_ids->push_back(i);
            continue;
        }
        //is it end action?
        if(active_list_p->at(i)->getEndTime() == *timeTaken) //end action is ready to be used
        {
            //delete temp tasks
            temp_tasks->clear();
            previously_not_useful.clear();
            return active_list_p->at(i);
        }
        if((active_list_p->at(i)->getEndTime() > -1)&&( active_list_p->at(i)->getEndTime() < *timeTaken)) //end action is ready to be used
        {
            //this might lead to Backtracking actually
            std::cerr << "main::getAction: ERROR: missed deadline of an action\n";
            throw;
        }
        /*if(active_list_p->at(i)->getTask()->getDeadline() < *timeTaken)
        {
            //this action missed its deadline
        }*/
        if(active_list_p->at(i)->getEndTime() == -1) //it is a start action
        {
          float action_dur = active_list_p->at(i)->getStep()->getDuration();
          precond_needed.at(i) = checkPreconditions(active_list_p->at(i),"",true);
          if (precond_needed.at(i)==1) //there is a new plan
          {
             time_requir.push_back(temp_tasks->at(i)->getPlan()->getDuration() + action_dur);
          }
          else if (precond_needed.at(i)==0) //nothing needed
          {
            time_requir.push_back(action_dur);
          }
          else //plan does not exits
          {
              time_requir.push_back(INT_MAX); // to show that this one is broken
          }
        }
        else //end action
        {
            //9/9 new - even end action can require some condtions to hold, check that
            //checkPreconditions already add temp task
            //multiply be for to make end action specific
            precond_needed.at(i) = 4* checkPreconditions(active_list_p->at(i),"at end",true);

            if(precond_needed.at(i) == -4) //we cant find a plan how to satisfy the precond
            {
                //test if some of end's actions which are not yet merged can satisfy it
                for(unsigned int j=0; j < active_list_p->size();j++)
                {
                    if ((j!=i)&&(active_list_p->at(j)->getEndTime() > -1))
                    {
                        std::vector<std::pair<std::string,std::string>> unsat_cond = checkEffects(active_list_p->at(i),"at end",active_list_p->at(j));
                        if (unsat_cond.size() == 0) //all conditions are satisfied by the other action
                        {
                            //the other end can be satisfied
                            if((precond_needed.at(j)==0)||(precond_needed.at(j)==4))
                            {
                              choose_instead.at(i) = j;
                              break; //we have found one
                            }
                            else if (precond_needed.at(j)==-4)
                            {
                                //the other end cannot be satisfied either
                                //do nothing, leave there -4
                            }
                        }
                        else
                        {
                            //the other action couldnt satisfy all the preconditions
                            //TODO TODO !!
                            std::cerr << "getAction:this is awaiting to be fixed\n";
                            throw;
                        }
                    }

                }
            }

            time_requir.push_back(INT_MAX); //we want this super big not be chosen before it can be, unless there is no other options to choose

        }
    }
    //check how to satisfied active links
   for(unsigned int i=0;i<active_links->size();i++)
   {
       if(previously_not_useful.size()>0)
       {
           std::vector<int>::iterator notu_ind = std::find(previously_not_useful.begin(),previously_not_useful.end(),(i+active_list_p->size()) );
           if(notu_ind!=previously_not_useful.end()) //index found
           {
               //this id lead to returned NULL previously, we dont want to choose it again
               precond_needed.at(active_list_p->size()+i)= 3; //skip this one
               time_requir.push_back(INT_MAX);
               temp_tasks->push_back(std::shared_ptr<Task>());
               continue;
           }
       }


          //we want to plan for link as long as it exists
          precond_needed.at(active_list_p->size()+i) = checkLinks(active_links->at(i));
          if (precond_needed.at(active_list_p->size()+i)==1) //there is a new plan
          {
               time_requir.push_back(temp_tasks->at(active_list_p->size()+i)->getPlan()->getDuration());
          }
          else if(precond_needed.at(active_list_p->size()+i)==0) //no plan needed
          {
              time_requir.push_back(0);
          }
          else //plan not found
          {
              time_requir.push_back(INT_MAX);
          }

   }


    std::vector<float>::iterator min_value;
    size_t index;


    bool threatsFound = false;
    do{ //check if chosen action/link does not cause threat on the realPlan links

        min_value = std::min_element(time_requir.begin(),time_requir.end());


        //get all actions and links with the minimal value
        //if the action and link has same time_requir, do actions first, as the links always requires adding new actions
        float active_time_requir = *min_value;
        std::vector<size_t> all_indexes;
        std::vector<float>::iterator itTR = std::find(time_requir.begin(),time_requir.end(),active_time_requir);
        while(itTR != time_requir.end())//we found something
        {
            all_indexes.push_back(std::distance(time_requir.begin(),itTR));
            itTR = std::find(itTR+1,time_requir.end(),active_time_requir);
        }
        //sort indexes - smaller are from actions, bigger from links
        std::sort(all_indexes.begin(), all_indexes.end());

        index = all_indexes.at(0); //choose first one


       //sort of same logic as above
       if(time_requir.at(index) == INT_MAX) //the smallest value is error value, the rest will be also error values
       {
          //find if there is some precond_needed 0 -> that signals on the end task with satisfied end condition
           std::vector<int>::iterator it = std::find_if(precond_needed.begin(), precond_needed.end(),zerosPrec);

           std::vector<size_t> indexes_of_zeros;
           while(it != precond_needed.end())
           {
              indexes_of_zeros.push_back(std::distance(precond_needed.begin(),it));
              it = std::find_if(it+1, precond_needed.end(),zerosPrec);
           }

           //find end action which needs to satisfy their precond
           std::vector<int>::iterator it2 = std::find_if(precond_needed.begin(), precond_needed.end(),fourPrec);

           std::vector<size_t> indexes_of_four;
           while(it2 != precond_needed.end())
           {
              indexes_of_four.push_back(std::distance(precond_needed.begin(),it2));
              it2 = std::find_if(it2+1, precond_needed.end(),fourPrec);
           }

           std::vector<std::pair<double, int> > deadlines;

           //if preconds are zero, and time_requis INT_MAX, it is only the END action with satified conditions
           if(indexes_of_zeros.size() != 0 )//we found some actions end which can be merged
           {
               //we need to choose such an action which has earlier deadline
               for(unsigned int d=0;d<indexes_of_zeros.size();d++)
               {
                   deadlines.push_back(std::pair<double,int>(active_list_p->at(indexes_of_zeros.at(d))->getEndTime(),indexes_of_zeros.at(d)));
               }
           }
           //we have also some end actions without satisfied precond
           if(indexes_of_four.size() != 0 )//we found some actions end
           {
               //we need to choose such an action which has earlier deadline
               for(unsigned int d=0;d<indexes_of_four.size();d++)
               {
                   deadlines.push_back(std::pair<double,int>(active_list_p->at(indexes_of_four.at(d))->getEndTime(),indexes_of_four.at(d)));
               }
           }
           //we have some end actions
           if(deadlines.size()>0)
           {   //sort them
               std::sort(deadlines.begin(),deadlines.end(), compareDeadlines);
               //choose the first -> the minimal one
               index = deadlines.at(0).second;

           }
           //the deadline with unsatisfied precond should be chosen
           if(precond_needed.at(index)==-4)
           {
               if(choose_instead.at(index)!=-1) //there is something else to pick
               {
                   size_t g_index_a = realPlan->getUnfishedId(active_list_p->at(index));


                   //update Orderings
                   //at_end ChooseInstead < at_end original

                   std::shared_ptr<const TemporalOrderings> to = realPlan->getOrderings();
                   if (to == NULL)
                   {
                       std::cerr << "getAction::ERROR:empty ordering\n";
                       throw;
                   }
                   size_t g_index_b = realPlan->getUnfishedId(active_list_p->at(choose_instead.at(index)));

                   if((g_index_a == std::numeric_limits<std::size_t>::max())||(g_index_b == std::numeric_limits<std::size_t>::max()))
                   {
                       std::cerr << "getAction::ERROR:unfinishedAction hasnt been found\n";
                       throw;
                   }

                   //before_id, before_time, after_id, after_time

                   to.reset(to->refine(Ordering(g_index_b,StepTime::AT_END,g_index_a,StepTime::AT_END,1), *active_list_p->at(index)->getStep()));
                   if (to == NULL)
                   {
                       std::cerr << ";makeOrderings::ERROR:adding new action to ordering failed\n";
                       throw;
                   }
                   if(DEBUG1)
                   {
                       std::cout << "------------------------\n";
                       std::cout << "swapping actions in getAction\n";
                       std::ostream &osg = std::cout;
                       to->print(osg);
                       std::cout << "\n";
                       std::cout << "------------------------\n";
                   }

                   double new_end = to->distanceDouble(2*g_index_a,0);
                   active_list_p->at(index)->setEndTime(new_end);

                   index = choose_instead.at(index);


                   realPlan->setOrderings(to);

               }
           }

           //hence wee do not have any end action, or it requires precondition
           //check if we have something with 2 -> backtrack
           //this prefers backtrack before adding precond to satify end action
           //the logic is -> if we have a backtrack signal, it feels that something is broken,
           //hence dont try to keep breaking it
           //TODO is the above assumption correct?
           if(precond_needed.at(index)!=0)
           {
               //we dont want to choose either -1 (no plan), or 3 - didnt lead anywhere last time

               std::vector<int>::iterator it = std::find_if(precond_needed.begin(), precond_needed.end(),twosPrec);
               if(it != precond_needed.end())
               {
                  index = (std::distance(precond_needed.begin(),it));
               }

           }

       }


      if(precond_needed.at(index)==0)
      {
        //if this method returns true, we need to choose another action/link
        threatsFound =threatsLinks(realPlan,active_list_p, index);
      }
      else if ((precond_needed.at(index)==1)|| (precond_needed.at(index)==4))//we are going to add some actions, test them!
      {
          std::shared_ptr< std::vector< std::shared_ptr<StepTimed> > > temp_active_list(new std::vector< std::shared_ptr<StepTimed> >() );
          std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > temp_active_links(new std::vector< std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > >() );
          std::shared_ptr<Task> tt(new Task(*temp_tasks->at(index).get()));
          addSteps(tt, temp_active_list, temp_active_links);

          threatsFound =threatsLinks(realPlan,temp_active_list, 0); //index is 0 in this case, as we want to test the first adding (that is going to be chosen)
      }
      else //precond_needed 2 or 3 or -1 or -4
      {
         threatsFound=false;
      }
     //change time_requir and precond_needed
     if(threatsFound)
     {
         time_requir.at(index) = INT_MAX;
         precond_needed.at(index) = -1; //to signal that this one is breaking links
         previously_not_useful.push_back(index);
     }
   } while(threatsFound);



    std::shared_ptr<StepTimed> active_action;

    //only if I have picked something
    if((precond_needed.at(index)==0)||(precond_needed.at(index)==1)||(precond_needed.at(index)==4))
    {

        //if the picked action must be order after Y, check if Y is already merged
        int active_ind;
        int active_task;
        std::string time;

        if(index < active_list_p->size()) //from actions
        {
            active_ind = active_list_p->at(index)->getStep()->getId();
            active_task = active_list_p->at(index)->getTask()->getID();

            if(active_list_p->at(index)->getEndTime()==-1) //start
                time="start";
            else
                time="end";

            if(active_task<0)
            {
                active_ind = active_list_p->at(index)->getTask()->getParentStepId();
                active_task = active_list_p->at(index)->getTask()->getParentTask();
                time = "start";
            }

        }
        else // it is from links
        {
            active_ind = active_links->at(index-active_list_p->size())->first->getToId();
            active_task = active_links->at(index-active_list_p->size())->second->getID();
            time = "start";
        }

        int num_steps_previous1 = 0;



        for(unsigned int i=0; i< tasks->size();i++)
        {
                if(tasks->at(i)->getID()!=active_task)
                {
                    num_steps_previous1+= tasks->at(i)->getPlan()->num_steps();
                }
                else
                {
                    break;
                }
        }

        //active_ind can be -1 when it came from to_id in a link and it is a goal step
        if(active_ind!=-1)
        {


            int ind_pick = (2*active_ind)+(2*num_steps_previous1);
            if(time=="start")
                ind_pick--;

            ind_pick--;

            //if it cannot come first, return NULL
            if(!realPlan->getCanComeFirst(num_steps_previous1+active_ind-1))
            {
                if(realPlan->amountOfSteps()==0)
                {
                  active_action = NULL;
                  previously_not_useful.push_back(index);
                  return active_action;
                }
            }

            //this for loop is checking if anything which must preceed picked action is merged
            for(unsigned int x=0; x < realPlan->getNumMainSteps();x++)
            {
               int ordering_value = realPlan->getStepsOrderings(ind_pick,x);
               if (ordering_value == INT_MAX)
                   continue;

               if(ordering_value > 0)
               {
                    //is x merged?

                   //get task_id
                   int previous_steps=0;
                   int recovered_task_id=-1;
                   for(unsigned int y=0; y < tasks->size(); y++)
                   {
                       if(x<(previous_steps+2*tasks->at(y)->getPlan()->num_steps()))
                       {
                           recovered_task_id = y;
                           break;
                       }
                       else
                       {
                           previous_steps+=2*tasks->at(y)->getPlan()->num_steps();
                       }
                   }
                   if(recovered_task_id == -1)
                   {
                       std::cout<<"getAction::ERROR: not able to recover task id\n";
                       throw;
                   }
                   else
                   {
                       int recovered_step_id =x;
                       if(x%2 == 0) //it is start action
                       {
                           recovered_step_id = x+2;
                       }
                       else
                       {
                           recovered_step_id = x+1;
                       }
                       //not 2*previous_steps as the multiply by 2 is done above
                       recovered_step_id -= previous_steps;
                       recovered_step_id = recovered_step_id/2;

                       //std::vector<std::pair<std::pair<size_t,float>,int> >
                       std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(recovered_task_id)->getPlan()->getSteps();
                       for(unsigned int y=0; y < sv.size(); y++)
                       {
                           if(sv.at(y).first->getId() == recovered_step_id)
                           {
                               //start or end action?
                               if((x%2==0)&&(sv.at(y).second == -1)) //the start is not merged
                               {
                                   active_action = NULL;
                                   previously_not_useful.push_back(index);
                                   return active_action;
                               }
                               else if((x%2==1)&&(sv.at(y).second !=1)) //the end is not merged
                               {
                                   active_action = NULL;
                                   previously_not_useful.push_back(index);
                                   return active_action;
                               }
                               else //OK
                               {
                                   //do nothing
                                   }
                                   break;
                               }
                           }
                       }
                   }
                }//end for
          }
     }

        if ((precond_needed.at(index)==1)||(precond_needed.at(index)==4))
        {
            unsigned int size_before = active_list_p->size();
            std::shared_ptr<Task> tt(new Task(*temp_tasks->at(index).get()));
            temp_tasks_used->push_back(tt);
            addSteps(tt, active_list_p, active_links);

            if (active_list_p->size() > size_before) //added some actions
            {
              //take the first added action
              active_action =active_list_p->at(size_before);

              //double check if the conditions are satisfied
              if(checkPreconditions(active_action, "",false)==1) //the preconditions are not satisfied
              {
                  previously_not_useful.push_back(index);
                  active_action = NULL;
              }
              else
              {
                  previously_not_useful.clear();
              }
            }
            else //no action was added, maybe some link? but for that, we need to plan
            {
              previously_not_useful.push_back(index);
              active_action = NULL;
            }

            //delete all temp tasks as they will become invalid
            //TODO is this correct way of deleting? Isnt a memory leak?
            temp_tasks.reset(new std::vector<std::shared_ptr<Task> >());

            return active_action;
        }
        else if (precond_needed.at(index)==0) // no plan needed
        {
            previously_not_useful.clear();
            if(time_requir.at(index) == INT_MAX) //this means that the end actions wants to be choosen, but the time is not yet ready
            {

                temp_tasks.reset(new std::vector<std::shared_ptr<Task> >());
                return active_list_p->at(index);
            }

            if(index < active_list_p->size()) //from actions
            {
              std::shared_ptr<StepTimed> active_action = active_list_p->at(index);
              temp_tasks->clear();
              return  active_action;
            }
            else //action comes from links, but there is no action, this shouldnt happen
            {
              std::cerr<<"main::getAction:ERROR: Action comes from links\n";
              throw;
            }
        }
        else if (precond_needed.at(index)==-1) // chosen plan doesnt exists
        {
            active_action = NULL;
            previously_not_useful.push_back(index);
            temp_tasks.reset(new std::vector<std::shared_ptr<Task> >());
            return active_action;
        }
        else if (precond_needed.at(index)==-4) //there is no way how to satisfy this => we need to backtrack
        {
            previously_not_useful.clear();
            std::shared_ptr<StepTimed> active_action = active_list_p->at(index);
            active_action->setBacktrack(true); //in order to push for backtracking
            temp_tasks->clear();
            return  active_action;
        }
        else if (precond_needed.at(index)==2) // the backtrack action was chosen
        {
            previously_not_useful.clear();
            //return the backtrack action, we will need to backtrack again
            std::shared_ptr<StepTimed> active_action = active_list_p->at(index);
            temp_tasks->clear();
            return  active_action;
        }

        return NULL;

}

/* 05/09 checked
 * this method purely test if the goal literals are in the current state
 * task = the task to test
 */
static bool checkOneTaskBeUsed(std::shared_ptr<Task> task)
{
    std::vector<std::string> goals = task->getPlan()->getGoals();
    std::string domain = task->getDomain();

    int goalSatisfied = 1;

    for (unsigned int i=0; i< goals.size(); i++)
    {

      //goal action has just "at end" conditions
      goalSatisfied *= areConditionsSatisfiedNow(goals.at(i), "all", domain);
      if(goalSatisfied == 0)
          break;
    }

    if(goalSatisfied == 1)
    {
        task->setUsed(true);
        return true;
    }
    return false;
}

/*15/09 checked
 */
static void checkTaskBeUsed()
{
   //task can be used even if not all his actions were used, check the condition
  for(unsigned int i=0; i< tasks->size();i++)
  {
    checkOneTaskBeUsed(tasks->at(i));
  }
}

/* 09/09 checked
 * this method creates links between steps already in the realPlan achieving some condition
 * and those being added
 * realPlan = the final plan
 * active_action_p = the action being merged
 * which_link ="start" or "end" to choose which conditions should be checked,depending if we merge beggining or end of the action
 * id_to = the global id in the real plan of the action being merged
 * return = the string of from_ids, ie actions producing some literal for the link
*/
static std::vector<size_t> createLinks(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p, std::string whichLink, size_t id_to)
{

    std::vector<size_t> from_ids; //return vector
    //link each condition of the active_action to its achiever

    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = active_action_p->getStep()->getConditions();

    std::string literal = "(and ";

    for(unsigned int i=0;i < condi.size();i++)
    {

       //if(condi.at(i).first.first.first != "at start")
       //{
           literal += "(";
           literal += condi.at(i).first.first.first;
           literal += " ";
       //}



        literal += condi.at(i).first.first.second;
        for(unsigned int j=0; j< condi.at(i).first.second.size();j++)
        {
            literal += " ";
            literal += active_action_p->getStep()->getParameters().at(condi.at(i).first.second.at(j));

        }
        literal += condi.at(i).second;
        //if(condi.at(i).first.first.first != "at start")
        //{
          literal += ")";
        //}
    }
    literal+= ")";

    std::vector<std::pair<std::string,std::string> > sub_actions = getSubConditions(literal);

    //for each condition create separate link
    for(unsigned int i = 0; i< sub_actions.size();i++)
    {

        //just add the links to the conditions, referring if we are adding start of the action or end
        if(((whichLink == "start")&&((sub_actions.at(i).second == "at start") || (sub_actions.at(i).second == "over all"))) ||
          ((whichLink == "end")&&((sub_actions.at(i).second == "at end")))) //not overall as it would add the link againss
        {
            std::shared_ptr<MyLink> lp;

            std::map<std::string, std::vector<std::pair<std::string, int> > >::iterator it;


            //TODO, negative conditions needs to be tested!


            //g_id is  the id of the sep in the realPlan
            bool threaten = (sub_actions.at(i).second == "over all"); //might be threaten

            it = realPlan->achievers.find(sub_actions.at(i).first);
            //what if the conditions literal is negative???
            //added 09/09 new
            if (it == realPlan->achievers.end()) // condition not found
            {
                //deleters are without the not
                if(sub_actions.at(i).first.find("(not")!=std::string::npos) //"not" found
                {
                     std::string pure_atom = sub_actions.at(i).first.substr(5,sub_actions.at(i).first.length()-5-1);//-1 to get away the last bracket
                     it = realPlan->deleters.find(pure_atom);
                }
                else //we cant have positive condition and be looking to deleters,
                {
                     it = realPlan->deleters.end();
                }
                if(it == realPlan->deleters.end()) //the condition have not been found in achievers nor deleters
                {
                     std::cerr << "main::createLinks:ERROR:the condition not found in achievers or deleters\n";
                      throw;
                }
            }

            //achiever is std::map<std::string, std::vector<std::pair<std::string, int>>>, last string is as steptime
            //that is the string literal, the time when an effect achieve it(start,end) and the id of the step achieving it
            //the last item in the vector is the last step achieving it
            //link - from_id, effect_time, to_id, lit *, cond_time, being_threaten, type
            std::pair<std::string,int> pr = *(it->second.end()-1); //get the last one

            //TODO test - the condition time will be now "" for "at start", is it problem somewhere?
            lp.reset(new MyLink(pr.second, pr.first, id_to, sub_actions.at(i).first, sub_actions.at(i).second, threaten));

            realPlan->addLink(lp);
            from_ids.push_back(pr.second);


        }
    }

    return from_ids;
}


/* 09/09 checked
 * it makes sure that all links have corresponding orderings
 * realPlan = the final plan
 * from_ids = the ids of actions in the final plan which produce a literal, hence must be ordered before the step s
 * s = a copy of step beiing merged
 */

static bool makeOrderings(std::shared_ptr<FinalPlan> realPlan, std::vector<size_t> from_ids,  std::shared_ptr<StepTimed> s,
                         size_t previous_id,std::shared_ptr<StepTime> previous_time, StepTime current_time, double * timeIncr, bool newStart)
{
    std::shared_ptr<const TemporalOrderings> to = realPlan->getOrderings();
    if (to == NULL)
    {
        std::cerr << "makeOrderings::ERROR:orderings are empty\n";
        throw;
    }

    //dummy ordering start at 0
    //before_id, before_time, after_id, after_time
    //can start with start
    to.reset(to->refine(Ordering(0,StepTime::AT_START,s->getStep()->getId(),StepTime::AT_START,s->getTask()->getReleaseDate()), *s->getStep().get() ));
    if (to == NULL)
    {
        std::cerr << ";makeOrderings::ERROR:adding new action to ordering failed\n";
        throw;
    }
    if(DEBUG1)
    {
        std::cout << "------------------------\n";
        std::cout << "initialised\n";
        std::ostream &osg = std::cout;
        to->print(osg);
        std::cout << "\n";
        std::cout << "------------------------\n";
    }


    //add the limitations based on added links

    *timeIncr = 0;
    std::shared_ptr< std::vector<std::shared_ptr<MyLink> > > vecL = realPlan->getLinks(); //dont delete thhis pointer as it is not owner

    for(unsigned int j=0; j<from_ids.size();j++)
    {
        for(unsigned int i=0;i < vecL->size();i++)
        {
            //the orderings must contains all the links
            if((vecL->at(i)->getFromId() == from_ids.at(j))&&
               (vecL->at(i)->getToId() == s->getStep()->getId()))
            {
                StepTime time;
                if(vecL->at(i)->getEffectTime() == "at start")
                {
                    time=StepTime::AT_START;
                }
                else if (vecL->at(i)->getEffectTime() == "at end")
                {
                    time=StepTime::AT_END;
                }
                else
                {
                    std::cerr << "wrong effect time\n";
                    throw;
                }

                if(from_ids.at(j)==0)
                {
                    time = StepTime::AT_START;
                }


                if(vecL->at(i)->getConditionTime() == "at start")
                {
                    //if the link is from previous id with at start, add incr
                    //unless it is a start of a new time window
                    if(from_ids.at(j)+1 == s->getStep()->getId()&&(!newStart))
                    {
                        *timeIncr = 1;
                        to.reset(to->refine(Ordering(from_ids.at(j),time,s->getStep()->getId(),StepTime::AT_START,1), *s->getStep().get()));

                    }
                    else
                    {
                        to.reset(to->refine(Ordering(from_ids.at(j),time,s->getStep()->getId(),StepTime::AT_START,0), *s->getStep().get()));

                    }

                }
                else if(vecL->at(i)->getConditionTime() == "over all")
                {
                    to.reset(to->refine(Ordering(from_ids.at(j),time,s->getStep()->getId(),StepTime::AT_START,0), *s->getStep().get()));
                    to.reset(to->refine(Ordering(from_ids.at(j),time,s->getStep()->getId(),StepTime::AT_END,0), *s->getStep().get()));
                }
                else if(vecL->at(i)->getConditionTime() == "at end")
                {
                    to.reset(to->refine(Ordering(from_ids.at(j),time,s->getStep()->getId(),StepTime::AT_END,0), *s->getStep().get()));
                }
                else
                {
                    std::cerr << "wrong condition time\n";
                    throw;
                }
                break;
            }
        }
    }
    if (to == NULL)
    {
        std::cerr << ";makeOrderings::ERROR:refining orderings based on links failed\n";
        throw; //originally return false;
    }
    if(DEBUG1)
    {
        std::cout << "------------------------\n";
        std::cout << "after links\n";
        std::ostream &osg = std::cout;
        to->print(osg);
        std::cout << "\n";
        std::cout << "------------------------\n";
    }


    //12/09 added
    //to set that the new action instant is after the previous one
    to.reset(to->refine(Ordering(previous_id,*previous_time,s->getStep()->getId(),current_time,0),  *s->getStep().get()));
    if (to == NULL)
    {
        std::cerr << ";makeOrderings::ERROR:refining orderings based on adding this action after previous one failed\n";
        throw; //originally return false;
    }
    if(DEBUG1)
    {
        std::cout << "------------------------\n";
        std::cout << "after my ordering\n";
        std::ostream &osg = std::cout;
        to->print(osg);
        std::cout << "\n";
        std::cout << "------------------------\n";
    }

    realPlan->setOrderings(to);
    return true;
}

/*15/09 checked
 * this was extracted from the mergeAction method
 */
static bool makeLinksandOrderings(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p, size_t active_id,
                                                    size_t previous_id,std::shared_ptr<StepTime> previous_time, double * timeIncr, bool newStart)
{
    std::shared_ptr<StepTimed> s;
    bool orderingSuc;
    if(active_action_p->getEndTime()==-1) //start action
    {
       //in order to change IDs of the final steps

        //in order to copy the step
       s.reset(new StepTimed(*active_action_p));
       s->getStep()->setId(active_id);
       realPlan->addStep(std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >(active_action_p,std::pair<std::string,size_t>("start",active_id)));
       realPlan->unfinished_actions[active_action_p] = active_id;
       std::vector<size_t> from_ids = createLinks(realPlan,active_action_p,"start",active_id);

       orderingSuc  = makeOrderings(realPlan, from_ids, s,previous_id,previous_time,StepTime::AT_START, timeIncr, newStart);



    }
    else //end action
    {
        std::map< std::shared_ptr<StepTimed>, size_t>::iterator it = realPlan->unfinished_actions.find(active_action_p);
        if (it == realPlan->unfinished_actions.end()) //not found anything
        {

          std::cerr << "main::makeLinksandOrderings:ERROR:Action is unfinished but it is not in the unfinished map\n";
          throw;
        }
       s.reset(new StepTimed(*active_action_p));
       s->getStep()->setId(active_id);
       realPlan->addStep(std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >(active_action_p,std::pair<std::string, size_t>("end", active_id)));
       realPlan->unfinished_actions.erase(it);
       std::vector<size_t> from_ids = createLinks(realPlan,active_action_p,"end", active_id);
       orderingSuc = makeOrderings(realPlan, from_ids, s,previous_id,previous_time,StepTime::AT_END, timeIncr, newStart);

       //change threaten flag
       std::shared_ptr<std::vector<std::shared_ptr<MyLink> > > vecL = realPlan->getLinks();

       std::vector<std::shared_ptr<MyLink> >::iterator itL = std::find_if(vecL->begin(), vecL->end(),canBeThreaten);

       while (itL != vecL->end())
       {
         if((*itL)->getToId() == active_id)
         {
            //as the action is finished now, the link is not threaten anymore
             //this change the privaee link inside the plan
            realPlan->changeLink(std::distance(vecL->begin(),itL),false);
         }
         itL = std::find_if(itL+1, vecL->end(),canBeThreaten);
       }
    }

    return orderingSuc;
}

/*15/09
 * this was extracted from the mergeAction method
 */
static void mergeEffects(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p,
                         size_t active_id, bool realplan)
{

    //time, predicate parameters end brackets
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effL = active_action_p->getStep()->getEffects();

    size_t index;
    std::string domain = active_action_p->getTask()->getDomain();

    for (unsigned int i =0; i< effL.size();i ++)
    {
        /*use only those effect which rely if we are merge start or end of the action*/
        if((active_action_p->getEndTime()==-1)&&(effL.at(i).first.first.first != "at start"))
        //we want start effects, but this is not one, do nothing
        {
            continue;
        }
        if((active_action_p->getEndTime()!=-1)&&(effL.at(i).first.first.first != "at end"))
        //we want end effects, but this is not one, do nothing
        {
            continue;
        }

        //we got correct effect, put it to the current state
        std::string literal;
        literal = effL.at(i).first.first.second;
        for(unsigned int k=0; k< effL.at(i).first.second.size();k++)
        {
            literal += " ";
            literal += active_action_p->getStep()->getParameters().at(effL.at(i).first.second.at(k));
        }
        literal += effL.at(i).second;

        std::string action = literal;
        bool neg = false;
        size_t index2;

        if(action.find("not") != std::string::npos) //negative
        {
            neg = true;
            index = action.find("(",2); //skip the first bracket
        }
        else
        {
          index = action.find("(");
        }


      if(index<std::numeric_limits<std::size_t>::max())
      {
        if(neg)
        {
            int index3 = action.find(")");
            action = action.substr(index,index3-index+1);//to ingore last bracket
        }
        else
            action = action.substr(index,action.size());

        //TODO I need to be really careful in the domain that between not and anything else
        //is a space
        std::string not_action = "(not "+action+")";
        std::vector<std::string> vec = current_state[domain];
        index2 = std::distance(vec.begin(),std::find(vec.begin(),vec.end(),action));

        size_t index2_not = std::distance(vec.begin(),std::find(vec.begin(),vec.end(),not_action)); //try to find negation of the action


        if(!neg) //it is positive atom
        {
            if(index2_not < vec.size()) //it is presented in the negative form
            {
                //we want to delete it add the new form
                if(realplan)
                {
                  current_state[domain].erase(current_state[domain].begin()+index2_not);
                  current_state[domain].push_back(action);
                }
                realPlan->addAchiever(action, effL.at(i).first.first.first, active_id);
            }
            else //it is not presented in the negative form
            {
                if(index2 < vec.size()) //it is presented in the positive form
                {
                   //it is already there, dont update current_state
                   realPlan->addAchiever(action, effL.at(i).first.first.first, active_id);
                }
                else //it is not presented
                {
                   if(realplan)
                     current_state[domain].push_back(action);
                   realPlan->addAchiever(action, effL.at(i).first.first.first, active_id);
                }

            }
        }
        else //it is negative form
        {
            if(index2 < vec.size()) //it is presented in positive form, we want to delete it
            {
                if(realplan)
                {
                  current_state[domain].erase(current_state[domain].begin()+index2);
                  current_state[domain].push_back("(not "+action+")");
                }
                //I am saving the positive conditions, Iam not having there "not"
                realPlan->addDeleter(action, effL.at(i).first.first.first, active_id);
            }
            else //it is not presented in the positive form
            {
                if(index2_not < vec.size()) //it is presented in the negative form
                {
                    //dont update current_state
                    realPlan->addDeleter(action, effL.at(i).first.first.first, active_id);
                }
                else //not presented in the negative form, we need to add the neg
                {
                    if(realplan)
                      current_state[domain].push_back("(not "+action+")");
                    //I am saving the positive conditions, Iam not having there "not"
                    realPlan->addDeleter(action, effL.at(i).first.first.first, active_id);
                }

            }

        }

      }

  }


}

/*
 * link - the link to change
 * condition - the link's condition to test
 * i - task id
 * domain - the task's domain
 * sat 0 = for not satisfied, 1= for satisfied
 */
static void changeGeneralLinks(std::pair<std::shared_ptr<MyLink> ,int> * link, std::string condition, int i, std::string domain, int sat)
{
    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > start_vec = tasks->at(i)->getPlan()->getSteps();

    for (unsigned int j =0; j < start_vec.size(); j++)
    {

      if(start_vec.at(j).first->getId() == link->first->getToId()) //if the action is to_id
      {
        //if the link is to AT_START, and the action has not become to be merged - change it
        //if the link is to AT_END, dtto
        //OVER_ALL, dtto
        if(start_vec.at(j).second==-1)
        {
           //changed here on 03/10 to "all" (instead "")
           if(areConditionsSatisfiedNow(condition, "all", domain)==sat) // 0 = they are not satisfied, 1 = they are satisfied
           {
               link->second = -link->second;

           }

        }
        //if the links is to AT_START but the action start has been already merged, dont change
         //AT_END cahnge
         //OVER_ALL check, throw exception if needed
        else if(start_vec.at(j).second==0)
        {
            if (link->first->getConditionTime() == "at end")
            {
                //changed here on 03/10 "at_end" instead ""
                if(areConditionsSatisfiedNow(condition, "all", domain)==sat) //0 = they are not satisfied, 1 = they are satisfied
                {
                    link->second = -link->second;
                }
            }
            else if (link->first->getConditionTime() == "over all")
            {

                //changed here on 03/10 "over_all" instead""
                if(areConditionsSatisfiedNow(condition, "all", domain)==0) //they are not satisfied
                {
                    std::cerr << "main::makeLinksUnsat:ERROR: the overall link condition is not satisfied\n";
                    throw;
                }
            }
        }
        //else the start_vec.at(j).second==1 - all of the action has been merged, dont change any link
        break; //we dont have to find any other action

      }
    }
    //the link goes to the GOAL and the task is not yet satisfied
    if((link->first->getToId()==Plan::GOAL_ID)&&(!checkOneTaskBeUsed(tasks->at(i))))
    {
        //changed here on 03/10 "all" instead ""
        if(areConditionsSatisfiedNow(condition, "all", domain)==sat) //they are not satisfied
        {
            link->second = -link->second;
        }
    }
}


/* this method change satisfied links to unsatisfied if the from_id has been merged, but to_id not yet
 * and the condition is not valid anymore
 * action_id = active action being merged
 * task_id = task id to which active action belongs
 * type = "start", "end" - what point of action it is
 */
static void makeLinksUnsat(size_t action_id, int task_Id, std::string type)
{
    //just for original tasks, TODO - maybe I will need to cahnge this if I want to reuse temporal tasks
    for(unsigned int i = 0; i< tasks->size();i++) //throw all tasks
    {
        std::string domain = tasks->at(i)->getDomain();
        std::vector<std::pair<std::shared_ptr<MyLink> ,int> > links_vec=tasks->at(i)->getPlan()->getLinks();
        for (unsigned int k = 0; k < links_vec.size(); k++)
        {
          if(links_vec.at(k).second == 1) //if they are satisfied, they might change
          {
            std::string condition = links_vec.at(k).first->getCondition();

            std::pair<int,int> pta = std::pair<int,int>(task_Id,action_id);
            std::map<std::pair<int,int>, std::pair<int,int> >::iterator it = similar_actions.find(pta);


            //if its the currently merged action or it is the copy of the currently merged action
            if (((tasks->at(i)->getID() == task_Id)&&(links_vec.at(k).first->getToId() == action_id))
                ||
                ((it!=similar_actions.end())&&(it->second.first==tasks->at(i)->getID())&&(it->second.second==links_vec.at(k).first->getToId()))
                )
            {
                //check the type of the link
                //if it is a start action and the link is to start, do nothing, the link is consumed
                //if it is a start action and the link is to end, it can change
                //if it is a start action and the link is overall, check it - it should keep holding
                if(type=="start")
                {
                    if (links_vec.at(k).first->getConditionTime() == "at end")
                    {
                        //here changed on 03/10
                        if(areConditionsSatisfiedNow(condition, "all", domain)==0) //they are not satisfied
                        {
                            links_vec.at(k).second = -1;
                        }
                    }
                    else if (links_vec.at(k).first->getConditionTime() == "over all")
                    {
                        //here changed on 03/10
                        if(areConditionsSatisfiedNow(condition, "all", domain)==0) //they are not satisfied
                        {
                            std::cerr << "main::makeLinksUnsat:ERROR: the overall link condition is not satisfied\n";
                        }
                    }
                }
                //else - it is an end action, and link lead to it, dont change anything, all is consumeD
            }
            else //the link doesnt go to active_action_id
            {
              changeGeneralLinks(&links_vec.at(k), condition, i, domain, 0);
            }

          }//end of if(links_vec.at(k).second)

        }
        tasks->at(i)->getPlan()->setLinks(links_vec);


    }
 }

static void makeLinksSat(std::vector<std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> >  linksToChange)
{
    for(unsigned int i=0;i<linksToChange.size();i++) //these are link to made them satisfied
    {

      std::vector<std::pair<std::shared_ptr<MyLink> ,int> > links_vec=tasks->at(linksToChange.at(i).second)->getPlan()->getLinks();
      for(unsigned int k=0;k<links_vec.size();k++)
      {

         std::string domain = tasks->at(linksToChange.at(i).second)->getDomain();
         std::string condition = links_vec.at(k).first->getCondition();

         if(links_vec.at(k).second == -1) //only if the link is not yet satisfied
         {

           if(linksToChange.at(i).first.first.first == links_vec.at(k).first->getFromId())
           {
              //we care just about the fromId
              if(((linksToChange.at(i).first.first.second ==  -1)&&(linksToChange.at(i).first.second == "")) //we dont care or
                  ||((linksToChange.at(i).first.first.second == links_vec.at(k).first->getToId())&&(linksToChange.at(i).first.second == condition))) //we care abotu specific one
              {
                  //time is empty "" as the condition is from link, it doesnt have a temporal information
                  //here changed on 03/10
                  if(areConditionsSatisfiedNow(condition, "all", domain)==1)
                  {
                    links_vec.at(k).second = 1;
                  }

              }
           }
         }
       } //end of k loop
       tasks->at(linksToChange.at(i).second)->getPlan()->setLinks(links_vec);
    } //end of i loop

    for(unsigned int i=0; i < tasks->size();i++)
    {
        std::vector<std::pair<std::shared_ptr<MyLink> ,int> > links_vec=tasks->at(i)->getPlan()->getLinks();
        for(unsigned int k=0;k<links_vec.size();k++)
        {
           if(links_vec.at(k).second == -1) //only if the link is not yet satisfied
           {

             std::string domain = tasks->at(i)->getDomain();
             std::string condition = links_vec.at(k).first->getCondition();
             //the link goes to the GOAL and the task is not yet satisfied
             if((links_vec.at(k).first->getToId()==Plan::GOAL_ID)&&(!checkOneTaskBeUsed(tasks->at(i))))
             {
                 //here changed on 03/10
                 if(areConditionsSatisfiedNow(condition, "all", domain)==1) //they are satisfied
                 {
                     links_vec.at(k).second =1;
                 }
             }
           }
        }
         tasks->at(i)->getPlan()->setLinks(links_vec);
     }



}

static void updateMainStepsOrderings(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p)
{
    int taskId = active_action_p->getTask()->getID();
    int stepId = active_action_p->getStep()->getId();
    if(taskId<0)
    {
        //get the parent id and mark that
        //taskId = active_action_p->getTask()->getParentTask();
        //stepId = active_action_p->getTask()->getParentStepId();
        return;
    }


    //get the last step which is not joining action
    int index = realPlan->amountOfSteps()-1;

    while(index>-1)
    {
        std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > lastStep = realPlan->getStepwithGIndex(index);
        if(lastStep.second.first!="none")
        {
           while(lastStep.first->getTask()->getID()<0)
           {
               index--;
               lastStep = realPlan->getStepwithGIndex(index);
               //if there is no step, there is no ordering to update
               if(lastStep.second.first=="none")
                   break;
           }
           if(lastStep.second.first=="none")
               break;
        }
        else
            break;



        int num_steps_previous1=0;
        int num_steps_previous2=0;

        bool t1 = false;
        bool t2 = false;

        for(unsigned int i=0; i< tasks->size();i++)
        {
            if(!t1)
            {
                if(tasks->at(i)->getID()!=lastStep.first->getTask()->getID())
                {
                    num_steps_previous1+= tasks->at(i)->getPlan()->num_steps();
                }
                else
                {
                    t1=true;
                }
            }
            if(!t2)
            {
                if(tasks->at(i)->getID()!=taskId)
                {
                    num_steps_previous2+= tasks->at(i)->getPlan()->num_steps();
                }
                else
                {
                    t2=true;
                }
            }
            if(t1&&t2)
                break;
        }

        int from_id = lastStep.first->getStep()->getId();
        int to_id = stepId;

        int id1 = (from_id*2)+(2*num_steps_previous1);

        if(lastStep.second.first == "start")
        {
            id1--;
        }
        id1--; //the end is -1, start -2

        int id2 =  (to_id*2)+(2*num_steps_previous2);
        if(active_action_p->getEndTime()==-1) //it is start action
        {
            id2--;
        }
        id2--; //the end is -1, start -2
        realPlan->setStepsOrderings(id1,id2,-1, false);

        index--;
        if(DEBUG)
        {
            std::cout<< "--------------------------\n";
            std::ostream &os = std::cout;

            int taskId_l=0;
            int stepId_l=0;

            for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

               os << taskId_l <<"_"<<stepId_l << ": ";
               if(r%2 == 1)
               {
                 os << "end: ";
                 stepId_l++;
               }
               else
               {
                 os<< "sta: ";
               }
               if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
               {
                   taskId_l++;
                   stepId_l=0;
               }
              for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
                os.width(7);
                int d;
                d = realPlan->getStepsOrderings(r,c);

                if (d < INT_MAX) {
              os << d;
                } else {
              os << "inf";
                }
              }
              os << std::endl;
            }
            std::cout << "-------------------------\n";
            for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
            {
                std::cout << realPlan->getCanComeFirst(k);
                std::cout << " ";
            }
            std::cout << "\n";
        }
    }//end while

    //id2 is the active_action
    int num_steps_previous2=0;

    for(unsigned int i=0; i< tasks->size();i++)
    {
            if(tasks->at(i)->getID()!=taskId)
            {
                num_steps_previous2+= tasks->at(i)->getPlan()->num_steps();
            }
            else
            {
               break;
            }
    }
    int to_id = stepId;
    int id2 =  (to_id*2)+(2*num_steps_previous2);

    if(active_action_p->getEndTime()==-1) //it is start action
    {
        id2--;
    }
    id2--; //the end is -1, start -2
    for(unsigned int i=0; i< realPlan->getNumMainSteps(); i++)
    {
        if(realPlan->getStepsOrderings(id2,i)==-2)
        {
            //delete all "2"
            for(unsigned int j=0;j<realPlan->getNumMainSteps();j++)
            {
                realPlan->setStepsOrderings(i,j,INT_MAX, false);
            }
            //make this ordering satisfied:
            realPlan->setStepsOrderings(id2,i,-5, false);
        }
    }
    if(DEBUG)
    {
        std::cout<< "--------------------------\n";
        std::ostream &os = std::cout;

        int taskId_l=0;
        int stepId_l=0;

        for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

           os << taskId_l <<"_"<<stepId_l << ": ";
           if(r%2 == 1)
           {
             os << "end: ";
             stepId_l++;
           }
           else
           {
             os<< "sta: ";
           }
           if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
           {
               taskId_l++;
               stepId_l=0;
           }
          for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
            os.width(7);
            int d;
            d = realPlan->getStepsOrderings(r,c);

            if (d < INT_MAX) {
          os << d;
            } else {
          os << "inf";
            }
          }
          os << std::endl;
        }
        std::cout << "-------------------------\n";
        for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
        {
            std::cout << realPlan->getCanComeFirst(k);
            std::cout << " ";
        }
        std::cout << "\n";
    }



}

//TODO IMPORTANT - STEPTIME CAN BE ALSO AFTER START AND BEFORE END, I need to take this into account
//we know that active_action can be merge, so do it
static bool mergeAction(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p, double * timeTaken,
                        std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links,
                        std::shared_ptr< std::vector< std::shared_ptr<StepTimed> > > active_list,
                        size_t active_id, bool realplan,
                        size_t * previous_id,std::shared_ptr<StepTime> previous_time, bool newStart)
{

    updateMainStepsOrderings(realPlan, active_action_p);


    //first create links - create only those depending if the action is start, or end
    //end condiitons might be satisfied later by something
    double timeIncr = -1;
    bool orderingSuc = makeLinksandOrderings(realPlan, active_action_p,active_id, *previous_id, previous_time, &timeIncr, newStart);

    if(!orderingSuc)
    {
        return false;
    }






    *previous_id = active_id;
    std::string action_type;

    if(active_action_p->getEndTime()==-1)
    {
       previous_time->point = StepTime::StepPoint::START;
       previous_time->rel = StepTime::StepRel::AT;
       action_type = "start";
       //30/10 fixed orderings so this shouldnt be needed anymore
       //however still some issue - two drive action need a space, otherwise VAL fails
       if(timeIncr == 1)
       {
         *timeTaken+= incr;
       }

       active_action_p->setStartTime(*timeTaken);
    }
    else
    {
        *timeTaken = active_action_p->getEndTime(); //progress with time
        previous_time->point = StepTime::StepPoint::END;
        previous_time->rel = StepTime::StepRel::AT;
        action_type = "end";
    }

    if(DEBUG2)
    {

       std::stringstream ss;
       std::ostream &os(ss);

       active_action_p->getStep()->printName(os);
       std::string action = ss.str();



       if(active_action_p->getEndTime() != -1)
       {
         std::cout << std::fixed;
         std::cout << std::setprecision(2);
         std::cout << ";end - M [" << *timeTaken << "]: " << action;
         std::cout << "\n";
       }
       else
       {
         std::cout << std::fixed;
         std::cout << std::setprecision(2);
         std::cout << ";sta - M [" << *timeTaken << "]: " << action;
         std::cout << "\n";
       }

     }


    int taskId = active_action_p->getTask()->getID();

    //here we know that the conditions are satisfied, we can merge effects
    mergeEffects(realPlan, active_action_p, active_id, realplan);

    //effects can cause that some links might stop become satisfied
    makeLinksUnsat(active_action_p->getStep()->getId(), taskId, action_type);

    //check if there is a copy of the active_action
    std::pair<int,int> pta = std::pair<int,int>(taskId,active_action_p->getStep()->getId());
    std::map<std::pair<int,int>, std::pair<int,int> >::iterator it = similar_actions.find(pta);

    if(it!=similar_actions.end())//there exists a similar action
    {
        if(it->second.first >=0) //it is not temporal task
        {
            std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(it->second.first)->getPlan()->getSteps();

            for(unsigned int i =0; i<sv.size();i++)
            {
                if(sv.at(i).first->getId() == it->second.second)
                {
                    if(active_action_p->getEndTime()==-1)//start action
                    {
                      sv.at(i).second = 0; //indicate is being merged
                      tasks->at(it->second.first)->getPlan()->setSteps(sv);
                      break;
                    }
                    else //the end action will be taken care in satisActions
                    {
                        break;
                    }
                }
            }
        }
    }

    //TODO I might need to change this if I will use temporal task
    if (taskId >=0) //action has come from origin tasks
    {
        //I need to get the original task!!
       std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(active_action_p->getTask()->getID())->getPlan()->getSteps();

       for(unsigned int i =0; i<sv.size();i++)
       {
           if(sv.at(i).first->getId() == active_action_p->getStep()->getId())
           {
               if(active_action_p->getEndTime()==-1)//start action
               {
                 sv.at(i).second = 0; //indicate is being merged
                 tasks->at(active_action_p->getTask()->getID())->getPlan()->setSteps(sv);
                 break;
               }
               else //the end action will be taken care in satisActions
               {
                   break;
               }
           }
       }
     }


    // even thought we dont have any satisActions, the effects can still cause that
    //some links will become satisfied!!

    //from_id, to_id, condition, taskId
    std::vector<std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> > linksToChange;

    //TODO if I will start to reuse the temporal tasks, this needs to be addressed
    if(taskId > -1)
    {
        std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> fakeLink;
        fakeLink.first.first.first = active_action_p->getStep()->getId();
        fakeLink.first.first.second = -1;
        fakeLink.first.second = "";
        fakeLink.second = taskId;
        linksToChange.push_back(fakeLink);
     }
    //check if there is a copy of the active_action
    if(it!=similar_actions.end()&&(it->second.first>-1))//there exists a similar action
    {
        std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> fakeLink;
        fakeLink.first.first.first = it->second.second; //action_id
        fakeLink.first.first.second = -1;
        fakeLink.first.second = "";
        fakeLink.second = it->second.first; //taskid
        linksToChange.push_back(fakeLink);
    }


    //first take care about active_action_p as it has swapped getEndTime comparing
    //to the rest of the active_actions

    std::shared_ptr< std::vector< std::shared_ptr<StepTimed> > > satisActions(new std::vector< std::shared_ptr<StepTimed> >() );

    int ind_act = std::distance(active_list->begin(),std::find(active_list->begin(),active_list->end(),active_action_p));

    //this was taken from canBeMerged
    if(active_action_p->getEndTime()==-1) //start action
    {
        //we want to add "end" actions to the end_list, ha easier - just set the endtime and not delete it
        active_action_p->setEndTime((active_action_p->getStep()->getDuration()+*timeTaken));
        //timeTaken hasnot been changed, because start action doesnt take anything
    }
    else //it is end action
    {
        if(*timeTaken != active_action_p->getEndTime())
        {
            std::cerr << "main::canBeMerged:wrong time to merge end effect\n";
            throw;
        }

        satisActions->push_back(active_action_p);
        active_action_p.reset();
        if(realplan) //only when the global plan is called, delete //TODO this might change
          active_list->erase(active_list->begin()+ind_act);
        ind_act = -1;

    }



    //deleting satisfied active_actions


  //if(realplan)
  //{
    //deleting satisfied active_links



    for(unsigned int i=0; i< active_links->size();i++)
    {

        std::string domain = active_links->at(i)->second->getDomain();
        std::string action = active_links->at(i)->first->getCondition();
        //here changed on 03/10
        if(areConditionsSatisfiedNow(action, "all", domain)==1)
        {
            //make the link satisfied in the task
            //int taskId = active_links->at(i)->first->getTaskId();
            int taskId = active_links->at(i)->second->getID();
            if (taskId > -1)
            {
                std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> fakeLink;
                fakeLink.first.first.first = active_links->at(i)->first->getFromId();
                fakeLink.first.first.second = active_links->at(i)->first->getToId();
                fakeLink.first.second = action;
                fakeLink.second = taskId;
                linksToChange.push_back(fakeLink);
            }

            active_links->erase(active_links->begin()+i);
            i--; //because the size has changed
        }
    }


    for(int j=0;j<satisActions->size();j++)
    {
        //if(satisActions->at(j)->getEndTime()==-1) //IMPORTANT - canBeMerged set the endtime to -1 for the end actions
            //the start action would have the deadline of the action
        //{

            int taskId = satisActions->at(j)->getTask()->getID();
            //checking used actions from the tasks
            pta = std::pair<int,int>(taskId,satisActions->at(j)->getStep()->getId());
            it = similar_actions.find(pta);

            if(it!=similar_actions.end()&&(it->second.first>-1))
            {
                std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(it->second.first)->getPlan()->getSteps();

                for(unsigned int i =0; i<sv.size();i++)
                {
                    if(sv.at(i).first->getId() == it->second.second)
                    {

                        sv.at(i).second = 1;
                        tasks->at(it->second.first)->getPlan()->setSteps(sv);
                        break;
                    }
                }

                //check if satisfied action hasnt satisfied some links

                std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> fakeLink;
                fakeLink.first.first.first = it->second.second; //action_id
                fakeLink.first.first.second = -1;
                fakeLink.first.second = "";
                fakeLink.second = it->second.first; //taskid
                linksToChange.push_back(fakeLink);
                //TODO should I delete the similar actions or not?
                //similar_actions.erase(it);

            }
            if (taskId >=0) //action has come from origin tasks
            {
                //I need to get the original task!!
               std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(taskId)->getPlan()->getSteps();

               for(int i =0; i<sv.size();i++)
               {
                   if(sv.at(i).first->getId() == satisActions->at(j)->getStep()->getId())
                   {

                       sv.at(i).second = 1;
                       tasks->at(taskId)->getPlan()->setSteps(sv);
                       break;
                   }
               }

               //check is satisfied action hasnt satisfied also some link
               std::pair<std::pair<std::pair<size_t,size_t>,std::string>, int> fakeLink;
               fakeLink.first.first.first = satisActions->at(j)->getStep()->getId();
               fakeLink.first.first.second = -1;
               fakeLink.first.second = "";
               fakeLink.second = taskId;
               linksToChange.push_back(fakeLink);


            }
       //}
    }

    //change the links
    makeLinksSat(linksToChange);

    //we need to delete such actions from active_list which dont have satisfied precond
    for(int i = active_list->size()-1;i>=0;i--)
    {
        std::string literal = "(and ";
        std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = active_list->at(i)->getStep()->getConditions();
        for(unsigned int k = 0; k < condi.size(); k++)
        {
          literal += condi.at(k).first.first.second;
          for(unsigned int j=0; j< condi.at(k).first.second.size();j++)
          {
            literal += " ";
            literal += active_list->at(i)->getStep()->getParameters().at(condi.at(k).first.second.at(j));

          }
          literal += condi.at(k).second;
        }
        literal += ")";

        std::string action = literal;
        std::string domain =active_list->at(i)->getTask()->getDomain();
        //it is end action
        if(active_list->at(i)->getEndTime()>-1)
        {
           if(areConditionsSatisfiedNow(action,"over all",domain)==0)
            {
                std::cerr<<"mergeAction::ERROR:the overall condition doesnt hold for all the action\n";
                //TODO maybe this doesnt have to be exception, but merge fail->backtrack?
                throw;
            }
        }
        //if it is start action
        if(active_list->at(i)->getEndTime()==-1)
        {
            //if at least one of the type of condition is not valid, change it
            if((areConditionsSatisfiedNow(action,"",domain)==0)
                ||(areConditionsSatisfiedNow(action,"over all",domain)==0))
            {
                active_list->erase(active_list->begin()+i);
                //I probbably dont need to adjust the i, as I iterate backward
            }
        }
    }


    //TODO not all actions from the task needs to be used - I think this is already address in mergeAction,
    //but I need to check

//}
//end of if(realplan)



    return true;
}

static std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > removeLinksandOrderings(std::shared_ptr<FinalPlan> realPlan, bool test)
{

   std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >  lastAc = realPlan->removeLastStep(test);
   //we need to remove the full action, that is the only way how to backtrack in time

   if(test) //I just want to get the action, not really deleting it
   {
       return lastAc;
   }


   //TODO this method can be done smarter, ie searching for the effects of the removed action
   realPlan->updateAchiDel(lastAc.second);

   //update orderings only when the start action has been removed, at the ordering removes both instances
   if(lastAc.second.first=="start")
   {
     std::shared_ptr<const TemporalOrderings> to = realPlan->getOrderings();
     if(to == NULL)
     {
         std::cerr<< "empty orderings\n";
         throw;
     }
     to.reset(to->removeLastStep(lastAc.second.second));
     if(to == NULL)
     {
         std::cerr<< "empty orderings\n";
         throw;
     }
     realPlan->setOrderings(to);
   }

   //remove all links with to_id = lastAc id and corresponding time

   std::shared_ptr<std::vector<std::shared_ptr<MyLink> > > vecL = realPlan->getLinks();
   for (unsigned int i=0; i<vecL->size(); i++)
   {
       //if the link is to removed id and to the end instant
       if((vecL->at(i)->getToId()== lastAc.second.second)&&(lastAc.second.first=="end"))
       {
           //if the link is to the end, remove it
           if(vecL->at(i)->getConditionTime()=="at end")
           {
             vecL->erase(vecL->begin()+i);
             i--;
           }
           else if (vecL->at(i)->getConditionTime() =="over all")
           {
               vecL->at(i)->setBeingThreaten(true); //might get threaten again
           }
       }
       //if the link is to removed id and to the start instant
       //remove any link, as the step is completely removed
       if((vecL->at(i)->getToId()== lastAc.second.second)&&
           (lastAc.second.first=="start"))
       {
           vecL->erase(vecL->begin()+i);
           i--;
       }

   }

   return lastAc;



}

/*
 * -1 = opposite order than link -> plan not found
 * 0 = nothing happened
 * 1 = we added needed orderings, proceeed
 * 2 = we need to backtrack one more
 */
static int  removeMainStepsOrderings(std::shared_ptr<FinalPlan> realPlan, std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > removedAct, std::shared_ptr<StepTimed> active_action_p, int type)
{

  int from_id = removedAct.first->getStep()->getId();
  int taskId = removedAct.first->getTask()->getID();
  std::string time_rem = removedAct.second.first;
  size_t second_index = removedAct.second.second;
  std::shared_ptr<StepTimed> removed_action = removedAct.first;

  int num_steps_previous1=0;
  int num_steps_previous2=0;

  bool t1 =false;
  bool t2 = false;

  if(taskId<0)
  {
      taskId = removedAct.first->getTask()->getParentTask();
      from_id = removedAct.first->getTask()->getParentStepId();
      time_rem = "start";
      //second_index = //index is the global index int he final plan, no need to change it
      //removed_action = //TODO new, I dont know about this

      //return 0;
  }

  int active_task_id;
  int active_step_id;
  std::string time;

  if(active_action_p->getTask()->getID()<0)
  {
      active_task_id = active_action_p->getTask()->getParentTask();
      active_step_id = active_action_p->getTask()->getParentStepId();
      time="start";
  }
  else
  {
      active_task_id = active_action_p->getTask()->getID();
      active_step_id = active_action_p->getStep()->getId();
      if(active_action_p->getEndTime()==-1)
          time="start";
      else
          time="end";
  }

  for(unsigned int i=0; i< tasks->size();i++)
  {
      if(!t1)
      {
          if(tasks->at(i)->getID()!=taskId)
          {
              num_steps_previous1+= tasks->at(i)->getPlan()->num_steps();
          }
          else
          {
              t1=true;
          }
      }
      if(!t2)
      {
          if(tasks->at(i)->getID()!=active_task_id)
          {
              num_steps_previous2+= tasks->at(i)->getPlan()->num_steps();
          }
          else
          {
              t2=true;
          }
      }
      if(t1&&t2)
          break;
  }

  int id1 = (from_id*2)+(2*num_steps_previous1); //the removed action
  int id2 = (active_step_id*2)+(2*num_steps_previous2); //the active action which failed to be merged
  if(time=="start")
      id2--;
  id2--;

  if(time_rem == "start")
  {
      id1--;
  }
  id1--; //the end is -1, start -2

  if(id1 == id2)
  {
      //the action which fails has come from the same task and same step as the action being removed
      //ie remove more!
      return 2;
  }
  int amount_of_main_steps=0;
  for(unsigned int j=0; j <realPlan->amountOfSteps();j++)
  {
      if(realPlan->getStepwithGIndex(j).first->getTask()->getID()>-1)
          amount_of_main_steps++;
  }

  for(unsigned int i=0; i < realPlan->getNumMainSteps(); i++)
  {
      //if it is type 3 (backtracking to the beggining and this is the last removed action
      //mark that it must come after some of the others
      //if amount_of_main steps is zero, we want to propagate that this choice needs to come after
      //if((type == 3)&&
      if(amount_of_main_steps==0)
      {
        realPlan->setStepsOrderings(id1,i,2,false);
      }
      else
      {
          if(abs(realPlan->getStepsOrderings(id1,i))!=2)
          {
             realPlan->setStepsOrderings(id1,i,INT_MAX, false);
          }
      }
  }

  //if((type == 3)&&
  if(amount_of_main_steps==0)
  {
      //clean up => delete any remaining 5 or 1
      for(unsigned int i=0; i < realPlan->getNumMainSteps(); i++)
      {
          for(unsigned int j=0; j < realPlan->getNumMainSteps();j++)
          {
              if(abs(realPlan->getStepsOrderings(i,j))==5)
              {
                  //overwrite 5
                  realPlan->setStepsOrderings(i,j, int(realPlan->getStepsOrderings(i,j)/5), true);

              }
              if(abs(realPlan->getStepsOrderings(i,j))==1)
              {
                   realPlan->setStepsOrderings(i,j, INT_MAX, false);
              }
          }
      }
      //set that this cannot go first again
      realPlan->setCanComeFirst(num_steps_previous1+from_id-1,false);
      if(DEBUG)
      {
          std::cout<< "--------------------------\n";
          std::ostream &os = std::cout;

          int taskId_l=0;
          int stepId_l=0;

          for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

             os << taskId_l <<"_"<<stepId_l << ": ";
             if(r%2 == 1)
             {
               os << "end: ";
               stepId_l++;
             }
             else
             {
               os<< "sta: ";
             }
             if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
             {
                 taskId_l++;
                 stepId_l=0;
             }
            for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
              os.width(7);
              int d;
              d = realPlan->getStepsOrderings(r,c);

              if (d < INT_MAX) {
            os << d;
              } else {
            os << "inf";
              }
            }
            os << std::endl;
          }
          std::cout << "-------------------------\n";
          for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
          {
              std::cout << realPlan->getCanComeFirst(k);
              std::cout << " ";
          }
          std::cout << "\n";
      }
      return -1;
  }


  if(DEBUG)
  {
      std::cout << "----------------\n";
      std::ostream &os = std::cout;

      int taskId_l=0;
      int stepId_l=0;

      for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

         os << taskId_l <<"_"<<stepId_l << ": ";
         if(r%2 == 1)
         {
           os << "end: ";
           stepId_l++;
         }
         else
         {
           os<< "sta: ";
         }
         if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
         {
             taskId_l++;
             stepId_l=0;
         }
        for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
          os.width(7);
          int d;
          d = realPlan->getStepsOrderings(r,c);

          if (d < INT_MAX) {
        os << d;
          } else {
        os << "inf";
          }
        }
        os << std::endl;
      }
      std::cout << "-------------------------\n";
      for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
      {
          std::cout << realPlan->getCanComeFirst(k);
          std::cout << " ";
      }
      std::cout << "\n";
  }

  if (type == 4) //we just want to remove ordering, not add a new one
  {
      return 1;
  }
  if(realPlan->getStepsOrderings(id1,id2)==INT_MAX)
  {
      realPlan->setStepsOrderings(id2,id1,-5, false);
      if(DEBUG)
      {
          std::cout << "----------------\n";
          std::ostream &os = std::cout;

          int taskId_l=0;
          int stepId_l=0;

          for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

             os << taskId_l <<"_"<<stepId_l << ": ";
             if(r%2 == 1)
             {
               os << "end: ";
               stepId_l++;
             }
             else
             {
               os<< "sta: ";
             }
             if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
             {
                 taskId_l++;
                 stepId_l=0;
             }
            for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
              os.width(7);
              int d;
              d = realPlan->getStepsOrderings(r,c);

              if (d < INT_MAX) {
            os << d;
              } else {
            os << "inf";
              }
            }
            os << std::endl;
          }
          std::cout << "-------------------------\n";
          for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
          {
              std::cout << realPlan->getCanComeFirst(k);
              std::cout << " ";
          }
          std::cout << "\n";
      }
      return 1;
  }
  else
  {
     if(realPlan->getStepsOrderings(id1,id2)==-10)// ie id1 < id2 because od the link, or start/end action
     {
         //active_action < removed one
         //but we need id2 < id1, this meands that id1 must become sooner -> we need to backtrack again

         //we need to find the last before this
         int index = second_index-1;

         //TODO NEW twice minus -1 ???
         index--; //to get the previous
         if(index>= realPlan->amountOfSteps())
         {
             std::cerr << "removeMainSteps:: ERROR: wrong index\n";
             throw;
         }
         std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > lastStep = realPlan->getStepwithGIndex(index);
         if(lastStep.second.first!="none")
         {
            while(lastStep.first->getTask()->getID()<0)
            {
                index--;
                lastStep = realPlan->getStepwithGIndex(index);
                //if there is no step, there is no ordering to update
                if(lastStep.second.first=="none")
                    return -1;
            }
         }
         else
             return -1;

         //removed action must come sooner

         //make a copy of the removedAct
         std::shared_ptr<StepTimed> new_act( new StepTimed(*removed_action));
         if(time_rem == "start")
            new_act->setEndTime(-1);

         return removeMainStepsOrderings(realPlan, lastStep, new_act, type);


     }
     //TODO need to test this part
     else if(abs(realPlan->getStepsOrderings(id1,id2))==5) //id1 < id2 but now we want id2 < id1, backtrack again
     {
         //we need to find the last before this

         int index = second_index-1;
         //indexes are one higher, ie get one lower, to get the pointing index
         index--;

         if(index>= realPlan->amountOfSteps())
         {
             std::cerr << "removeMainSteps:: ERROR: wrong index\n";
             throw;
         }
         std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > lastStep = realPlan->getStepwithGIndex(index);
         if(lastStep.second.first!="none")
         {
            while(lastStep.first->getTask()->getID()<0)
            {
                index--;
                lastStep = realPlan->getStepwithGIndex(index);
                //if there is no step, there is no ordering to update
                if(lastStep.second.first=="none")
                    return -2;
            }
         }
         else
             return -2;

         //removed action must come sooner

         //make a copy of the removedAct
         std::shared_ptr<StepTimed> new_act( new StepTimed(*removed_action));
         if(time_rem == "start")
            new_act->setEndTime(-1);

         //lastStep must be after id1 or id2

         int num_steps_previous3 = 0;
         for(unsigned int i=0; i< tasks->size();i++)
         {
                 if(tasks->at(i)->getID()!=lastStep.first->getTask()->getID())
                 {
                     num_steps_previous3+= tasks->at(i)->getPlan()->num_steps();
                 }
                 else
                 {
                    break;
                 }
         }

         from_id = lastStep.first->getStep()->getId();
         int id3 = (from_id*2)+(2*num_steps_previous3); //the removed action

         if(lastStep.second.first == "start")
         {
             id3--;
         }
         id3--; //the end is -1, start -2

         //lastStep must be after id1 or id2
         realPlan->setStepsOrderings(id1,id3,-2,false);
         realPlan->setStepsOrderings(id2,id3,-2,false);

         //but also remove the num 5 between id1 and id2
         realPlan->setStepsOrderings(id1,id2,INT_MAX,true);

         if(DEBUG)
         {
             std::cout<< "--------------------------\n";
             std::ostream &os = std::cout;

             int taskId_l=0;
             int stepId_l=0;

             for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

                os << taskId_l <<"_"<<stepId_l << ": ";
                if(r%2 == 1)
                {
                  os << "end: ";
                  stepId_l++;
                }
                else
                {
                  os<< "sta: ";
                }
                if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
                {
                    taskId_l++;
                    stepId_l=0;
                }
               for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
                 os.width(7);
                 int d;
                 d = realPlan->getStepsOrderings(r,c);

                 if (d < INT_MAX) {
               os << d;
                 } else {
               os << "inf";
                 }
               }
               os << std::endl;
             }
             std::cout << "-------------------------\n";
             for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
             {
                 std::cout << realPlan->getCanComeFirst(k);
                 std::cout << " ";
             }
             std::cout << "\n";
         }
         return 2;


     }
     else
     {
         std::cout << realPlan->getStepsOrderings(id1,id2) << "\n";
         std::cerr << "removeMainStepsOrderings::ERROR: unexpected value in the orderings\n";
         throw;
     }
  }

}

static void unmergeEffects(std::shared_ptr<FinalPlan> realPlan, std::shared_ptr<StepTimed> active_action_p, std::string time)
{
    //time, predicate parameters end brackets
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > effL = active_action_p->getStep()->getEffects();

    std::string domain = active_action_p->getTask()->getDomain();

    for (unsigned int i =0; i< effL.size();i ++)
    {
        /*use only those effect which rely if we are merge start or end of the action*/
        if((time=="start")&&(effL.at(i).first.first.first != "at start"))
        //we want start effects, but this is not one, do nothing
        {
            continue;
        }
        if((time=="end")&&(effL.at(i).first.first.first != "at end"))
        //we want end effects, but this is not one, do nothing
        {
            continue;
        }

        //we got correct effect, put it to the current state
        std::string literal;
        literal = effL.at(i).first.first.second;
        for(unsigned int k=0; k< effL.at(i).first.second.size();k++)
        {
            literal += " ";
            literal += active_action_p->getStep()->getParameters().at(effL.at(i).first.second.at(k));
        }
        literal += effL.at(i).second;

        std::string effect = literal;


        //test if the effect holds
        //what time??? the effect should hold no matter time
        //changed here on 03/10 from "" to "all"
        if(areConditionsSatisfiedNow(effect, "all", domain)==1)
        {
            if(effect.find(("(not"))!=std::string::npos) //it is negative
            {
                std::string c_effect = effect.substr(5, effect.size()-5-1);

                std::map< std::string, std::vector<std::pair<std::string, int> > >::iterator it_del = realPlan->deleters.find(c_effect);
                std::map< std::string, std::vector<std::pair<std::string, int> > >::iterator it_ach = realPlan->achievers.find(c_effect);

                std::vector<std::pair<std::string, int> >  achi;
                std::vector<std::pair<std::string, int> >  dele;

                StepTime step_timeo_achi;
                StepTime step_timeo_del;

                if(it_ach!=realPlan->achievers.end())
                {
                  achi = it_ach->second;
                  if(it_ach->second.back().first == "at start")
                  {
                      step_timeo_achi = StepTime::AT_START;
                  }
                  else if (it_ach->second.back().first == "at end")
                  {
                      step_timeo_achi = StepTime::AT_END;
                  }
                  else
                  {
                     std::cerr << "unMergeEffects::wrong achiever steptime\n";
                     throw;
                  }
                }
                if(it_del!=realPlan->deleters.end())
                {
                  dele = it_del->second;
                  if(it_del->second.back().first == "at start")
                  {
                      step_timeo_del = StepTime::AT_START;
                  }
                  else if (it_del->second.back().first == "at end")
                  {
                      step_timeo_del = StepTime::AT_END;
                  }
                  else
                  {
                     std::cerr << "unMergeEffects::wrong achiever steptime\n";
                     throw;
                  }
                }

                //if the achiever is after the deleter, I need to add the positive
                //else dont add anything
                   for( int j = current_state[domain].size()-1; j>=0; j--)
                   {
                       if(current_state[domain].at(j)==effect)
                       {
                           if((it_del != realPlan->deleters.end())&&(it_del->second.size()>0)) //we have found a deleter
                           {
                               if(it_ach != realPlan->achievers.end())//we have some achiever
                               {
                                   if(realPlan->getOrderings()->possibly_after(it_ach->second.back().second, step_timeo_achi, it_del->second.back().second,  step_timeo_del))
                                   {
                                       current_state[domain].erase(current_state[domain].begin()+j);
                                       current_state[domain].push_back(c_effect);
                                   }
                                   else
                                   {
                                       //nothing
                                   }

                               }
                               else //there is a previous deleter and no achiever, keep it
                               {
                                   //do nothing
                               }

                           }
                           else //there is no previous deleter
                           {
                             if(it_ach  == realPlan->achievers.end()) //there is no previous achiever, hence, the atom was not specified before this
                             //just remove it from the current state
                             {
                                current_state[domain].erase(current_state[domain].begin()+j);
                             }
                             else //there is previous achiever, add the positive thing
                             {
                                current_state[domain].erase(current_state[domain].begin()+j);
                                current_state[domain].push_back(c_effect);
                             }

                           }
                           break;
                       }
                  }

            }
            else //the effect is possitive
            {
                std::map< std::string, std::vector<std::pair<std::string, int> > >::iterator it_del = realPlan->deleters.find(effect);
                std::map< std::string, std::vector<std::pair<std::string, int> > >::iterator it_ach = realPlan->achievers.find(effect);

                std::vector<std::pair<std::string, int> >  achi;
                std::vector<std::pair<std::string, int> >  dele;

                StepTime step_timeo_achi;
                StepTime step_timeo_del;

                if(it_ach!=realPlan->achievers.end())
                {
                  achi = it_ach->second;
                  if(it_ach->second.back().first == "at start")
                  {
                      step_timeo_achi = StepTime::AT_START;
                  }
                  else if (it_ach->second.back().first == "at end")
                  {
                      step_timeo_achi = StepTime::AT_END;
                  }
                  else
                  {
                     std::cerr << "unMergeEffects::wrong achiever steptime\n";
                     throw;
                  }
                }
                if(it_del!=realPlan->deleters.end())
                {
                  dele = it_del->second;
                  if(it_del->second.back().first == "at start")
                  {
                      step_timeo_del = StepTime::AT_START;
                  }
                  else if (it_del->second.back().first == "at end")
                  {
                      step_timeo_del = StepTime::AT_END;
                  }
                  else
                  {
                     std::cerr << "unMergeEffects::wrong achiever steptime\n";
                     throw;
                  }
                }

                //if the achiever is after the deleter, I need to add the positive
                //else dont add anything



                   for( int j = current_state[domain].size()-1; j>=0; j--)
                   {
                       if(current_state[domain].at(j)==effect)
                       {
                           if(it_ach != realPlan->achievers.end())//we have some achiever
                           {
                               if((it_del != realPlan->deleters.end())&&(it_del->second.size()>0)) //we have found a deleter
                               {
                                   //if the deleter is after the achiever, I need to add the negative
                                   //else dont add anything
                                   if(realPlan->getOrderings()->possibly_after(it_del->second.back().second, step_timeo_del, it_ach->second.back().second,  step_timeo_achi))
                                   {
                                       current_state[domain].erase(current_state[domain].begin()+j);
                                       current_state[domain].push_back("(not "+ effect+")");
                                   }
                                   else
                                   {
                                       //nothing
                                   }

                               }
                               else //there is a previous achiever and no deleter, keep it
                               {
                                   //do nothing
                               }

                           }
                           else //there is no previous achiever
                           {
                             if(it_del  == realPlan->deleters.end()) //there is no previous deleter, hence, the atom was not specified before this
                             //just remove it from the current state
                             {
                                current_state[domain].erase(current_state[domain].begin()+j);
                             }
                             else //there is previous deleter, add the negative thing
                             {
                                current_state[domain].erase(current_state[domain].begin()+j);
                                current_state[domain].push_back("(not "+effect+")");
                             }

                           }
                           break;
                       }
                  }
            }
        }

    }


}

static bool backtrack(double * timeTaken, std::shared_ptr<FinalPlan> realPlan, std::shared_ptr< std::vector< std::shared_ptr<StepTimed> > > active_list,
                       std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links,
                       size_t * previous_id,std::shared_ptr<StepTime> previous_time,
                      int parentTaskId, int parentStepId, int back_id, int type, std::shared_ptr<StepTimed> active_action_p)
{


    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > removedAct;
    if(back_id != -1)
    {
        //remove from the final plan - call in test mode
        removedAct = removeLinksandOrderings(realPlan, true);

        if(removedAct.second.first == "none") //there is nothing more to remove
        {
            //there is no more slave actions, but the original backtrack was satisfied->hence, true
            return true;
        }
    }
    else //called from merging
    {
        removedAct = removeLinksandOrderings(realPlan, false);
    }

    if((type == 3)&&(removedAct.second.first == "none"))
        //we have reached the beginning
    {
        return true;
    }
    if((type != 3)&&(removedAct.second.first == "none"))
    {
        //TODO MEGA, is this false correct to do? When the algortihm should stop?
        return true; //change on 4/12 -> if we reach beginning that is allright
        //the alg should return false only when the orderings are wrong
    }
    //we are removing temporary actions belonging to some original step
    if((parentTaskId!=-1)&&(removedAct.first->getTask()->getParentTask() != parentTaskId))
    {
        //this action is not a kid of previoussly removed action
        if(type == 1) //the call came from the original action to remove all its preconditions actions
            return true; //
        if(type == 2) //we have removed joining actions of some actions, which couldnt be merged
            //hence we want to remove the last original step and push the orderings
        {
            //nothing
        }
    }
    else
    {
        if((parentStepId!=-1)&&(removedAct.first->getTask()->getParentStepId() != parentStepId))
        {
            if(type ==1)
              return true;
        }
    }
    if(back_id != -1)
    {
        //this couldnt be done above, as the backtrack can quit
        active_list->at(back_id)->setBacktrack(false);
        removedAct = removeLinksandOrderings(realPlan, false);
    }

    std::stringstream ss;
    std::ostream &os(ss);
    removedAct.first->getStep()->printName(os);
    std::string action = ss.str();
    number_of_backtracks++;

    //if(removedAct.second.first=="end")
    //{
        //get the step before, do not delete, ie -> true
        std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >  lastAc = realPlan->removeLastStep(true);
        if(lastAc.second.first == "none")
        {
            //*timeTaken = 0; not zero! when I have some other release dates, this is wrong
        }
        else
        {
            //there is action before, reverse to that time
            if(lastAc.second.first == "start") //it is a start action
            {
              *timeTaken = lastAc.first->getStartTime();//*timeTaken - removedAct.first->getEndTime();//getStep()->getDuration();
            }
            else //it is end action
            {
                *timeTaken = lastAc.first->getEndTime();
            }
        }
    /*}
    else //it is start
    {
        //30/10 fixed orderings so this shouldnt be needed anymore
        //nope, still needs this
        *timeTaken -= incr;
    }*/

    if(DEBUG2)
    {

      if(removedAct.second.first=="end")
      {
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << ";end - B [" << *timeTaken << "]: " << action;
        std::cout << "\n";
      }
      else
      {
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << ";sta - B [" << *timeTaken << "]: " << action;
        std::cout << "\n";
      }

      if(*timeTaken <=22)
      {
          std::cout << "pause\n";
      }
    }

    //removed any stepsOrderings
    bool backtrack_to_beginning =false;
    bool backtrack_again = false;

    int order = removeMainStepsOrderings(realPlan, removedAct,active_action_p, type);
    if(order == -1)
    {
        //all the steps orderings are not valid in this order, backtrack to beginning
        //and propagate that the first choice is wrong
        backtrack_to_beginning = true;
    }
    else if(order == -2)
    {
        return false; //there is no solution
    }
    else if (order ==2)
    {
        backtrack_again =true;
    }

    if(type==3)
        backtrack_to_beginning = true;

    if(type==4) //we want to backtrack to a specific action
    {
        if((removedAct.first->getTask()->getID()==parentTaskId)&&(removedAct.first->getStep()->getId()==parentStepId))
        {
            backtrack_again = false;
        }
        else
            backtrack_again = true;
    }

    //order 0 or 1 - everything allright, proceed

    //remove from the current state
    unmergeEffects(realPlan, removedAct.first,removedAct.second.first);

    if(removedAct.first->getTask()->getID() >= 0)
    {
        //change status of the action in the original plan
        //std::vector<std::pair<std::pair<size_t,float>,int> >
        std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > start_vec = tasks->at(removedAct.first->getTask()->getID())->getPlan()->getSteps();

        for (unsigned int j =0; j < start_vec.size(); j++)
        {

            //i need to comparing the original ids
          if(start_vec.at(j).first->getId() == removedAct.first->getStep()->getId())
          {
              if(removedAct.second.first == "end")
              {
                  start_vec.at(j).second = 0; //being merged
              }
              else //it is the start
              {
                  start_vec.at(j).second = -1; //not merged yet
              }
          }

        }
        tasks->at(removedAct.first->getTask()->getID())->getPlan()->setSteps(start_vec);
    }

    //check if there is a copy of the removed_action
    std::pair<int,int> pta = std::pair<int,int>(removedAct.first->getTask()->getID(),removedAct.first->getStep()->getId());
    std::map<std::pair<int,int>, std::pair<int,int> >::iterator it = similar_actions.find(pta);

    if(it!=similar_actions.end()&&(it->second.first>-1))//there exists a similar action
    {
        std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > sv = tasks->at(it->second.first)->getPlan()->getSteps();

        for(unsigned int i =0; i<sv.size();i++)
        {
            if(sv.at(i).first->getId() == it->second.second)
            {
                if(removedAct.second.first == "end")
                {
                  sv.at(i).second = 0; //indicate is being merged
                  tasks->at(it->second.first)->getPlan()->setSteps(sv);
                  break;
                }
                else //the start action
                {
                    sv.at(i).second = -1; //not merged yet
                    tasks->at(it->second.first)->getPlan()->setSteps(sv);
                    break;
                }
            }
        }
    }

    //some original links might change
    for(unsigned int i = 0; i< tasks->size();i++) //throw all tasks
    {
        std::vector<std::pair<std::shared_ptr<MyLink>, int > >  links_vec=tasks->at(i)->getPlan()->getLinks();
        for (unsigned int k = 0; k < links_vec.size(); k++)
        {

          std::string domain = tasks->at(i)->getDomain();

          std::string condition = links_vec.at(k).first->getCondition();

          if(links_vec.at(k).second == 1) //if they are satisfied, they might change
          {
              changeGeneralLinks(&links_vec.at(k), condition, i, domain,0);
          }
          else //they are not satisfied, they might become
          {
              changeGeneralLinks(&links_vec.at(k), condition, i, domain,1);
          }
        }
        tasks->at(i)->getPlan()->setLinks(links_vec);
    }



    int return_back_id;
    if(removedAct.second.first=="start")
    {
        //the action should be in the active_list -> change deadline, make it again as Start action
        bool actFound = false;
        for(int i = active_list->size()-1; i>=0;i--)
        {

            //if the steps are same and belong to same task
            if((*active_list->at(i)->getStep()==*removedAct.first->getStep())&&
              (active_list->at(i)->getTask()->getID() == removedAct.first->getTask()->getID())
             )
            {
                //if it is a start action, the action is still in the active_list, with the set deadline
                //I can either put no deadline there,
                active_list->at(i)->setEndTime(-1);
                if (order == 2)
                {
                    //we pushed new ordering, the removed action can be chosen actually again, do not set backtrack flag
                    active_list->at(i)->setBacktrack(false);
                }
                else
                   active_list->at(i)->setBacktrack(true);

                return_back_id=i;
                actFound = true;
            }
        }
        if(!actFound)
        {
            std::cerr << "backtrack::ERROR:action was not found in the active list\n";
            throw;
        }
        g_id--; //make smaller as this assigns ids to actions
    }
    else //it is an end action
    {
        if(removedAct.first->getEndTime()==-1) //this should be set to real deadline
        {
            std::cerr << "backtrack::ERROR:expecting set deadline but it is not\n";
            throw;
        }
        else
        {

            active_list->insert(active_list->begin(),removedAct.first);
            if(order == 2)
               active_list->at(0)->setBacktrack(false);
            else
               active_list->at(0)->setBacktrack(true);
            return_back_id=0;
            realPlan->unfinished_actions[removedAct.first] = removedAct.second.second;

        }
    }

    //find step before the removed one
    if(realPlan->amountOfSteps()>0) //there are steps
    {
        std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > lastAc = realPlan->getLastStep();
        *previous_id = lastAc.second.second;
        if(lastAc.second.first=="start")
        {
          previous_time->point = StepTime::StepPoint::START;
          previous_time->rel = StepTime::StepRel::AT;
        }
        else
        {
          previous_time->point = StepTime::StepPoint::END;
          previous_time->rel = StepTime::StepRel::AT;
        }
    }
    else //we are at the beginning
    {
        *previous_id = 0;
        previous_time->point = StepTime::StepPoint::START;
        previous_time->rel = StepTime::StepRel::AT;
    }


    //we need to delete such actions from active_list which dont have satisfied precond
    for(int k = active_list->size()-1;k>=0;k--)
    {
        std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = active_list->at(k)->getStep()->getConditions();
        std::string literal = "(and ";

        for(unsigned int i=0;i < condi.size();i++)
        {


            literal += "(";
            literal += condi.at(i).first.first.first;
            literal += " ";

            literal += condi.at(i).first.first.second;
            for(unsigned int j=0; j< condi.at(i).first.second.size();j++)
            {
                literal += " ";
                literal += active_list->at(k)->getStep()->getParameters().at(condi.at(i).first.second.at(j));

            }
            literal += condi.at(i).second;

            literal += ")";
        }
        literal+= ")";

        std::string action = literal;
        std::string domain =active_list->at(k)->getTask()->getDomain();
        //it is end action
        if(active_list->at(k)->getEndTime()>-1)
        {
           if((areConditionsSatisfiedNow(action,"at end",domain)==0)
                ||(areConditionsSatisfiedNow(action,"over all",domain)==0))
            {
                //I should "delete" the end action, ie. make it start action
                active_list->at(k)->setEndTime(-1);
                continue;
            }
        }
        //if it is start action
        if(active_list->at(k)->getEndTime()==-1)
        {
            //if at least one of the type of condition is not valid, change it
            if((areConditionsSatisfiedNow(action,"at start",domain)==0)
                ||(areConditionsSatisfiedNow(action,"over all",domain)==0))
            {
                active_list->erase(active_list->begin()+k);
                //I probbably dont need to adjust the i, as I iterate backward
            }
        }
    }


    std::vector<int> links_to_delete; //I cant delete throw the list, I will loose my indexes
    //hence, just remember which should be deleted and delete afterward
    for(unsigned int i = 0;i<active_links->size();i++)
    {
        //if I am removing start action, remove all links
        //if I am removing end action, remove just links with effects End
        if((removedAct.second.first=="start")&&(active_links->at(i)->first->getFromId()==removedAct.second.second))
        {
            links_to_delete.push_back(i);
            //active_links->erase(active_links->begin()+i);
        }
        if((removedAct.second.first=="end")&&(active_links->at(i)->first->getFromId()==removedAct.second.second)
                &&(active_links->at(i)->first->getEffectTime() == "at end"))
        {
            links_to_delete.push_back(i);
            //active_links->erase(active_links->begin()+i);
        }

        std::string domain = active_links->at(i)->second->getDomain();
        std::string condition = active_links->at(i)->first->getCondition();
        //if the active link is satisfied, delete it
        if(areConditionsSatisfiedNow(condition,"all",domain)==1)
        {
            links_to_delete.push_back(i);
            //active_links->erase(active_links->begin()+i);
        }

    }

    if(links_to_delete.size()>0)
    {
        std::sort(links_to_delete.begin(),links_to_delete.end());
        for(unsigned int i = 0;i<active_links->size();i++)
        {
            if(links_to_delete.size()==0)
            {
                break;
            }
            if(i == links_to_delete.at(0))
            {
                //descrese indexed
                std::transform (links_to_delete.begin(), links_to_delete.end(), links_to_delete.begin(), bind2nd(std::plus<int>(), -1));
                //delete all indexes which are smaller than the current index
                while(links_to_delete.at(0)<i)
                {
                    links_to_delete.erase(links_to_delete.begin());
                    if(links_to_delete.size()==0)
                        break;
                }

                active_links->erase(active_links->begin()+i);
            }
        }
    }

    if(backtrack_to_beginning)
    {
        return backtrack(timeTaken,realPlan, active_list, active_links, previous_id,previous_time,-1,-1,-1,3, active_action_p);

    }

    if(backtrack_again)
    {
        if(type != 4) //we havent request this before
        {
            //we want to remove the previous original action
            int index = removedAct.second.second-1;
            //indexes are one higher, ie get one lower, to get the pointing index
            index--;

            if(index>= realPlan->amountOfSteps())
            {
                std::cerr << "backtrack:: ERROR: wrong index\n";
                throw;
            }
            std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > lastStep = realPlan->getStepwithGIndex(index);
            if(lastStep.second.first!="none")
            {
               if(lastStep.first->getTask()->getID()<0)
               {
                   parentTaskId = lastStep.first->getTask()->getParentTask();
                   parentStepId = lastStep.first->getTask()->getParentStepId();
               }
               else
               {
                   parentTaskId = lastStep.first->getTask()->getID();
                   parentStepId = lastStep.first->getStep()->getId();
               }
            }
            else
                return true; //TODO is this correct?


            return backtrack(timeTaken,realPlan,active_list,active_links, previous_id,previous_time, parentTaskId ,parentStepId, return_back_id, 4, active_action_p);
         }
        else
        {
            //we have requested before, use the parentIds
            return backtrack(timeTaken,realPlan,active_list,active_links, previous_id,previous_time,  parentTaskId,parentStepId, return_back_id, 4, active_action_p);


        }

    }


    //if the removedAct is from an original task, remove all its actions satisfying precond
    //I cannot delete in between, I must from the end, so I just want to keep deleting
    //until I have an action from a temporary task with parentStepId equal to the removedAct id
    if(removedAct.first->getTask()->getID()>-1)
    {
        //TODO what should I do with the backtrack flag?
        return backtrack(timeTaken,realPlan,active_list,active_links, previous_id,previous_time,  removedAct.first->getTask()->getID(),removedAct.first->getStep()->getId(), return_back_id, 1, active_action_p);

    }
    //if the removedAct is from a temporary task then keep removing until the removedAct belongs to same task and step
    else
    {
        return backtrack(timeTaken,realPlan,active_list,active_links, previous_id,previous_time, removedAct.first->getTask()->getParentTask(),removedAct.first->getTask()->getParentStepId(), return_back_id, 2, active_action_p);
    }

    return true;
}

/* 09/09 checked
 * this method get one action to be merged and if it does not invalidate deadlines, it merges it
 * init_time = time from which to start_time
 * realPlan = the final plan
 */
static bool merging(double init_time,std::shared_ptr<FinalPlan> realPlan)
{

    int numUnUsed = 0;
    double smallestRelease;

    std::shared_ptr< std::vector< std::shared_ptr<StepTimed> > > active_list(new std::vector< std::shared_ptr<StepTimed> >() );
    std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > > > active_links(new std::vector< std::shared_ptr<std::pair<std::shared_ptr<MyLink>,std::shared_ptr<Task> > > >() );

    std::shared_ptr<std::vector<int> > back_ids (new std::vector<int>() );
    if(tasks->size()<=0)
    {
        std::cout << "no tasks\n";
        return true;
    }

    //pairs of release dates and indexes
    std::vector<std::pair<double, int> > releaseDates;
    for(unsigned int i=0; i<tasks->size();i++)
    {

        releaseDates.push_back(std::pair<double,int>(tasks->at(i)->getReleaseDate(),i));
        continue;

        numUnUsed++;

    }
    std::sort(releaseDates.begin(),releaseDates.end(),sortRelease);
    smallestRelease = releaseDates.at(0).first;
    init_time = smallestRelease;
    //std::cout << "size" << releaseDates.size();
    releaseDates.erase(releaseDates.begin());

    int run =0;
    bool newStart = true;
    int iteration = 0;
    size_t previous_id = 0;
    std::shared_ptr<StepTime> previous_time;
    previous_time.reset(new StepTime());
    previous_time->point = StepTime::StepPoint::START;
    previous_time->rel = StepTime::StepRel::AT;

    //in domains without release dates this loop should be done just once
    while (std::find_if(tasks->begin(), tasks->end(),taskUnused) < tasks->end()) // some tasks plans are still unsused
    {
        //TODO get away this when you will start to do deadlines
        /*if(run>0)
        {
            std::cerr << "main::merging:ERROR: running while loop moretimes\n";
            throw;
        }
        run++;*/

        //this fills the queues
        checkActiveActions(init_time, active_list,active_links);

        if ((active_list->size()==0)&&(active_links->size()==0)) //no time window is opened
        {

          if(releaseDates.size()>0)
          {
            smallestRelease = releaseDates.at(0).first;
            releaseDates.erase(releaseDates.begin());
            init_time = smallestRelease; //open the closes time window
            newStart = true;
            checkActiveActions(init_time, active_list,active_links);
          }
          else
          {
              std::cerr << "merging::ERROR:we run out of time windows to open\n";
              throw;
          }
        }

        int active_size = active_list->size() + active_links->size();
        std::shared_ptr<StepTimed> backtrack_a (NULL);
        while(active_size!=0)
        {

          std::shared_ptr<StepTimed> active_action_p;
          do{
              active_action_p = getAction(realPlan,active_list,active_links,&init_time, back_ids);

              if((previously_not_useful.size() != 0)&&(previously_not_useful.size() == (active_list->size()+active_links->size())))
              {
                  //if all are not useful, there is no solution
                  return false;
              }
          } while(active_action_p == NULL);

          //chechking deadlines of tasks
          //even though I need to check this just for end action, if this will be violated alredy in the start action
          //there is no point of merging it
          //if it is start action, check if it will violate deadline
          if(((active_action_p->getEndTime()==-1)&&((active_action_p->getStep()->getDuration()+init_time)>active_action_p->getTask()->getDeadline()))
            ||((active_action_p->getEndTime()!=-1)&&((init_time)>active_action_p->getTask()->getDeadline()))
              ||(active_action_p->getBacktrack()))
          {

              for(unsigned int i=0;i<back_ids->size();i++)
              {
                  active_list->at(back_ids->at(i))->setBacktrack(false);
                  back_ids->erase(back_ids->begin()+i);
                  i--;
              }
              if(!backtrack(&init_time,realPlan, active_list, active_links, &previous_id,previous_time,-1,-1,-1,0, active_action_p))
              {   //return false; //the backtrack failed->there is no plan*/
                  return false;
              }

          }
          else // I can merge
          {
              for(unsigned int i=0;i<back_ids->size();i++)
              {
                  active_list->at(back_ids->at(i))->setBacktrack(false);
                  back_ids->erase(back_ids->begin()+i);
                  i--;
              }



              int active_id;


              if(active_action_p->getEndTime() == -1) //START action
              {
                 g_id++;
                 active_id = g_id;
              }
              else
              {
                std::map< std::shared_ptr<StepTimed>, size_t>::iterator it = realPlan->unfinished_actions.find(active_action_p);
                if (it != realPlan->unfinished_actions.end()) //found something
                {
                   active_id = it->second;
                }
                else
                {
                  std::cerr << "main::merging:ERROR:Action is unfinished but it is not in the unfinished map\n";
                  throw;
                }
              }


                bool suc = mergeAction(realPlan,active_action_p,&init_time,active_links,active_list,active_id,true,&previous_id, previous_time, newStart);
                if(!suc)
                {
                    return false;
                }
              //}
           }//end of merge

           checkActiveActions(init_time, active_list,active_links);
           active_size = active_list->size()+active_links->size();
           checkTaskBeUsed();
           iteration++;
           newStart = false;


        }//end of while(active_size!=0)

    }

    return true;

}


static void readConfig(std::shared_ptr<Task> task,std::shared_ptr<FinalPlan> realPlan)
{

    //read the config and not add later those predicates, which were added in the relax planning graph
    std::string line;
    std::ifstream file_in;
    size_t index1=task->getProblem().find("/");
    size_t index;
    while (index1 != std::string::npos)
    {
        index = index1;
        index1 = task->getProblem().find("/",index1+1);
    }

    std::string filename = task->getProblem().substr(0,int(index+1)) + "config";

    std::vector<std::string> not_be_added;

    file_in.open ((filename).c_str());

    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {

            index = line.find("(");
            if(index<std::numeric_limits<std::size_t>::max())
            {
                line = line.substr(index,line.size()-index+1);
            }
            not_be_added.push_back(line);
        }
    }
    file_in.close();

    std::vector<std::string> raw_init_state;
    bool init_reach = false;
    //reading the init state as the problem init action doesnt contain negative actions
    file_in.open ((task->getProblem()).c_str());

    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {

            index = line.find("(:init");
            if(index<std::numeric_limits<std::size_t>::max())
            {
                init_reach = true;
                continue;

            }
            index = line.find("(:goal");
            if(index<std::numeric_limits<std::size_t>::max())
            {
                init_reach = false;

            }
            if(init_reach)
            {
                index = line.find("(");
                if(index<std::numeric_limits<std::size_t>::max())
                {
                    size_t index2 = line.find(")");
                    while(true)
                    {
                        if(line.substr(index2+1,1)==")")
                        {
                            index2++;
                        }
                        else
                            break;
                    }
                    std::string literal = line.substr(index,index2-index+1);
                    if((literal.substr(0,1)!="(")||(literal.substr(literal.size()-1,1)!=")"))
                    {
                        std::cout << "wrong literals in problem\n";
                        throw;
                    }
                    index=0;
                    index = literal.find("(",index);
                    int num_brackets=0;
                    while(index!=std::string::npos)
                    {
                        num_brackets++;
                        index = literal.find("(",index+1);
                    }
                    index=0;
                    index = literal.find(")",index);
                    while(index!=std::string::npos)
                    {
                        num_brackets--;
                        index = literal.find(")",index+1);
                    }
                    if(num_brackets!=0)
                    {
                        std::cout << "wrong number of brackets\n";
                        throw;
                    }

                    raw_init_state.push_back(literal);

                }
            }


        }
    }
    file_in.close();


    //TODO get the actual name of domain
    std::string domain = task->getDomain();
    std::map<std::string,std::vector<std::string>>::iterator it;
    it = current_state.find(domain);
    if (it == current_state.end())
    {
        std::vector<std::string> init;
        current_state[domain] = init;
    }

    for(unsigned int i = 0; i < raw_init_state.size();i++)
    {
      //check if the action is not in the lines not be added
      //action is not found, we can add it
      if(std::find(not_be_added.begin(), not_be_added.end(),raw_init_state.at(i))==not_be_added.end())
      {
        //check if it is not already in the current_state
        if(std::find(current_state[domain].begin(),current_state[domain].end(),raw_init_state.at(i)) == current_state[domain].end())
        {
          current_state[domain].push_back(raw_init_state.at(i));
          if(raw_init_state.at(i).find("not") == std::string::npos) //it is not negative
             realPlan->addAchiever(raw_init_state.at(i), "at start", 0);
          else //TODO but this contains not, shouldnt I save just the action? -> YES, I need to fix this here
             realPlan->addDeleter(raw_init_state.at(i), "at start", 0);

        }
      }

    }

}




/* Cleanup function. */
static void cleanup() {
  Problem::clear();
  Domain::clear();
 }


void timeSlack(std::shared_ptr<FinalPlan> realPlan)
{
  //map for faster links search
  //from_id, to_id
  std::map<size_t, std::vector<size_t> > links_ids;
  std::map<size_t, std::vector<size_t> >::iterator it2;
  for(unsigned int i =0; i< realPlan->getLinks()->size();i++)
  {
      if(realPlan->getLinks()->at(i)->getToId() != Plan::GOAL_ID)
      {
         it2 = links_ids.find(realPlan->getLinks()->at(i)->getFromId());
         if(it2 == links_ids.end())
         {
             std::vector<size_t> vec;
             vec.push_back(realPlan->getLinks()->at(i)->getToId());
             links_ids[realPlan->getLinks()->at(i)->getFromId()] = vec;
         }
         else
         {
             links_ids[realPlan->getLinks()->at(i)->getFromId()].push_back(realPlan->getLinks()->at(i)->getToId());
         }
      }
  }

  //std::shared_ptr<std::vector<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > >
  for(int i=realPlan->getSteps()->size()-1;i>=0;i--)
  {
      std::map<size_t, std::vector<size_t> >::iterator it = links_ids.find(realPlan->getSteps()->at(i).second.second);
      double slack = realPlan->getSteps()->at(i).first->getTask()->getDeadline() -realPlan->getSteps()->at(i).first->getEndTime();
      if(it == links_ids.end()) //we havent found anything
      {
          //

          realPlan->getSteps()->at(i).first->setTimeSlack(slack);
      }
      else
      {
          for(unsigned j=0;j<it->second.size();j++)
          {
             double newSlack = realPlan->getStep(it->second.at(j))->getTimeSlack();
              if( newSlack == -1)
              {
                  std::cerr << "computation of time slack error\n";
                  throw;
              }
            if(slack > newSlack)
            {
              slack = newSlack;
            }
          }
          realPlan->getSteps()->at(i).first->setTimeSlack(slack);
      }



  }

}

//TODO how to call it with more problems
int main(int argc, char* argv[])
{
    atexit(cleanup);

    /*init CPU measure*/
    init();

    std::chrono::high_resolution_clock::time_point start, end;

    start = std::chrono::monotonic_clock::now();


    //read the config of the problem to solve
    char c;
    std::ifstream file_in;

    int num_items= 0;
    int num_lines = 0;
    int num_args = 0;
    std::string release = "";
    std::string deadline = "";
    std::string domain = "";
    std::string problem = "";
    std::string arguments = "";
    int num_tasks = 0; //the first task should have id 1
    char ** args;

    char * word;

    if(argc < 2)
    {
        std::cout << "you must specify config file of a problem to solve";
        return -1;
    }
    else if (argc ==3)
    {

        file_in.open(argv[1]);

        std::string xal(argv[1]);
        //std::cout << xal << "\n";
        if (file_in.is_open())
        {
            while ( file_in.get(c))
            {
                if(num_lines ==0) //store arguments
                {
                    if (c == ';')
                    {
                        num_items++;
                        if (num_items == 1)
                        {
                            num_args = std::stod(arguments);

                            args = new char*[num_args+2+3]; //+2 because of adding domain and problem file later +1 for .vhpop +1 for null pointer +1 output
                            arguments = "";
                        }
                        else
                        {
                            word = new char[arguments.length()];
                            std::strcpy(word,arguments.c_str());
                            args[num_items-2+1] = word;
                            arguments = "";
                        }
                    }
                    else if (c=='\n')
                    {
                        num_lines++;
                        arguments ="";
                        num_items = 0;
                    }
                    else
                        arguments += c;
                }
                else
                {
                    if(c == ';')
                        num_items++;
                    else if (num_items ==0) //release_date
                        release += c;
                    else if (num_items == 1)
                        deadline += c;
                    else if (num_items == 2)
                        domain += c;
                    else if (num_items == 3)
                        problem += c;
                    else if (c == '\n')
                    {
                        domain += "\0";
                        problem += "\0";
                        std::shared_ptr<Task> t(new Task(num_tasks,std::stod(release),std::stod(deadline),domain,problem));
                        tasks->push_back(t);
                        release = "";
                        deadline = "";
                        domain = "";
                        problem = "";
                        num_items = 0;
                        num_tasks++;
                    }
                }

             }
        }
        file_in.close();
    }
    else if(argc>=4) //a single task
    {
       std::cerr << "single task unsupported\n";
       throw;
    }



    std::shared_ptr<FinalPlan> realPlan(new FinalPlan());
    int num_steps=0;

    for (int i=0;i<tasks->size();i++)
    {


        //reading problem file to get the correct name of the problem
        std::ifstream file_prob;
        std::string line_p;

        file_prob.open(tasks->at(i)->getProblem().c_str());
        if (file_prob.is_open())
        {
            getline (file_prob,line_p);
            while(line_p.find("(define (problem")==-1)
              getline (file_prob,line_p);

            std::string nameprob = line_p.substr(17,line_p.length());
            nameprob = nameprob.substr(0,nameprob.length()-1);
            tasks->at(i)->setNameProblem(nameprob);
        }
        file_prob.close();

        //------------


        std::string vhpop = std::string(argv[2]);
        word = new char[vhpop.length()+1];//+1 to copy also null char in the end
        std::strcpy(word,vhpop.c_str());
        args[0] = word;
        //args[0] = "./../../phd/Phd/temp_evaluation/vhpop-run/vhpop/vhpop";

        std::string d= tasks->at(i)->getDomain();
        word = new char[d.length()+1];//+1 to copy also null char in the end
        std::strcpy(word,d.c_str());
        args[num_args+1] = word;

        std::string p = tasks->at(i)->getProblem();
        word = new char[p.length()+1]; //+1 to copy also null char in the end
        std::strcpy(word,p.c_str());
        args[num_args+2] = word;

        std::string o = tasks->at(i)->getProblem()+"out";
        word = new char[o.length()+1]; //+1 to copy also null char in the end
        std::strcpy(word,o.c_str());
        args[num_args+3] = word;


        args[num_args+4] = NULL;

        tasks->at(i)->callPlanner(num_args+2+1+1, args);

        if(tasks->at(i)->getPlan()->getVirMemory() > g_vir_memory )
           g_vir_memory = tasks->at(i)->getPlan()->getVirMemory();
        if(tasks->at(i)->getPlan()->getPhyMemory() > g_phy_memory)
           g_phy_memory = tasks->at(i)->getPlan()->getPhyMemory();

        std::shared_ptr<MyPlan>  pt= tasks->at(i)->getPlan();
        if(pt == NULL)
        {
            std::cerr << "main::ERROR: no input plan found\n";
            throw;
        }
        else
        {
            //std::cout << "how many steps:" << pt->getSteps().size();
        }


        if(DEBUG2)
        {
          std::shared_ptr<const Orderings> to = pt->getOrderings();
          std::ostream &osg = std::cout;
          to->print(osg);
          std::cout << "\n";


          pt->print();
          std::cout << "\n";
        }

        num_steps+= pt->num_steps();
        readConfig(tasks->at(i),realPlan);


    }

    //TODO read move_action from the config
    move_action = "move";
    move_preposition = "robot_at";
    move_waypoint_pos = 2;

    double init_time = 0;
    g_argc = num_args+4;
    g_argv = args;

    realPlan->initStepsOrderings(num_steps);
    //realPlan->printStepsOrderings();
   // std::cout << "-------------------------\n";

    int num_steps_previous=0;
    for(unsigned int i=0; i<tasks->size();i++)
    {
        updateTaskLinks(tasks->at(i));
        if(i>0)
        {
            num_steps_previous+=tasks->at(i-1)->getPlan()->num_steps();
        }
        for(unsigned int j=0; j< tasks->at(i)->getPlan()->getLinks().size();j++)
        {
            std::vector<std::pair<std::shared_ptr<MyLink>, int > >();

            std::shared_ptr<MyLink> link = tasks->at(i)->getPlan()->getLinks().at(j).first;


            size_t from_id = link->getFromId();
            size_t to_id =link->getToId();
            //it is a link between actions
            if((from_id!=0)&&(to_id!=Plan::GOAL_ID))
            {
               int id1 = (from_id*2)+(2*num_steps_previous);
               if(link->getEffectTime() == "at start")
               {
                   id1--;
               }
               id1--; //the end is -1, start -2

               int id2 =  (to_id*2)+(2*num_steps_previous);
               if((link->getConditionTime() =="at start")
                       ||(link->getConditionTime() =="over all"))
               {
                   id2--;
               }
               id2--; //the end is -1, start -2
               realPlan->setStepsOrderings(id1,id2,-10,false);
               int id = num_steps_previous + to_id -1;
               realPlan->setCanComeFirst(id,false);
            }
       }
    }

    if(DEBUG)
    {
      //realPlan->printStepsOrderings();
      std::ostream &os = std::cout;

      int taskId_l=0;
      int stepId_l=0;

      for (size_t r = 0; r < realPlan->getNumMainSteps(); r++) {

         os << taskId_l <<"_"<<stepId_l << ": ";
         if(r%2 == 1)
         {
           os << "end: ";
           stepId_l++;
         }
         else
         {
           os<< "sta: ";
         }
         if (stepId_l>= tasks->at(taskId_l)->getPlan()->num_steps())
         {
             taskId_l++;
             stepId_l=0;
         }
        for (size_t c = 0; c < realPlan->getNumMainSteps(); c++) {
          os.width(7);
          int d;
          d = realPlan->getStepsOrderings(r,c);

          if (d < INT_MAX) {
        os << d;
          } else {
        os << "inf";
          }
        }
        os << std::endl;
      }
      std::cout << "-------------------------\n";
      for(int k=0; k < round(realPlan->getNumMainSteps()/2); k++)
      {
          std::cout << realPlan->getCanComeFirst(k);
          std::cout << " ";
      }
      std::cout << "\n";
    }



    //std::cout << "input tasks obtained\n";

    bool success = merging(init_time, realPlan);

    end = std::chrono::monotonic_clock::now();


    if(success)
    {
        std::cout << ";FULL PLAN\n";
        realPlan->print();
        std::cout << ";------------------\n";

        /*for(unsigned int i = 0; i< subplans.size(); i++)
        {
            std::cout << ";SUBPLAN " << i << "\n";
            //TODO thing about better place to call complete
            subplans.at(i)->complete();
            subplans.at(i)->print();
            std::cout << ";------------------\n";
        }*/

     }
    else
    {
        std::cout << "; no plan can be found\n";
        std::cout << ";------------------\n";
    }


    //compute a time slack
    //timeSlack(realPlan);




    //here we are summing the memories, as getVirtualMemoty wil lbe always presented
    std::cerr << ";virtual memory consumed " << (g_vir_memory + getVirtualMemory())
              << std::endl;

    std::cerr << ";physical memory consumed " << (g_phy_memory + getPhysicalMemory())
              << std::endl;

    std::cerr << ";CPU consumed " << getCurrentValueCPU()
              << std::endl;

    std::chrono::duration<double> dt = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << ";detail time: ";
    std::cout << dt.count();
    std::cout << "\n";
    std::cout << ";number of backtracks" << number_of_backtracks << "\n";


    //here conditional planning
    //Conditional::mainConditions(realPlan, execution_state, g_argc, g_argv);




    delete [] args; //TODO not sure about this

    return 0;
}
