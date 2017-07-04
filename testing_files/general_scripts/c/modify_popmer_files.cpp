#include "modify_popmer_files.h"

#include "add_deadlines.h"

#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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




    //std::cout << "----------------\n";

    for(int i=0;i<problem.size();i++)
    {
        std::string name_of_problem;
        std::vector<std::string> setting;
        std::vector<std::string> input;
        std::vector<std::string> goals;
        std::vector<std::string> end;


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

                      setting.push_back(line);
                }
                else
                {
                    init_reached = true;
                }


                found=line.find("(:goal (and");
                if((found == std::string::npos)&&(!goal_reached)&&(init_reached))
                {//copy


                        input.push_back(line);

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

                           std::string end_string = line.substr(found3+7,line.size()-found3-7-1);
                           //std::cout << end_string << "\n";

                           goals.push_back("        (success task"+std::to_string(taskid+1)+" package"+end_string+")");
                           //last item in the input hsould be bracket ")"

                           if(type == 1)
                           {
                              std::string rel = std::to_string(release_date.at(taskid));
                             input.insert(input.begin()+input.size()-1, "        (at "+rel+" (valid task"+std::to_string(taskid+1)+"))");
                             std::string dead_str = std::to_string(deadlines.at(taskid));
                             std::string include =  "        (at "+dead_str+" (not (valid task"+std::to_string(taskid+1)+")))";
                             input.insert(input.begin()+input.size()-1,include);
                           }
                           else
                           {
                             input.insert(input.begin()+input.size()-1, "        (valid task"+std::to_string(taskid+1)+")");
                           }

                           std::string set = "        task"+std::to_string(taskid+1)+" - task";
                           setting.insert(setting.begin()+setting.size()-1,set);
                       }
                       else
                       {
                         if(found2 < found) //the brackets
                         {

                                 end.push_back(line);


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

                      end.push_back(line);
                }

            }
        }
        file_in.close();

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
        for(int j=0; j <end.size();j++)
        {

          file_out << end.at(j) << "\n";

        }

        file_out.close();


     }//end for






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

