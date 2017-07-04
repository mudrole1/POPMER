#include "concatenate.h"
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

/* this class concatenate the seqeunce plans*/
void concat(char * in_file, char * in_file2)
{

    std::string inputFile(in_file);
    std::string inputFile2(in_file2);
    std::string line;
    std::ifstream file_in;
    std::ofstream file_copy;
    double makespan = 0;

    file_copy.open((inputFile2+"t").c_str());

    std::vector<std::string> files;
    files.push_back(inputFile);
    files.push_back(inputFile2);

    for(unsigned int i=0; i<files.size();i++)
    {
        file_in.open(files.at(i).c_str());
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

                  file_copy << (start+makespan) << line.substr(ind1, line.length()-ind1) << "\n";
              }
              ind2 = line.find(";makespan ");
              if(ind2 != std::string::npos)
              {
                  double tempm = std::stod(line.substr(ind2+10, line.length()-ind2-10));
                  //if (tempm == 0)
                  //  makespan += 0.01;
                  //else
                    makespan += tempm;
              }
          }
          file_in.close();
        }
     }

    //if(makespan > 0)
    //{
        file_copy << ";makespan " << makespan << "\n";
    //}
    //else
    //{
    //    file_copy << ";no solution\n";
    //}
    file_copy.close();

}


int main(int argc, char* argv[])
{

    //argv[1] plan file



    if(argc < 3)
    {
        std::cout << "you must specify two file to be concatenated";
        return -1;
    }


    concat(argv[1], argv[2]);


    return 1;

}
