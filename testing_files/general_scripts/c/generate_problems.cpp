#include "generate_problems.h"

#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <stdio.h>


void generate(char * domain, char * in_file, char * out_file, int type, int type_r, char * min_makespan)
{
    std::ofstream file_out;
    std::ifstream file_in;
    int files_count = 0;

    //find out how many single problems are there
    int j = 1;

    while(true)
    {
      std::string file = std::string(in_file)+"p21_"+std::to_string(j) +".pddl";

      file_in.open(file.c_str());
      if (file_in.is_open())
      {
        files_count++;
        file_in.close();
      }
      else
      {
          break;
      }
      j++;
    }

    std::string line;
    std::vector<double> deadlines;
    //load min_deadlines

    file_in.open(min_makespan);
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
            deadlines.push_back(std::stod(line));

        }
    }

    file_in.close();

    /* initialize random seed: */
     srand (time(NULL));

    //generate POP_MER files
    for (unsigned int i=1; i<=files_count;i++)
    {
        std::string output;
        if(i < 10)
          output = std::string(out_file)+"config_0"+std::to_string(i);
        else
          output = std::string(out_file)+"config_"+std::to_string(i);
        file_out.open (output.c_str());
        //first line is always same:
        //std::string setting = "20;-g;-h;ADDR/ADDR_WORK/BUC/LIFO;-f;{n,s}LR/{l}MW_add;-l;10000;-f;{n,s}LR/{u}MW_add/{l}MW_add;-l;100000;-f;{n,s,l}LR;-l;200000;-f;{n,s,u}LR/{l}LR;-l;unlimited;-v0;\n";
        std::string setting = "1;-v0;\n";
        file_out << setting;

        //for big deadlines, we want to be at minimum saved in deadlines, maximum 2 as big

        double release = 0;
        double old_deadline = 0;

        for (unsigned j=1; j <= i; j++)
        {
            double deadline;

            if(type_r == 1) //sequential release dates
            {
                release+=old_deadline+20;
            }

            if(type_r == 2) //random release date
            {
                release = rand() % int(1000);
            }

            if(type == 1) //big one
              deadline = rand() % int(deadlines.at(i-1)) + deadlines.at(i-1) + release;
            else if (type == 0) //might be ok, might fail
              deadline = rand() % int(deadlines.at(i-1)/2) + deadlines.at(i-1)+release;
            else if (type == -1) //impossible
              deadline = rand() % int(deadlines.at(i-1))+release;
            else if (type == 2) //even smaller time windows than in the middle
              deadline = rand() % int(deadlines.at(i-1)) + deadlines.at(i-1)/2+release;

            old_deadline = deadline;



            //release date; deadline; domain; problem file;
            //std::cout << std::fixed << std::setprecision(2) << release << ";" << deadline << "\n";
            file_out << std::fixed << std::setprecision(2) << release << ";" << deadline << ";" << std::string(domain) << ";" << std::string(in_file) << "p21_" << std::to_string(j) << ".pddl;\n";
        }
        file_out.close();
    }



}


int main(int argc, char* argv[])
{


    if(argc < 2)
    {
        std::cout << "you must specify a file to be split";
        return -1;
    }


    int type = std::stoi(std::string(argv[4]));
    int type_r = std::stoi(std::string(argv[5]));

    //for example
    //./generate_problems /data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/domain_deadlines.pddl /data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/separate_problems/ /data/Phd_final/Phd_evaluation/deadlines/domains/driverlog/POPMER_impossible_deadlines/ -1
    //domain, the folder of separate problems, the folder where to put generated POP-MER problems
    generate(argv[1], argv[2],argv[3], type, type_r, argv[6]);

}
