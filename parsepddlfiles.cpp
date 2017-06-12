#include "parsepddlfiles.h"

/*
 * Before running make sure domain is:
 * each condition and effect is on its line
 * the action end bracket is on a separate line
 * the parameters do not contain "-"
 */

std::map<std::string, std::shared_ptr<MyActionSchema>> ParsePDDLfiles::parseDomain(std::string domain_file)
{

    std::string line;
    std::ifstream file_in;;

    file_in.open(domain_file.c_str());

    std::string domain_name;
    std::map<std::string, std::shared_ptr<MyActionSchema>> actions;
    std::vector<std::string> parameters;
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  condi;
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effects;
    std::string name;


    bool parametersON = false;
    bool conditionsON = false;
    bool effectsON = false;

    unsigned int num_l = 0;
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
           if(num_l == 0)
           {
               size_t index = line.find("domain");
               if(index !=std::string::npos)
               {
                   //find the next word after
                   index = line.find(" ",index+1);
                   size_t index2 = line.find(")",index+1);
                   if((index !=std::string::npos)&&(index2 !=std::string::npos))
                   {
                       domain_name = line.substr(index+1,index2-index-1);
                   }
               }
               num_l++;
               continue;
           }
           size_t index = line.find("(:durative-action");
           size_t index2;
           if(index !=std::string::npos)
           {
                if(effectsON)//for some reason we missed the last bracket
                {
                    effectsON = false;

                    actions[name]->setEffects(effects);

                    effects.clear();
                }
                std::shared_ptr<MyActionSchema> schema(new MyActionSchema());
                //find the next word after
                index = line.find(" ",index+1);
                if(index !=std::string::npos)
                {
                    //get the name until the end of line
                    name = line.substr(index+1,line.size()-index-1);
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    //TODO this is assummimg well behave domain writer
                    schema->setDomainName(domain_name);
                    schema->setName(name);
                }
                else
                {
                    throw;
                }
                actions[name] = schema;
           }
           index = line.find(":parameters");
           if(index !=std::string::npos)
           {
                parametersON = true;
                continue;

           }


           if(parametersON)
           {

               index = line.find("?");
               index2 = line.find("-");
               while((index !=std::string::npos)&&(index2 !=std::string::npos))
               {
                  std::string par = line.substr(index,index2-index);
                  if(par.find(" ")!=std::string::npos)
                      par = par.substr(0,par.size()-1);
                  parameters.push_back(par);
                  index = line.find("?",index+1);
                  index2 = line.find("-",index2+1);
               }
               index = line.find(")");
               if(index !=std::string::npos)
               {
                   parametersON= false;
                   actions[name]->setParameters(parameters);
                   parameters.clear();
               }
           }
           index = line.find("?duration");
           if(index !=std::string::npos)
           {
                index = line.find(" ", index);
                index2 = line.find(")", index);
                double dur = std::stod(line.substr(index+1,index2));
                actions[name]->setDuration(dur);

           }
           index = line.find(":effect");
           if(index !=std::string::npos)
           {
                conditionsON = false;
                effectsON = true;
                actions[name]->setConditions(condi);
                condi.clear();
                continue;

           }


           if((line == ")")&&(effectsON)) //closing braket
           {
                effectsON = false;
                actions[name]->setEffects(effects);
                effects.clear();
                continue;
           }

           index = line.find(":condition");
           if(index !=std::string::npos)
           {
               conditionsON = true;
               continue;
           }
           if((conditionsON)||(effectsON))
           {
               //TODO not
               //(over all (at ?truck ?loc))
               std::string time="";
               std::string predicate;
               std::vector<int> par_indexes;

               index = line.find("(over all");
               if(index !=std::string::npos)
               {
                   time = "over all";
               }
               else
               {
                   index = line.find("(at start");
                   if(index !=std::string::npos)
                   {
                       time = "at start";
                   }
                   else
                   {
                       index = line.find("(at end");
                       if(index !=std::string::npos)
                       {
                           time = "at end";
                       }
                       else
                       {
                           time="";
                       }
                   }
               }
               if( time != "")//we found osmething
               {
                   bool noparameters = false;
                   index2 =line.find("?");
                   if (index2 == std::string::npos)
                   {
                       noparameters = true;
                   }
                   index2 = line.find("(",index+1);
                   size_t index3 = line.find(" ",index2+1);


                   if(index3 !=std::string::npos)
                   {
                       predicate = line.substr(index2,index3-index2);
                       //(at start (not (at ?obj ?loc)))
                       if(predicate == "(not")
                       {
                           index3 = line.find(" ",index3+1) ;
                           if(index3 !=std::string::npos)
                           {
                               predicate = line.substr(index2,index3-index2);
                           }
                           else
                           {
                               //in not there will be alway 2 brackets
                               index3 = line.find(")",index2+1);
                               index3 = line.find(")",index3+1);
                               //there are no parameters)
                               predicate = line.substr(index2,index3-index2+1);
                               noparameters = true;
                           }
                       }
                       index2 = line.find(" ",index3+1);

                       if(!noparameters)
                       {
                           while(index2 !=std::string::npos)
                           {
                               std::string expression = line.substr(index3+1, index2-index3-1);
                               std::vector<std::string> parameters = actions[name]->getParameters();
                               std::vector<std::string>::iterator it = std::find(parameters.begin(),parameters.end(),expression);
                               if(it!=parameters.end())
                               {
                                   par_indexes.push_back(std::distance(parameters.begin(),it));
                               }
                               else
                               {
                                   throw;
                               }
                               index3 = index2;
                               index2 = line.find(" ",index3+1);
                               index = line.find(")",index3+1);
                               if(index < index2)
                                  break;
                           }
                           index2 = line.find(")",index3+1);
                           if(index2 !=std::string::npos)
                           {
                               std::string expression = line.substr(index3+1, index2-index3-1);
                               std::vector<std::string> parameters = actions[name]->getParameters();
                               std::vector<std::string>::iterator it = std::find(parameters.begin(),parameters.end(),expression);
                               if(it!=parameters.end())
                               {
                                   par_indexes.push_back(std::distance(parameters.begin(),it));
                               }
                               else
                               {
                                   throw;
                               }

                           }
                        }

                   }
                   else
                   {
                      index3 = line.find(")",index);
                      if(index3 !=std::string::npos)
                      {
                          predicate = line.substr(index2,index3-index2-1);
                      }
                   }
                   std::pair<std::string, std::string> first;
                   std::pair< std::pair<std::string, std::string>, std::vector<int> > second;
                   std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > third;

                   first.first = time;
                   first.second = predicate;
                   second.first = first;
                   second.second = par_indexes;
                   third.first = second;
                   if(predicate.find("(not")!=std::string::npos)
                   {
                     third.second = "))";
                   }
                   else
                     third.second = ")";

                   if(conditionsON)
                      condi.push_back(third);
                   else if (effectsON)
                      effects.push_back(third);

               }
           }


        }
    }
    file_in.close();

    return actions;

}

