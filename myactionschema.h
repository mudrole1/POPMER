#ifndef MYACTIONSCHEMA_H
#define MYACTIONSCHEMA_H

#include <string>
#include <vector>
#include <limits.h>
#include <iostream>

class MyActionSchema
{
private:
    std::string domain_name_;
    std::string name_;
    std::vector<std::string> parameters_;

    //time, predicate parameters end brackets
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > > conditions_;
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effects_;
    double duration_;
    unsigned int id_;

public:
    MyActionSchema()
    {
        id_ = UINT_MAX;
    }

    MyActionSchema( const MyActionSchema &o);

    void setDomainName(std::string name)
    {
        domain_name_ = name;
    }
    std::string getDomainName()
    {
        return domain_name_;
    }

    void setName(std::string name)
    {
        name_ = name;
    }
    std::string getName()
    {
        return name_;
    }

    void setParameters(std::vector<std::string> parameters)
    {
        parameters_= parameters;
    }

    std::vector<std::string> getParameters()
    {
        return parameters_;
    }

    void setConditions(std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  condi)
    {
        conditions_ = condi;
    }
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  getConditions()
    {
        return conditions_;
    }
    void setEffects(std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  effects)
    {
        effects_ = effects;
    }
    std::vector<std::pair< std::pair<std::pair<std::string, std::string>, std::vector<int> >,std::string > >  getEffects()
    {
        return effects_;
    }
    void setDuration(double dur)
    {
        duration_ = dur;
    }
    double getDuration()
    {
        return duration_;
    }
    int getId()
    {
        return id_;
    }
    void setId(unsigned int id)
    {
        id_ = id;
    }

    void printName(std::ostream& os);

    friend std::ostream& operator<<(std::ostream& os, const MyActionSchema& schema);

    friend bool operator== (MyActionSchema &s1, MyActionSchema &s2);
};

#endif // MYACTIONSCHEMA_H
