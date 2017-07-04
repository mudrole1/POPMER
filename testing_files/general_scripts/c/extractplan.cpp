#include "extractplan.h"
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

/*This class is extracting only the plan from a file and computing makespanS*/
void extract(char * in_file)
{

    std::string inputFile(in_file);
    std::string line;
    std::ifstream file_in;
    std::ofstream file_copy;
    double makespan = 0;
    file_in.open(inputFile.c_str());
    //I want to check if the input file has already structure N.N
    size_t ind = inputFile.find("/");
    size_t indL;
    while(ind != std::string::npos)
    {
        indL = ind;
        ind = inputFile.find("/", ind+1);
    }
    ind = inputFile.find(".", indL+1);
    if(ind != std::string::npos) //we found a dot
    {
        file_copy.open((inputFile+"t").c_str());
    }
    else
    {
        file_copy.open((inputFile+".1t").c_str());
    }


    bool nosolution = false;
    while (file_in.is_open())
    {
      while ( getline (file_in,line) )
      {
          size_t ind1, ind2;
          ind1 = line.find("[");
          if(ind1 != std::string::npos) //we have found an action
          {
              ind1 = line.find(":");
              double start = std::stod(line.substr(0, ind1));
              ind1 = line.find("[");
              ind2 = line.find("]");
              double end = start + std::stod(line.substr(ind1+1, ind2-ind1-1));
              if(end > makespan)
              {
                  makespan = end;
              }
              file_copy << line << "\n";
          }

          //optic
          if(line.find("; Problem unsolvable!")!=std::string::npos)
          {
              nosolution =true;
          }

      }
      file_in.close();
    }

    if(!nosolution)
    {
        file_copy << ";makespan " << makespan << "\n";
    }
    else
    {
        file_copy << ";no solution\n";
    }
    file_copy.close();

}

int main(int argc, char* argv[])
{

    //argv[1] plan file



    if(argc < 2)
    {
        std::cout << "you must specify a file to be parsed";
        return -1;
    }


    extract(argv[1]);


    return 1;

}