static std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > process_lines(std::vector<std::string> lines, std::map<std::string, std::pair<std::string, int> > * achievers,
              std::vector<std::pair<std::shared_ptr<MyLink>, int > > * links,
              std::map<std::string, std::shared_ptr<MyActionSchema>> domain_actions,
              std::shared_ptr<const TemporalOrderings> * to, unsigned int id)
{

    std::vector<double> start_times;
    std::vector<double> end_times;

    size_t index, index2, index3;

    std::string line;
    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > groundActions;

    //time is higher, process the lines, delete them and save new one!
    //intensionally in reverse order due to satisfying achievers
    for(int g= lines.size()-1; g>=0; g--)
    {
        id++;
        line = lines.at(g);
        index= line.find(":");
        start_times.push_back(std::stod(line.substr(0,index)));
        index = line.find("(",index);
        index2 = line.find(" ", index+1);
        std::string name = line.substr(index+1,index2-index-1);
        //TODO here must come find in the domain!!!
        std::map<std::string, std::shared_ptr<MyActionSchema>>::iterator it = domain_actions.find(name);
        if(it == domain_actions.end())
            throw;

        end_times.push_back(start_times.back()+it->second->getDuration());
        to->reset((*to)->refine2(id, start_times.back(),end_times.back()));

        /*std::ostream &osg = std::cout;
        (*to)->print(osg);
        std::cout << "\n";*/

        std::vector<std::string> expression;
        index3 = line.find(" ",index2+1);
        while(index3 !=std::string::npos)
        {
            expression.push_back( line.substr(index2+1, index3-index2-1));

            index2 = index3;
            index3 = line.find(" ",index2+1);
            index = line.find(")",index2+1);
            if(index < index2)
               break;
        }
        index3 = line.find(")",index2+1);
        if(index2 !=std::string::npos)
        {
            expression.push_back( line.substr(index2+1, index3-index2-1));
        }
        std::pair<std::shared_ptr<MyGroundAction>, int > p(std::shared_ptr<MyGroundAction>(new MyGroundAction(*(it->second.get()), expression, id)), -1);
        groundActions.push_back(p);
        std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > condi = groundActions.back().first->getConditions();
        for(unsigned int i=0; i < condi.size();i++)
        {
            std::string literal;
            literal = condi.at(i).first.first.second;
            for(unsigned int j=0; j< condi.at(i).first.second.size();j++)
            {
                literal += " ";
                literal += groundActions.back().first->getParameters().at(condi.at(i).first.second.at(j));

            }
            literal += condi.at(i).second;
            //time, predicate parameters end brackets

            std::map<std::string, std::pair<std::string, int> >::iterator it = (*achievers).find(literal);
            if(it!=(*achievers).end())
            {
                //create link
                std::pair<std::shared_ptr<MyLink>, int > p (std::shared_ptr<MyLink>( new MyLink(it->second.second, it->second.first, id, literal, condi.at(i).first.first.first, false)),-1);
                links->push_back(p);
            }
            else
            {
                throw;
            }


        }
        std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > eff = groundActions.back().first->getEffects();
        for(unsigned int i=0; i < eff.size();i++)
        {
            std::string literal;
            literal = eff.at(i).first.first.second;
            for(unsigned int j=0; j< eff.at(i).first.second.size();j++)
            {
                literal += " ";
                literal += groundActions.back().first->getParameters().at(eff.at(i).first.second.at(j));
            }
            literal += eff.at(i).second;
            //time, predicate parameters end brackets

            (*achievers)[literal] = std::pair<std::string, int>(eff.at(i).first.first.first,id);

           //plan->addAchiever(std::string literal, StepTime s, size_t id)
        }
    }
    return groundActions;
}

