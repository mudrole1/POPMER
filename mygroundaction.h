#ifndef MYGROUNDACTION_H
#define MYGROUNDACTION_H

#include <myactionschema.h>

class MyGroundAction: public MyActionSchema
{
private:

public:
    MyGroundAction(MyActionSchema o, std::vector<std::string> parameters, unsigned int id);
    MyGroundAction(const MyGroundAction& o);
};

#endif // MYGROUNDACTION_H
