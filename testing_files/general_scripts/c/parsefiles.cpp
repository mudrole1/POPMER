#include "parsefiles.h"
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>


std::vector<std::vector<std::string> > makespan;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > CPU;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > VM;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > PM;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > T;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > TT;//(1,std::vector<std::string>(files.size(),"-1"));
std::vector<std::vector<std::string> > valid;//(1,std::vector<std::string>(files.size(),"-1"));


/* VM - virtual memory
 * PM physical memory
 * T time to get a result
 * TT total time of running */
void parse(char * in_file, std::vector<std::string> files, int type)
{


    std::string line;
    std::ifstream file_in;
    std::ofstream file_copy;



    for(int i=0;i<files.size();i++)
    {


       int j =1;
       if(type !=5)
       {
           CPU.push_back(std::vector<std::string>());
           VM.push_back(std::vector<std::string>());
           PM.push_back(std::vector<std::string>());
           T.push_back(std::vector<std::string>());
           TT.push_back(std::vector<std::string>());
       }

       makespan.push_back(std::vector<std::string>());
       valid.push_back(std::vector<std::string>());

      std::string inputFile(in_file);
      inputFile+=files.at(i);
      //std::cout << "input file "<< inputFile << "\n";


      if((type == 3)||(type==2)||(type==1)||(type==0)||(type==5)) //these improving algs + optic //type5 - mine pre-parsed files
      {
          inputFile+=".";
          inputFile+=std::to_string(j);
      }


      //std::cout << inputFile << "\n";
      file_in.open(inputFile.c_str());



      std::size_t MSfound = -1;
      std::size_t MSfound2 = -1;
      std::size_t MSfound3 = -1;
      std::size_t MSfound4 = -1;

      bool save = false;
      bool onePlan = false;
      bool fileFound =false;

      bool unsolvable = false;
      int numOfPlans = 0;


      double overallDur = 0;
      std::string dur;
      std::string st;

      while (file_in.is_open())
      {

        while ( getline (file_in,line) )
        {
            if(type==5)//mine pre-parsed files
            {
                MSfound=line.find(";makespan ");
                if(MSfound !=std::string::npos) //we found init
                {
                  makespan.at(i).push_back(line.substr(MSfound+10,line.size()-MSfound-10));

                }

                MSfound = line.find(";no solution");
                if(MSfound !=std::string::npos)
                {
                    makespan.at(i).push_back("0");
                }

            }

            if(type == 1) //tfd
            {
                size_t ind1 = line.find("[");
                size_t ind2 = line.find("]");
                dur = line.substr(ind1+1,ind2-ind1-1);
                ind1 = line.find(":");
                st = line.substr(0,ind1-1);
            }
            if(type == 2) //optic
            {
              MSfound=line.find("; Plan found with metric ");
              if(MSfound !=std::string::npos) //we found init
              {
                onePlan = true;
                makespan.at(i).push_back(line.substr(MSfound+25,line.size()-MSfound-25));

              }

              MSfound = line.find("; Problem unsolvable!");
              if(MSfound !=std::string::npos)
              {
                  unsolvable = true;
              }

              MSfound=line.find(";CPU consumed :");
              if(MSfound !=std::string::npos) //we found init
              {
                CPU.at(i).push_back(line.substr(MSfound+15,line.size()-MSfound-14));
              }

              MSfound=line.find(";Consumed virtual memory:");
              if(MSfound !=std::string::npos) //we found init
              {
                VM.at(i).push_back(line.substr(MSfound+25,line.size()-MSfound-24));
              }

              MSfound=line.find(";Consumed physical memory:");
              if(MSfound !=std::string::npos)
              {
                PM.at(i).push_back(line.substr(MSfound+26,line.size()-MSfound-25));
              }

              MSfound=line.find("; Time ");
              if(MSfound !=std::string::npos)
              {
                T.at(i).push_back(line.substr(MSfound+7,line.size()-MSfound-7));

              }

              MSfound=line.find("Whole time of ");
              if(MSfound !=std::string::npos)
              {
                TT.at(i).push_back(line.substr(MSfound+14,line.size()-MSfound-14));
              }
            }

            else if(type == 4) //optic but just split files
            {
              //std::cout << "in optic\n";
              fileFound = true;
              MSfound=line.find("; Plan found with metric ");
              if(MSfound !=std::string::npos) //we found init
              {

                onePlan=true;

                save = true;
                numOfPlans++;
                std::string newFile =inputFile+"."+std::to_string(numOfPlans);
                file_copy.open(newFile.c_str());
                //std::cout << "file open" << newFile << "\n";
              }
              MSfound = line.find("; Problem unsolvable!");
              if(MSfound !=std::string::npos)
              {
                  unsolvable = true;
              }

              MSfound=line.find(" * All goal ");
              if(MSfound !=std::string::npos)
              {
                save = false;
                file_copy.close();
              }
              if(save)
              {
                  if((line.find("CPU")!=std::string::npos)||(line.find("Consumed")!=std::string::npos))
                  {
                      file_copy << ";" << line << "\n";
                  }
                  else
                  {
                    file_copy << line;
                    file_copy << "\n";
                  }

              }
            }
            else // I will use Makespan for vhpop and mine//(inputFile.find("vhpop") != std::string::npos)
            {
                MSfound = line.find("; no plan can be found");
                if(MSfound !=std::string::npos)
                {
                    unsolvable = true;
                }

                //std::cout << "in else\n";
                MSfound=line.find("Makespan: ");
                MSfound2=line.find("makespan: ");
                MSfound3=line.find("Makespan ");

                MSfound4=line.find("Metric time: ");
                int space;
                if((MSfound !=std::string::npos)) //we found init
                {
                  space = line.find(" ", MSfound+10);
                  makespan.at(i).push_back(line.substr(MSfound+10,space));
                }
                if((MSfound2 !=std::string::npos)) //we found init
                {
                  //std::cout << "makespan found\n";
                  space = line.find(" ", MSfound2+10);
                  //std::cout << "space" << space << "\n";
                  makespan.at(i).push_back(line.substr(MSfound2+10,space-MSfound2-10));
                }
                if((MSfound3 !=std::string::npos)) //we found init
                {
                  //std::cout << "makespan found\n";
                  //space = line.find(" ", MSfound3+9);
                  //std::cout << "space" << space << "\n";
                  makespan.at(i).push_back(line.substr(MSfound3+9,line.size()-MSfound3-9));
                }
                if((MSfound4 !=std::string::npos)) //we found init
                {
                  //std::cout << "makespan found\n";
                  makespan.at(i).push_back(line.substr(MSfound4+13,line.size()-MSfound4-13));
                }

                MSfound=line.find("CPU consumed ");
                if(MSfound !=std::string::npos) //we found init
                {
                  //std::cout << "cpu\n";
                  CPU.at(i).push_back(line.substr(MSfound+13,line.size()-MSfound-13));
                }

                MSfound=line.find("virtual memory consumed ");
                if(MSfound !=std::string::npos) //we found init
                {
                  //std::cout << "vm\n";
                  VM.at(i).push_back(line.substr(MSfound+24,line.size()-MSfound-24));
                }

                MSfound=line.find("physical memory consumed ");
                if(MSfound !=std::string::npos)
                {
                  //std::cout << "pm\n";
                  PM.at(i).push_back(line.substr(MSfound+25,line.size()-MSfound-25));
                }

                MSfound=line.find("detail time: ");
                if(MSfound !=std::string::npos)
                {
                  //std::cout << "dt\n";
                  T.at(i).push_back(line.substr(MSfound+13,line.size()-MSfound-13));
                }

                MSfound=line.find("Whole time of ");
                if(MSfound !=std::string::npos)
                {
                  //std::cout << "tt\n";
                  TT.at(i).push_back(line.substr(MSfound+14,line.size()-MSfound-14));
                }





            }



        }

        file_in.close();
        if(type==1)
        {
          overallDur = std::stod(st)+std::stod(dur)-0.001;
          std::cout << "duration " << overallDur << "\n";
          makespan.at(i).push_back(std::to_string(overallDur));
          overallDur = 0;
        }


        std::string vf = inputFile+"_valid";

        //std::cout<< "file\n";
        //std::cout << vf << "\n";

        file_in.open(vf.c_str());
        bool planvalid = false;
        while (file_in.is_open())
        {
          while ( getline (file_in,line) )
          {
              if(line.find("Plan valid")!= std::string::npos)
              {
                  valid.at(i).push_back("1");

                  planvalid = true;
                  break;
              }
          }
          file_in.close();
        }

        if(planvalid==false)
            valid.at(i).push_back("0");

        //if(valid.at(i).at(i)=="-1")
         //   V->at(0).at(i)="0";

        if((type == 3)||(type == 2)||(type==1)||(type==5)) //these improving algs or optic
        {
            j++;
            std::string inputFile1(in_file);
            inputFile= inputFile1 + files.at(i);
            inputFile+=".";
            inputFile+=std::to_string(j);
            file_in.open(inputFile.c_str());
        }

      }//end of while
      if((type == 4)&&(!onePlan)&&(fileFound)) //no plan
      {
          std::string newFile =inputFile+"."+std::to_string(1);
          file_copy.open(newFile.c_str());
          if(unsolvable)
          {
              file_copy << "; Problem unsolvable!\n";
          }

          file_copy << "\n";
          file_copy.close();
      }
      if((type == 2)&&(!onePlan)&&(unsolvable))
      {
          makespan.at(i).push_back("999999999");
      }
      if((type == 0)&&(unsolvable))
      {
          makespan.at(i).push_back("999999999");
      }
    }


}

