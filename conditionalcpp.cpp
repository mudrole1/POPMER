#include "conditionalcpp.h"
#include "tree_four.h"
#include "finalplan.h"

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

std::map<std::string, std::vector<std::string> > exec_state;

int ge_argc;
char ** ge_argv;
int g_temp_prob_e = 0;
std::shared_ptr< std::vector< std::shared_ptr<Task> > > temp_tasks_e(new std::vector<std::shared_ptr<Task>>());

struct StepSorter {
  StepSorter(std::map<size_t, float>& dist)
    : dist(dist) {}

  bool operator()(const Step* s1, const Step* s2) const {
    return dist[s1->id()] < dist[s2->id()];
  }

  std::map<size_t, float>& dist;
};

static std::string writeInitialState(std::string domain)
{

    std::string output="";
    for (int i=0;i<exec_state[domain].size();i++)
    {
      output += "    ";
      output += exec_state[domain].at(i);
      output += "\n";
    }
    output += ")\n";
    return output;
}

static std::vector<std::string> createTempProblem(Task * origin, std::vector<int> invalidFacts)
{
    std::stringstream ss;
    std::ostream &os(ss);

    std::string problemName = origin->getProblem();

    size_t index = problemName.find("problem.pddl");

    std::string line;
    std::ofstream file_out;
    std::ifstream file_in;
    std::string filename = problemName.substr(0,int(index)) + "temp_problem.pddl\0";
    std::string probname;

    for(int i=0;i<invalidFacts.size();i++)
    {
        std::string domain = origin->getDomain();
        std::string action = exec_state[domain].at(invalidFacts.at(i));
        int index;
        if(action.find("not") != std::string::npos) //negative
        {

            index = action.find("(",2); //skip the first bracket
            action = action.substr(index,action.length()-index-1);
        }
        else //possitive
        {
           action = "(not "+ action + ")";
        }
        exec_state[domain].at(invalidFacts.at(i)) = action;
    }

    file_out.open ((filename).c_str());

    bool init_r = false;
    //copying settings of the problem file

    //TODO I am copying here goal, but I will have problems if I have more files
    //and goals are separated
    file_in.open(problemName.c_str());
    int h = 0;
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
           if (h==0) //first line
           {
               h++;
               line = line.substr(0,line.length()-1)+"_temp"+std::to_string(g_temp_prob_e) + ")";
               g_temp_prob_e++;
               probname = line;
           }
           if(line.find("(:init") == std::numeric_limits<std::size_t>::max()) //copy
           {
               if(!init_r)
                  file_out << line << "\n";
               if (line.find("(:goal") == 0) //found init
               {
                   init_r = false;
               }
           }
           else
           {
             if (line.find("(:init") == 0) //found init
             {
                 init_r = true;
                 file_out << "(:init\n";
                 file_out << writeInitialState(origin->getDomain());
                 file_out << "(:goal (and\n";
             }

           }
        }
    }
    file_in.close();


    file_out.close();

    probname = probname.substr(17,probname.length()-1);
    probname = probname.substr(0,probname.length()-1)+"\0";

    std::vector<std::string> names;
    names.push_back(filename);
    names.push_back(probname);

    return names;
}

static void planTempTask(Task * origin, std::vector<int> invalidFacts)
{
    std::vector<std::string> names = createTempProblem(origin, invalidFacts);
    std::string problemName = names.at(0);


    std::shared_ptr<Task> temp(new Task(-1,origin->getReleaseDate(),origin->getDeadline(),origin->getDomain(),problemName,names.at(1)));
    char * word;

    word = new char[problemName.length()+1]; //+1 to copy also null char in the end
    std::strcpy(word,problemName.c_str());
    ge_argv[ge_argc-1] = word;


    word = new char[origin->getDomain().length()+1]; //+1 to copy also null char in the end
    std::strcpy(word,origin->getDomain().c_str());
    ge_argv[ge_argc-2] = word;

    temp->callPlanner(ge_argc,ge_argv);



    temp_tasks_e->push_back(temp);
}

