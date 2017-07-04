#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

void split(char * in_file, char * out_file)
{

    std::vector<std::string> input;
    std::vector<std::string> goals;
    std::vector<std::string> remain;
    std::string line;
    std::ofstream file_out;
    std::ifstream file_in;

    bool goal_reached = false;
    bool other_lines = false;


    //copying settings of the problem file
    file_in.open(in_file);
    int h = 0;
    if (file_in.is_open())
    {
        while ( getline (file_in,line) )
        {
            //std::cout << "in get line\n";
            std::size_t found;
           //in tms domain, I need to change init state
           found=line.find("(:init ");
           if(found !=std::string::npos) //we found init
           {
              input.push_back("(:init");
              input.push_back("(ready k2)");
              input.push_back(")");
           }
           else
           {

               found=line.find("(:goal (and ");
               if((found == std::string::npos)&&(!goal_reached)&&(!other_lines))
               {//copy except kiln8
                 std::size_t found2 = line.find("(:objects k1 - kiln8");
                 if(found2 != std::string::npos)
                 {
                     input.push_back("(:objects");
                 }
                 else{
                     input.push_back(line);
                 }

               }
               else if(found < line.size())
               {
                 goal_reached = true;
               }
               else if(goal_reached)
               {
                  //do something not get the last brackets
                  found = line.find("(");
                  size_t found2 = line.find(")");
                  if(found < found2)
                     goals.push_back(line);
                  else
                  {
                      goal_reached = false;
                      other_lines = true;
                  }
               }
               if (other_lines)
               {
                   remain.push_back(line);
               }
            }
        }
    }
    file_in.close();

    std::cout << goals.size();

    for(int i=0;i<goals.size();i++)
    {
        std::string output = std::string(out_file)+std::to_string(i);
        file_out.open (output.c_str());

        for(int j=0; j< input.size();j++)
        {
            if(j==0)
            {
                size_t index = input.at(j).find(")");
                file_out << input.at(j).substr(0,index)+"-"+std::to_string(i) + ")\n"; //to have different names
            }
            else {
                file_out << input.at(j) << "\n";
            }
        }
        file_out << "(:goal (and " << "\n";
        file_out << goals.at(i) << "\n";
        for(int j=0; j < remain.size();j++)
        {
            file_out << remain.at(j) << "\n";
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

    split(argv[1],argv[2]);

}