int main(int argc, char* argv[])
{

    //argv[1] folder of files
    //argv[2] output file, use underscore in the end
    //argv[3] type 0 - mymerge, type 1 - tfd, type 2 - optic, type 3 - improving algs, type 4 - just extract separate plans in optic
    //argv[4] numfiles

    if(argc < 3)
    {
        std::cout << "you must specify a file to be parsed";
        return -1;
    }

    std::vector<std::string> files;
    std::string pre = "p21_M";
    std::string suf = ".pddl";


    int type = atoi(argv[3]);
    std::cout << "type" << type;

    std::cout << "in main\n";

    int numfiles;
    if(argc < 5)
    {
       numfiles=24;
    }
    else
    {
       numfiles= atoi(argv[4])+1;
    }

    for(int i=1;i<numfiles;i++)
    {
        if(i< 10)
          files.push_back("0"+std::to_string(i));
        else
          files.push_back(std::to_string(i));

    }


    parse(argv[1],files,type);
    //parse(argv[2],&makespan,&CPU,&VM,&PM,&T,&TT,files,1);
    //parse(argv[3],&makespan,&CPU,&VM,&PM,&T,&TT,files,2);


    if(type ==4)
    {
        //not save files;
        return 1;
    }

    {
    std::string outputFile(argv[2]);
    outputFile+="makespan";

    std::ofstream file_out;

    file_out.open(outputFile.c_str());


    for(int i=0; i<files.size();i++)
    {
        for(int j=0;j<makespan.at(i).size();j++)
        {
          file_out << makespan.at(i).at(j);
          file_out << ";";
        }
        file_out << "\n";
    }

    file_out.close();
   }

    //-----------------------------

    if(!CPU.empty())
   {
    std::string outputFile(argv[2]);
    outputFile+="cpu";

    std::ofstream file_out;

    file_out.open(outputFile.c_str());

    for(int i=0; i<files.size();i++)
    {
        for(int j=0;j<CPU.at(i).size();j++)
        {
          file_out << CPU.at(i).at(j);
          file_out << ";";
        }
        file_out << "\n";

    }

    file_out.close();
   }

    if(!VM.empty())
    {
     std::string outputFile(argv[2]);
     outputFile+="vm";

     std::ofstream file_out;

     file_out.open(outputFile.c_str());

     for(int i=0; i<files.size();i++)
     {
         for(int j=0;j<VM.at(i).size();j++)
         {
           file_out << VM.at(i).at(j);
           file_out << ";";
         }
         file_out << "\n";
     }

     file_out.close();
    }

    if(!PM.empty())
    {
     std::string outputFile(argv[2]);
     outputFile+="pm";

     std::ofstream file_out;

     file_out.open(outputFile.c_str());

     for(int i=0; i<files.size();i++)
     {
         for(int j=0;j<PM.at(i).size();j++)
         {
           file_out << PM.at(i).at(j);
           file_out << ";";
         }
         file_out << "\n";
     }

     file_out.close();
    }

      if(!T.empty())
    {
     std::string outputFile(argv[2]);
     outputFile+="t";

     std::ofstream file_out;

     file_out.open(outputFile.c_str());


     for(int i=0; i<files.size();i++)
     {
         for(int j=0;j<T.at(i).size();j++)
         {
            /*if(type==0) //our merge
            {
              double v = stod(T.at(i).at(j));
              v=ceil(v);
              file_out << v;
              file_out << ";";
            }*/
            //else
            //{
                file_out << T.at(i).at(j);
                file_out << ";";
            //}
         }
         file_out << "\n";



     }

     file_out.close();
    }

    {
     std::string outputFile(argv[2]);
     outputFile+="valid";

     std::ofstream file_out;

     file_out.open(outputFile.c_str());

     for(int i=0; i<files.size();i++)
     {
         for(int j=0;j<valid.at(i).size();j++)
         {
           file_out << valid.at(i).at(j);
           file_out << ";";
         }
         file_out << "\n";
     }

     file_out.close();
    }

    /*if(type==2)//only optic
    {
     std::string outputFile(argv[2]);
     outputFile = outputFile.substr(0,outputFile.length()-6);
     //std::cout << outputFile;
     outputFile+="conti";

     std::ofstream file_out;

     file_out.open(outputFile.c_str());

     for(int i=0; i<files.size();i++)
     {
         for(int j=0;j<T.at(i).size();j++)
         {
           file_out << (i+1) <<";";
           file_out << (j+1) <<";";
           file_out << T.at(i).at(j) <<";";
           file_out << VM.at(i).at(j) << " kB;";
           file_out << "\n";
         }
     }

     file_out.close();
    }*/




}