static char eventHandling(Tree_four * t, std::vector<int> invalidFacts)
{



   planTempTask(t->getNode()->data->first->getTask(), invalidFacts);

   if(temp_tasks_e->at(g_temp_prob_e-1)->getPlan() != NULL)
   {
       std::cout << "success replan\n";
       Tree_four * r = new Tree_four();
       int id=t->getNode()->data->first->getStep()->id()+1;
       std::vector<const Step*> ordered_steps;

       for (const Chain<Step>* sc = temp_tasks_e->at(g_temp_prob_e-1)->getPlan()->steps(); sc != NULL; sc = sc->tail) {
         const Step& step = sc->head;
         if (step.id() == 0) {
           //init = &step;
         } else if (step.id() == Plan::GOAL_ID) {
           //goal = &step;
         } else {
           ordered_steps.push_back(&step);
         }
       }

       std::map<size_t, float> start_times;
       std::map<size_t, float> end_times;
       float makespan = temp_tasks_e->at(g_temp_prob_e-1)->getPlan()->orderings().schedule(start_times, end_times);

       sort(ordered_steps.begin(), ordered_steps.end(), StepSorter(start_times));

       for(int i=0;i<ordered_steps.size();i++)
       {
           //TODO these steps are not ordered!!!
         std::unique_ptr<Step> step_p(new Step(*ordered_steps.at(i)));


         std::shared_ptr<StepTimed> st( new StepTimed(std::move(step_p),temp_tasks_e->at(g_temp_prob_e-1).get() ));
         std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > pa(new std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >());
         pa->first = st;
         pa->second.first = "start";
         pa->second.second = id;

         std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > pb(new std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> >());
         pb->first = st;
         pb->second.first = "end";
         pb->second.second = id;
         id++;

         r->insert(pa);
         r->insert(pb);


       }
       t->getNode()->repeat = r->root; //TODO I am assigning the pointer, I need to use
       //the smart pointers later;

   }
   else
   {
       std::cout << "replan failed\n";
       throw;
   }
   return 'r';
}

static int areConditionsSatisfiedNow(std::string action, std::string when, std::string domain)
{
    std::vector<size_t> indexes;
    std::vector<std::string> sub_actions;
    std::string sub_action;
    int skip_index;
    int index;

    bool andF = false;
    int index2 = -1;


    //if the predicate has "not", it is saved, but I need to test it
    if(action.find("and") != std::string::npos) //and statement
    {
        andF = true;
        skip_index = 2; //skip the first bracket
        index = action.find("(",skip_index);
        //find all predicates
        while( index != std::string::npos) //something found
        {
            indexes.push_back(index);
            skip_index = action.find(")",skip_index+1);
            skip_index++;  //there always two brackets, I need the second one
            sub_action = action.substr(index,skip_index-index+1); //those predicates can have "not"
            index = action.find("(",skip_index);
            //subaction contains "at end" or over all
            if (when == "") //it is at start
            {
              if((sub_action.find("at end")==std::string::npos)&&(sub_action.find("over all")==std::string::npos))
              //it wasnt find none of those
              {
                  index2 = -1;
                  sub_actions.push_back(sub_action.substr(index2+when.size()+1,sub_action.size()-1-index2-when.size()-1)); //-1 dont take last bracket
              }
              else
              {
                  //we found at end or overall, skip it
                  continue;
              }

            }
            else
                index2 = sub_action.find(when);
            if(index2 != std::string::npos) //something found
            {
                sub_actions.push_back(sub_action.substr(index2+when.size()+1,sub_action.size()-1-index2-when.size()-1)); //-1 dont take last bracket
            }
        }
    }
    else
    {
      andF = false;
      index = action.find("("); //TODO deleted here num 1, does it break things?
      index2 = -1;
      if (index != std::string::npos)
      {
          if (when == "") //it is at start
          {
            if((action.find("at end")==std::string::npos)&&(action.find("over all")==std::string::npos))
            //it wasnt find none of those
            {
                index2 = -1;
                sub_actions.push_back(action.substr(index2+when.size()+1,action.size()-1-index2-when.size())); // changed -1 took away - dont take last bracket
            }
            else
            {
                //we found at end, over all when we are interested just in "at start" - skip it
                index2 = -1;
            }

          }
          else
              index2 = action.find(when);
          if(index2 != std::string::npos) //something found
          {
              sub_actions.push_back(action.substr(index2+when.size()+1,action.size()-1-index2-when.size()-1)); //-1 dont take last bracket
          }

      }
    }

    int predicateSatisfied = 1;
    for(unsigned int j=0;j<sub_actions.size();j++)
    {
        int index_c = std::distance(exec_state[domain].begin(),std::find(exec_state[domain].begin(),exec_state[domain].end(),sub_actions.at(j)));

        if (index_c == exec_state[domain].size()) //nothing found
        {
          predicateSatisfied*= 0; //this goal is not yet satisfied
          break; //we do not have to check others
        }
    }
    return predicateSatisfied;
}

