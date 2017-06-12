#include "mygroundaction.h"

MyGroundAction::MyGroundAction(MyActionSchema o, std::vector<std::string> parameters, unsigned int id)
    : MyActionSchema(o)
{
    this->setParameters(parameters);
    this->setId(id);
}

MyGroundAction::MyGroundAction(const MyGroundAction& o)
    :MyActionSchema(o)
{
}

