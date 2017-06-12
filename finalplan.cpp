#include "finalplan.h"

FinalPlan::FinalPlan()
{
  links_.reset(new std::vector<std::shared_ptr<MyLink> >());
  orderings_.reset(new TemporalOrderings());
  steps_.reset(new std::vector<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > >());
}

void FinalPlan::addLink(std::shared_ptr<MyLink> link)
{
  links_->push_back(link);
}

void FinalPlan::changeLink(int id, bool threat)
{
    links_->at(id)->setBeingThreaten(threat);
}

void FinalPlan::addStep(std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > stepT)
{
    steps_->push_back(stepT);
}

//the id is in the sense of "new or global id" in contract to ids saved in StepTimed
std::shared_ptr<StepTimed> FinalPlan::getStep(size_t id)
{
    for(unsigned int i=0;i<steps_->size();i++)
    {
        if(steps_->at(i).second.second == id)
        {
            return steps_->at(i).first;
        }
    }

    return nullptr;
}

/* 15/09 checked
 * this method add achiver or change the id of achieving action if the atom has been already noted
 * realPlan = the final plan
 * condition = the atom to be noted down
 * when = time of the effect which it has achieved it
 * id = id of the action achieving it
 */
void FinalPlan::addAchiever(std::string condition, std::string when, int id)
{


    //if condition is not yet in achievers, add it
    std::map<std::string, std::vector<std::pair<std::string, int> > >::iterator it;
    it = achievers.find(condition);
    if(it == achievers.end()) //nothing found
    {
        std::vector< std::pair<std::string, int> > ach;
        ach.push_back(std::pair<std::string, int>(when,id));
        achievers[condition] = ach;
    }
    else //replace the id with the newer one
    {
        achievers[condition].push_back(std::pair<std::string, int>(when,id));
    }
}

/* 15/09 checked
 * this method add deleter or change the id of achieving action if the atom has been already noted
 * realPlan = the final plan
 * condition = the atom to be noted down
 * when = time of the effect which it has achieved it
 * id = id of the action achieving it
 */
void FinalPlan::addDeleter(std::string condition, std::string when, int id)
{
     //TODO to test this
    //if condition include not, get it away, deleters are pure atoms
    if(condition.find("(not")!=std::string::npos) // we found it
    {
        condition = condition.substr(5,condition.length()-5-1);
    }

    //if condition is not yet in deleters, add it
    std::map<std::string,std::vector<std::pair<std::string, int> > >::iterator it;
    it = deleters.find(condition);
    if(it == deleters.end()) //nothing found
    {
        std::vector<std::pair<std::string, int> > del;
        del.push_back(std::pair<std::string, int>(when,id));
        deleters[condition] = del;
    }
    else //replace the id with the newer one
    {
        deleters[condition].push_back(std::pair<std::string, int>(when,id));
    }
}

std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > FinalPlan::removeLastStep(bool test)
{
    if(steps_->size()==0)
    {
        std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > pr;
        pr.second.first="none";
        return pr;
    }
    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > s = steps_->back();
    if(test)
    {
        return s;
    }

    steps_->pop_back();

    if(s.second.first=="start")
    {
        std::map< std::shared_ptr<StepTimed>, size_t>::iterator it = unfinished_actions.find(s.first);
        if (it != unfinished_actions.end()) //found something
        {
            unfinished_actions.erase(it);
        }

    }
    return s;
}

std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > FinalPlan::getLastStep()
{
    std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > s = steps_->back();

    return s;
}

size_t FinalPlan::getUnfishedId(std::shared_ptr<StepTimed> step)
{
    std::map<std::shared_ptr<StepTimed>, size_t>::iterator it = unfinished_actions.find(step);
    if (it != unfinished_actions.end())
        return it->second;
    else
        return std::numeric_limits<std::size_t>::max();
}

std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > FinalPlan::getStepwithGIndex(int index)
{
    if ((index < steps_->size())&&(index>=0))
        return steps_->at(index);
    else
    {
        std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > pr;
        pr.second.first="none";
        return pr;
    }

}

std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > FinalPlan::getFullStep(size_t id)
{
    for(unsigned int i=0;i<steps_->size();i++)
    {
        if(steps_->at(i).second.second == id)
        {
            std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > a;
            a.reset(&steps_->at(i));
            return a;
        }
    }

    return nullptr;
}


std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > FinalPlan::getSeqStep(size_t index)
{
  if (index < steps_->size())
  {
      std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > a;
      a.reset(&steps_->at(index));
      return a;
  }
  else
      return nullptr;
}

long double FinalPlan::duration()
{
    std::map<size_t, long double> start_times;
    std::map<size_t, long double> end_times;
    long double makespan = orderings_->getMakespan(start_times, end_times) - Orderings::threshold;
    return makespan;
}


