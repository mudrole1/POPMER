//------------------------------
//guarding to avoid multiple including
#ifndef __TASK_H_INCLUDED__
#define __TASK_H_INCLUDED__


#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <stdlib.h>

#include "vhpop/plans.h"
#include "vhpop/vhpop_callable.h"
#include "myplan.h"

class Task
{
  int id; //a number refering to the task
  double release_date;  //the time when the task can start
  double deadline;    //the time when the task needs to be finished
  double duration; //task duration, ignoring travel


  std::string domain_file;
  std::string problem_file; //filename
  std::string name_problem; //to save name of the problem from the file

  std::shared_ptr<MyPlan> plan;
  bool plan_available;
  bool used;
  std::vector<std::pair<std::pair<size_t,float>,int> > start_vector; //to save ordering steps
  std::vector<std::pair<std::shared_ptr<const Link> ,bool> > links_vector; //to save links

  Vhpop_callable v_callable;

  int parent_task; //if it just temporary task, it will have a parent_task -> the one which action has requested this
  size_t parent_step; //the action id which requested this temporary task

  public:
  Task (int, double, double, std::string, std::string); //default priority = 1
  Task (int, double, double, std::string, std::string, std::string);
  Task (const Task&);


  int getID();
  double getReleaseDate();
  double getDeadline();
  double getDuration();
  std::string getDomain();
  std::string getProblem();
  void setNameProblem(std::string);
  std::string getNameProblem();
  void setUsed(bool);
  bool getUsed();

  int getParentTask();
  void setParentTask(int id);
  int getParentStepId();
  void setParentStepId(int id);

  bool getPlanAvailable()
  {
      return plan_available;
  }



  std::vector<Step> steps;
  std::vector<const Action *> action;

  std::shared_ptr<MyPlan> getPlan();
  int callPlanner(int, char* []);
  Task& operator=(Task);

  friend std::ostream& operator<<(std::ostream&, const Task&);
  friend bool operator== (Task &t1, Task &t2);

};

#endif