static bool executeOneAction(Tree_four * t)
{

    TreeNode * n = t->getNode();

    if(n->data == NULL) //there is none chosen action, but we still have things to merge
    {
        return false;
    }
    else
    {
        std::shared_ptr<StepTimed> active_action_p = n->data->first;

        std::string domain = active_action_p->getTask()->getDomain();
        std::stringstream ss1;
        std::ostream &os2(ss1);

        /*before starting to merge, check if the conditions of the actions are really satisfied*/
        active_action_p->getStep()->action().condition().print(os2,active_action_p->getStep()->id(), *active_action_p->getTask()->getPlan()->bindings_anytime());

        std::string condition = ss1.str();
        if(n->data->second.first=="start") //at start action - pass an empty string, as the stupid timed literal is not using at start
        {
          if (areConditionsSatisfiedNow(condition, "", domain)!=1)
          {
            return false;
          }
          //we need to test also over all conditions, as they must be true over all action
          if (areConditionsSatisfiedNow(condition, "over all", domain)!=1)
          {
            return false;
          }
        }
        else //at end action
        {
          if (areConditionsSatisfiedNow(condition, "at end", domain)!=1)
          {
            return false;
          }
          //we need to test also over all conditions, as they must be true over all action
          if (areConditionsSatisfiedNow(condition, "over all", domain)!=1)
          {
            return false;
          }
        }


    EffectList effL = active_action_p->getStep()->action().effects();
    size_t index;
    //std::string domain = active_action_p->getTask()->getDomain();

    //IMPORTANT - canBeMerged set the endtime to -1 for the end actions
    //the start action would have the deadline of the action
    for (int i =0; i< effL.size();i ++)
    {
        /*use only those effect which rely if we are merge start or end of the action*/
        if((n->data->second.first=="start")&&(effL.at(i)->when() != Effect::AT_START))
        //we want start effects, but this is not one, do nothing
        {
            continue;
        }
        if((n->data->second.first=="end")&&(effL.at(i)->when() != Effect::AT_END))
        //we want end effects, but this is not one, do nothing
        {
            continue;
        }



        //we got correct effect, put it to the current state
        std::stringstream ss;
        std::ostream &os(ss);


        effL.at(i)->literal().print(os,active_action_p->getStep()->id(), *active_action_p->getTask()->getPlan()->bindings_anytime());


        std::string action = ss.str();
        std::string condition;


        bool neg = false;
        int index2;

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
        std::vector<std::string> vec = exec_state[active_action_p->getTask()->getDomain()];
        index2 = std::distance(vec.begin(),std::find(vec.begin(),vec.end(),action));


        if(index2 == vec.size()&&(!neg)) //nothign found
        {
            exec_state[domain].push_back(action);

        }
        else if ((index2 < vec.size())&&(!neg)) //it is there, but it is not negated
        {// do nothing,

        }
        else if((index2 < vec.size())&&(neg)) //we want to delete it
        {
            exec_state[domain].erase(exec_state[domain].begin()+index2);
            // Lenka fix - we can't only deleted, we need to add the negation
            //otherwise it leads to undefined situations
            //TODO check if this logic is not broken at more places of the code
            //TODO should be not action be as achiever of not???
            exec_state[domain].push_back("(not "+action+")");

        }
        else if ((index2 == vec.size())&&(neg)) //we need to add the neg
        {
            exec_state[domain].push_back("(not "+action+")");

        }

        int index2_not = std::distance(vec.begin(),std::find(vec.begin(),vec.end(),not_action)); //try to find negation of the action

        if((index2_not < vec.size())&&(!neg)) //we found a negation in the current state, the action needs to be deleted
        {
            exec_state[domain].erase(exec_state[domain].begin()+index2_not);

        }

        //TODO test: what if the current state has not state and it should be deleted by added thing


      }

    }
  }

    t->PrintOne(n);
    std::cout<< "\n";
    return true;
}

