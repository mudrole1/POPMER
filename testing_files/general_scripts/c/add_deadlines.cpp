#include "add_deadlines.h"

#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <stdio.h>


//in_file - POPMER file
void split(char * in_file, char * out_file, int type)
{

    std::ofstream file_out;
    std::ifstream file_in;
    std::string line;
    std::vector<double> release_date;
    std::vector<double> deadlines;
    std::vector<std::string> domain;
    std::vector<std::string> problem;
    //first I need to get files from POPMER configs
    file_in.open(in_file);
    int l =0;
    if (file_in.is_open())
    {
        while (getline(file_in, line))
        {
            //skip first line
            if (l==0)
            {
                l++;
                continue;
            }
            std::size_t old_i =0;
            std::size_t index = line.find(";",old_i);
            if(index!=std::string::npos)
            {
                std::string rd = line.substr(old_i,index-old_i);
                //std::cout << "release:" << rd << "\n";
                release_date.push_back(std::stod(rd));
                old_i = index +1;
            }
            index = line.find(";",old_i);
            if(index!=std::string::npos)
            {
               std::string rd = line.substr(old_i,index-old_i);
               //std::cout << "deadline" << rd << "\n";
               deadlines.push_back(std::stod(rd));
               old_i = index +1;
            }
            index = line.find(";",old_i);
            if(index!=std::string::npos)
            {
               std::string rd = line.substr(old_i,index-old_i);
               //std::cout << "domain:" <<  rd << "\n";
               domain.push_back(rd);
               old_i = index +1;
            }
            index = line.find(";",old_i);
            if(index!=std::string::npos)
            {
               std::string rd = line.substr(old_i,index-old_i);
               //std::cout << "problem:" << rd << "\n";
               problem.push_back(rd);
               old_i = index +1;
            }

        }
    }
    file_in.close();

    //now we need to copy the initial state,
    //here I assume that all files have same init state

    std::string name_of_problem;
    std::vector<std::string> setting;
    std::vector<std::string> input;
    std::vector<std::string> goals;
    std::vector<std::string> constraints;
    std::vector<std::string> end;
    std::vector<std::pair<std::string,bool> > config;

    //package name, location, what task
    std::map<std::string, std::string> pack;





    //read config file which position where added to the problem
    //find last "/"
    std::string path;
    for(int i=problem.at(0).size()-1;i>=0;i--)
    {
        size_t found = problem.at(0).find("/",i);
        if(found!=std::string::npos)
        {
            path = problem.at(0).substr(0,found+1)+"config";
            break;
        }

    }


    file_in.open(path.c_str());
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
            config.push_back(std::pair<std::string,bool>(line,false));

        }
    }
    file_in.close();

    std::cout << "----------------\n";

    for(int i=0;i<problem.size();i++)
    {
        bool goal_reached = false;
        bool init_reached = false;
        bool end_reached = false;

        int taskid = i ;
        const char * file = problem.at(taskid).c_str();
        file_in.open(file);
        int h=0;

        if (file_in.is_open())
        {
            //std::cout << "file open: " << problem.at(taskid) << "\n";
            while ( getline (file_in,line) )
            {
                if(h==0)
                {
                    name_of_problem = line;
                    h++;
                    continue;
                }

                std::size_t found;
                found = line.find("(:init");
                if((found == std::string::npos)&&(!goal_reached)&&(!init_reached))
                {
                    if(i==0)
                      setting.push_back(line);
                }
                else
                {
                    init_reached = true;
                }


                found=line.find("(:goal (and");
                if((found == std::string::npos)&&(!goal_reached)&&(init_reached))
                {//copy
                   if(i==0)
                   {
                       bool cf = false;
                      for(int j=0;j<config.size();j++)
                      {
                          if(!config.at(j).second)
                          {
                              if(line.find(config.at(j).first)!=std::string::npos)
                              {

                                  config.erase(config.begin()+j);
                                  cf =true;
                                  break;
                              }
                          }
                      }
                      if(!cf)
                      {
                        if((type!=2)&&(type!=-1))
                        {
                          if(line.find("at truck")!=std::string::npos)
                          {
                            input.push_back("        (atr"+line.substr(line.find("at truck")+2,line.size()-2-line.find("at truck")));
                            /*for(unsigned int j=0; j < problem.size(); j++)
                            {
                              input.push_back(line.substr(0,line.size()-1)+" task"+std::to_string(j+1)+")");
                            }*/
                          }
                          else if(line.find("at package")!=std::string::npos)
                          {
                            size_t indexP = line.find(" ",12);
                            std::string typP = line.substr(12,indexP-12);
                            std::string loc = line.substr(indexP+1,line.size()-indexP-1-1);
                            //std::cout << typP << "," << loc << "\n";
                            pack[typP]=loc;
                          }
                          else
                          {
                            input.push_back(line);
                          }
                        }
                        else
                        {
                            input.push_back(line);
                        }

                      }
                   }
                }
                else if(found < line.size())
                {
                    //std::cout << "goal reached\n";
                  goal_reached = true;
                }
                else if((goal_reached)&&(!end_reached))
                {

                    //std::cout << "goal reached!\n";
                   //do something not get the last brackets
                   found = line.find("(");
                   size_t found2 = line.find(")");

                   //if((found < found2)&&(found4 ==std::string::npos))
                   //{
                       size_t found3 = line.find("package");
                       if(found3 < line.size())
                       {
                           //just original problem
                           if(type == -1)
                           {
                               std::map<std::string,std::string>::iterator it = pack.begin();
                               while(it != pack.end())
                               {
                                 input.insert(input.begin()+input.size()-1,"        (at package"+it->first+" "+it->second+")");
                                 it++;
                               }
                               goals.push_back(line);
                           }
                           else if ((type ==1)||(type==0))
                           {
                             std::string end_string = line.substr(found3+7,line.size()-found3-7-1);
                             //std::cout << end_string << "\n";
                             std::string p = end_string.substr(0,end_string.find(" "));
                             //std::cout << p << "\n";
                             input.insert(input.begin()+input.size()-1,"        (at package"+p+" "+pack[p] + " task"+std::to_string(taskid+1) + ")");

                             goals.push_back("        (success task"+std::to_string(taskid+1)+" package"+end_string+")");
                             //last item in the input hsould be bracket ")"

                             std::string set = "        task"+std::to_string(taskid+1)+" - task";
                             setting.insert(setting.begin()+setting.size()-1,set);

                           }

                           if(type == 1)
                           {
                              std::string rel = std::to_string(release_date.at(taskid));
                             input.insert(input.begin()+input.size()-1, "        (at "+rel+" (valid task"+std::to_string(taskid+1)+"))");
                             std::string dead_str = std::to_string(deadlines.at(taskid));
                             std::string include =  "        (at "+dead_str+" (not (valid task"+std::to_string(taskid+1)+")))";
                             input.insert(input.begin()+input.size()-1,include);
                           }
                           else if (type == 0)
                           {
                             input.insert(input.begin()+input.size()-1, "        (valid task"+std::to_string(taskid+1)+")");
                           }

                           else if (type ==2)
                           {

                               std::string rel = std::to_string(release_date.at(taskid));
                               //input.insert(input.begin()+input.size()-1, "        (at "+rel+" (valid task"+std::to_string(taskid+1)+"))");
                              std::string dead_str = std::to_string(deadlines.at(taskid));
                              //std::string include =  "        (at "+dead_str+" (not (valid task"+std::to_string(taskid+1)+")))";
                              //input.insert(input.begin()+input.size()-1,include);


                               std::string end_string = line.substr(found3+7,line.size()-found3-7-1);
                               //std::cout << end_string << "\n";

                               constraints.push_back("        (within " + dead_str + " (at package"+end_string+"))");
                               //last item in the input hsould be bracket ")"

                               //std::string set = "        task"+std::to_string(taskid+1)+" - task";
                               //setting.insert(setting.begin()+setting.size()-1,set);
                           }

                       }
                       else
                       {
                         if(found2 < found) //the brackets
                         {
                             if(i==0)
                             {
                                 end.push_back(line);

                             }
                             end_reached=true;
                         }
                         else
                         {
                           std::cout << taskid << "" << line << ";\n";
                           std::cerr << "unexpected goal\n";
                           throw;
                         }
                       }
                  // }
                }
                else if(end_reached)
                {
                    if(i==0)
                      end.push_back(line);
                }

            }
        }
        file_in.close();

     }





        std::string output = std::string(out_file);
        file_out.open (output.c_str());

        file_out << name_of_problem << "\n";

        for(int j=0; j< setting.size();j++)
        {
           file_out << setting.at(j) << "\n";

        }

        for(int j=0; j< input.size();j++)
        {
           file_out << input.at(j) << "\n";

        }
        file_out << "(:goal (and " << "\n";
        for(int j=0; j <goals.size();j++)
        {

          file_out << goals.at(j) << "\n";

        }

        if (type==2)
        {
            file_out<< "))\n";
            file_out<< "(:constraints (and\n";
            for(int j=0; j <constraints.size();j++)
            {

              file_out << constraints.at(j) << "\n";

            }

        }

        for(int j=0; j <end.size();j++)
        {

          file_out << end.at(j) << "\n";

        }




        file_out.close();

}


int main(int argc, char* argv[])
{


    if(argc < 2)
    {
        std::cout << "you must specify a file to be split";
        return -1;
    }

    int type = std::stod(std::string(argv[3]));
    split(argv[1],argv[2], type);

}
