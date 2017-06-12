#include "myactionschema.h"

MyActionSchema::MyActionSchema(const MyActionSchema &o)
      : domain_name_ (o.domain_name_), name_(o.name_), parameters_ (o.parameters_),
        conditions_ (o.conditions_), effects_ (o.effects_), duration_ (o.duration_), id_ (o.id_)
     {}

void MyActionSchema::printName(std::ostream& os)
{
   os << "(";
   os << name_ ;
   for (unsigned int i =0 ; i <parameters_.size();i++)
   {
       os<< " ";
       os << parameters_.at(i);
   }
   os << ")";

}

std::ostream& operator<<(std::ostream& os, const MyActionSchema& schema)
{
    os << "domain name:" << schema.domain_name_ << "\n";
    os << "(:durative-action ";
    os << schema.name_;
    os << "\n :parameters\n  (";
    for (unsigned int i =0 ; i <schema.parameters_.size();i++)
    {
        os << schema.parameters_.at(i) << "\n";
    }
    os << " )\n ";
    os << ":duration (= ?duration "<< schema.duration_ << ")\n";
    os << " :condition\n" <<"  (and\n";
    for(unsigned int j=0; j< schema.conditions_.size();j++)
    {
      os << "(" << schema.conditions_.at(j).first.first.first << " ";
      os << schema.conditions_.at(j).first.first.second << " ";
      for(unsigned int i=0; i< schema.conditions_.at(j).first.second.size();i++)
      {
          os << schema.parameters_.at(schema.conditions_.at(j).first.second.at(i));
          os << " ";
      }
      os << schema.conditions_.at(j).second;
      os << ")\n";
    }
    os << ")\n";

    os << " :effect\n" <<"  (and\n";
    for(unsigned int j=0; j< schema.effects_.size();j++)
    {
      os << "(" << schema.effects_.at(j).first.first.first << " ";
      os << schema.effects_.at(j).first.first.second << " ";
      for(unsigned int i=0; i< schema.effects_.at(j).first.second.size();i++)
      {
          os << schema.parameters_.at(schema.effects_.at(j).first.second.at(i));
          os << " ";
      }
      os << schema.effects_.at(j).second;
      os << ")\n";
    }
    os << ")\n";
    os << ")\n";



    return os;

}

bool operator== (MyActionSchema &s1, MyActionSchema &s2)
{
    if((s1.domain_name_ == s2.domain_name_)&&(s1.name_ == s2.name_)&&(s1.parameters_ == s2.parameters_)&&(s1.conditions_ == s2.conditions_)
            &&(s1.effects_ == s2.effects_)&&(s1.duration_ == s2.duration_)&&(s1.id_ == s2.id_))
    {
        return true;
    }
    return false;
}