static void printExecState(std::string domain)
{
   std::cout<< "---------------------\n";
   std::cout << "The current execution state is:\n";
   for(int i=0;i<exec_state[domain].size();i++)
   {
      std::cout << i << ": " << exec_state[domain].at(i) << "\n";
   }
   std::cout<< "---------------------\n";
}

/*static void printGoals(Task * task)
{
    std::string domain = task->getDomain();
        std::stringstream ss;
        std::ostream &os(ss);
        const Problem* pr = Problem::find(task->getNameProblem());
        const Formula * goal = &pr->goal();
        goal->print(os,Plan::GOAL_ID,*task->getPlan()->bindings_anytime());

        std::string action = ss.str();
}*/

static int execute(Tree_four * t)
{
    char in;
    while(t->getNode()!=nullptr)
    {
        TreeNode * n = NULL;
        if (!executeOneAction(t))
        {
            std::cout << "invalid execution\n";
            throw;
        }

        //print current exec_state
        printExecState(t->getNode()->data->first->getTask()->getDomain());

        std::cout << "Are all the fact still true?\n";
        std::cout << "Press c for yes, otherwise key in the number of a fact which is not true anymore.\n";
        std::cout << "Press d when you are finished.\n";

        std::vector<int> invalidFacts;
        //std::string str = "";
        //getline(str,std::cin);
        //while(str=="")
        //{
        //    getline(str,std::cin);
        //}
        std::cin >> in;
        while(isdigit(in))
        {
            int v = in - '0';
            std::cout << "chosen";
            std::cout << v;
            std::cout << "\n";
            invalidFacts.push_back(int(v));
            std::cin >>in;
        }

        while((!invalidFacts.empty())&&(in != 'd'))
        {
            std::cout << "key in a number or press d to terminate\n";
            std::cin >> in;
        }

        while((in != 'd')&&(in != 'c'))
        {
            std::cout << "use c or d to terminate\n";
        }
        if(in == 'd')
        {
           in = eventHandling(t,invalidFacts);
           n = t->getSubTree(in);
        }
        else if ((in=='c')&&(invalidFacts.empty()))
        {
           n = t->getSubTree(in);
        }


        if(n==NULL)
        {
            std::cout<<"no plan available, total fail\n";
            break;
        }
        t = new Tree_four();
        t->insert(n);

    }

}

bool Conditional::mainConditions(std::shared_ptr<FinalPlan> realPlan, std::map<std::string, std::vector<std::string> > execution_state, int argc,
char ** argv)
{
    Tree_four * theTree = new Tree_four();

    exec_state = execution_state;
    ge_argc = argc;
    ge_argv = argv;
    //TODO I should add "zero action" to the root
    for(int i=0; i<realPlan->amountOfSteps() ;i++)
    {
        theTree->insert(realPlan->getSeqStep(i));
    }
    //realPlan->getFullStep(1);
    //std::cout <<"-----------------------------------------------------\n";
    //theTree->PrintTree();
    //std::cout << "\n";
    execute(theTree);
}