void FinalPlan::print()
{
    //std::map<size_t, long double> start_times;
    //std::map<size_t, long double> end_times;
    //long double makespan = orderings_->getMakespan(start_times, end_times);
    double makespan = steps_->back().first->getEndTime();
    std::cout << "; Plan found\n";
    std::cout << ";makespan: " << makespan << "\n";
    double final_time;

    std::ostream &osx1 = std::cout;

    for(int i=0; i<steps_->size();i++)
    {
        if(steps_->at(i).second.first == "start")
        {

          std::cout << std::fixed;
          std::cout << std::setprecision(2);

          std::cout << steps_->at(i).first->getStartTime() << ": ";
          steps_->at(i).first->getStep()->printName(osx1);


          std::cout << " [" << steps_->at(i).first->getStep()->getDuration() << "]";
          std::cout << "\n";
        }
    }
}

void FinalPlan::updateAchiDel( std::pair<std::string, size_t> step)
{
    //step.first = what type of action "start","end"
    //step.second = id
    std::map<std::string, std::vector<std::pair<std::string, int> > >::iterator it;
    it = achievers.begin();
    int count =0;
    while(it!=achievers.end())
    {
        if(it->second.back().second == step.second)
        {
            if((it->second.back().first=="at start")&&(step.first=="start"))
            {
              it->second.pop_back(); //delete the last item

            }
            else if((it->second.back().first=="at end")&&(step.first=="end"))
            {
              it->second.pop_back(); //delete the last item     
            }
            if(it->second.size()==0)
            {
                achievers.erase(it);
                count--;
            }
        }
        count++;
        it = std::next(achievers.begin(),count);
    }

    count=0;
    it = deleters.begin();
    while(it!=deleters.end())
    {
        if(it->second.back().second == step.second)
        {
            if((it->second.back().first=="at start")&&(step.first=="start"))
            {
              it->second.pop_back(); //delete the last item
            }
            else if((it->second.back().first=="at end")&&(step.first=="end"))
            {
              it->second.pop_back(); //delete the last item
            }
            if(it->second.size()==0)
            {
                deleters.erase(it);
                count--;
            }
        }
        count++;
        it = std::next(deleters.begin(),count);
    }

}

void FinalPlan::initStepsOrderings(int num_steps)
{
    canComeFirst = std::vector<bool>(num_steps,true);

    num_main_steps = 2*num_steps;
    //first vector is rows, then columns
    steps_ordering = std::vector<std::vector<int> >((2*num_steps), std::vector<int>((2*num_steps),INT_MAX));
    for(int r=0; r < num_main_steps; r++)
    {
        for(int c=0; c< num_main_steps; c++)
        {
            if(r==c)
                steps_ordering.at(r).at(c) = 0;
            if((r%2 == 1)&&(c%2 == 0)&&(c+1==r))
                steps_ordering.at(r).at(c) = 10;
            if((r%2 == 0)&&(c%2 == 1)&&(r+1==c))
                steps_ordering.at(r).at(c) = -10;
        }

    }
}

void  FinalPlan::setStepsOrderings(int from_id, int to_id, int type, bool overwrite)
{
    //from_ids rows, to_ids column
    if((from_id>=num_main_steps)||(to_id>=num_main_steps))
    {
           std::cerr << "FinalPlan::ERROR: invalid index\n";
           throw;
    }
    else
    {

        if((steps_ordering.at(from_id).at(to_id)!=INT_MAX)&&(steps_ordering.at(from_id).at(to_id)<0)&&(type>0)&&(type!=INT_MAX)&&(type!=2))
        {
            //it is already set by different value
            std::cerr << "FinalPlan::ERROR: already set position\n";
            throw;
        }

        if((steps_ordering.at(from_id).at(to_id)!=INT_MAX)&&(steps_ordering.at(from_id).at(to_id)>0)&&(type<0)&&(type!=INT_MAX)&&(type!=2))
        {
            //it is already set by different value
            std::cerr << "FinalPlan::ERROR: already set position\n";
            throw;
        }

        if(abs(steps_ordering.at(from_id).at(to_id))==10) //do nothing, do not change
            return;

        if(from_id == to_id) //dont change
            return;

        if((abs(steps_ordering.at(from_id).at(to_id))==5)&&(!overwrite))
           return; //do not change

        if((abs(steps_ordering.at(from_id).at(to_id))==5)&&(overwrite))
        {
            if(type==INT_MAX)
            {
                  steps_ordering.at(from_id).at(to_id) = type;
                  steps_ordering.at(to_id).at(from_id) = type;
                  return;

            }
            else
            {
              steps_ordering.at(from_id).at(to_id) = type;
              steps_ordering.at(to_id).at(from_id) = -type;
            }
        }

        if(type==INT_MAX)
        {
              steps_ordering.at(from_id).at(to_id) = type;
              steps_ordering.at(to_id).at(from_id) = type;
              return;

        }
        else
        {
          steps_ordering.at(from_id).at(to_id) = type;
          steps_ordering.at(to_id).at(from_id) = -type;
        }
    }

}

int FinalPlan::getStepsOrderings(int ind1, int ind2)
{
    return steps_ordering.at(ind1).at(ind2);
}

void FinalPlan::printStepsOrderings()
{
    std::ostream &os = std::cout;

    for (size_t r = 0; r < num_main_steps; r++) {

      for (size_t c = 0; c < num_main_steps; c++) {
        os.width(7);
        int d;
        d = steps_ordering.at(r).at(c);

        if (d < INT_MAX) {
      os << d;
        } else {
      os << "inf";
        }
      }
      os << std::endl;
    }
}