std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > ParsePDDLfiles::parseGroundActions(std::map<std::string, std::pair<std::string, int> > * achievers,
                                                                                 std::vector<std::pair<std::shared_ptr<MyLink>, int > > * links,
                                                                                 std::string output, std::map<std::string,
                                                                                 std::shared_ptr<MyActionSchema>> domain_actions,
                                                                                 std::shared_ptr<const TemporalOrderings> * to,
                                                                                 double * duration, double * vir_memory,
                                                                                 double * phy_memory)
{


    *duration = 0;
    *vir_memory = 0;
    *phy_memory = 0;
    std::string line;
    std::ifstream file_in;;

    file_in.open(output.c_str());

    size_t index, index2, index3;
    unsigned int id = 0;

    std::vector<std::string> lines; //to save actions that have same starting time
    double current_time = 0;


    std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > groundActions;

    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
             //there is action not processed, but this line is not action (ie plan ended)
            //we need to process this last action too
            if((lines.size()!=0)&&(line.find("[")==std::string::npos))
            {
                std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > tempGA = process_lines(lines, achievers,links, domain_actions, to, id);
                id += tempGA.size();
                std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > >::iterator itGA = groundActions.begin() + groundActions.size();
                groundActions.insert(itGA,tempGA.begin(),tempGA.end());
                lines.clear();
                current_time=0;
            }
            if(line.find(";")!=std::string::npos)
            {
                std::string com = ";virtual memory consumed ";
                index = line.find(com);
                if(index!=std::string::npos)
                {
                    std::string mem = line.substr(index+com.size(), line.size()-index-com.size());
                    *vir_memory = stod(mem);
                }
                com = ";physical memory consumed ";
                index = line.find(com);
                if(index!=std::string::npos)
                {
                    std::string mem = line.substr(index+com.size(), line.size()-index-com.size());
                    *phy_memory = stod(mem);
                }
\

                continue;
            }
            index = line.find("Makespan");
            if(index!=std::string::npos)
            {
                index = line.find(" ", index+1);
                *duration = std::stod(line.substr(index,line.size()-index));
                continue;
            }


            index = line.find("["); //action
            if(index!=std::string::npos)
            {
                index2 = line.find("]");
                //dur = line.substr(index+1,index2-index-1);
                //*duration+= stod(dur);

                index= line.find(":");
                double time = std::stod(line.substr(0,index));

                if(current_time == 0) //nothign save
                {
                   current_time  = time;
                   lines.push_back(line);
                   continue;
                }
                else //the current time is set
                {
                    if(current_time == time) //we want to save it
                    {
                        lines.push_back(line);
                        continue;
                    }
                    else
                    {
                        std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > tempGA = process_lines(lines, achievers,links, domain_actions, to, id);
                        id += tempGA.size();
                        std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > >::iterator itGA = groundActions.begin() + groundActions.size();
                        groundActions.insert(itGA,tempGA.begin(),tempGA.end());
                        lines.clear();
                        lines.push_back(line);
                        current_time = time;
                    }
                }
            }
        }
    }
    file_in.close();
    return groundActions;
}

std::map<std::string, std::pair<std::string, int> > ParsePDDLfiles::parseProblem(std::string problem_file, std::vector<std::string> * goals)
{
    std::ifstream file_in;
    std::string line;
    size_t index;
    bool init_reach = false;
    bool goal_reach = false;


    std::map<std::string, std::pair<std::string, int> > raw_init_state;
    file_in.open (problem_file);

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
                goal_reach = true;
                continue;

            }
            index = line.find("(:metric");
            if(index<std::numeric_limits<std::size_t>::max())
            {
                goal_reach = false;
                continue;

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

                    raw_init_state[literal]=std::pair<std::string, int>("at start",0);

                }
            }
            if(goal_reach)
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

                    goals->push_back(literal);

                }

            }


        }
    }
    file_in.close();
    return raw_init_state;
}
