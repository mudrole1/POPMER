#ifndef PARSEPDDLFILES_H
#define PARSEPDDLFILES_H

#include "myactionschema.h"
#include "mygroundaction.h"
#include "vhpop/orderings.h"
#include "mylink.h"

#include <string>
#include <fstream>
#include <memory>
#include <algorithm>
#include <map>


class ParsePDDLfiles
{
public:
    static std::map<std::string, std::shared_ptr<MyActionSchema>>  parseDomain(std::string domain_file);
    static std::vector<std::pair<std::shared_ptr<MyGroundAction>, int > > parseGroundActions(std::map<std::string, std::pair<std::string, int> > * achivers,
                                                                            std::vector<std::pair<std::shared_ptr<MyLink>, int > > * links,
                                                                            std::string output, std::map<std::string,
                                                                            std::shared_ptr<MyActionSchema>> domain_actions,
                                                                             std::shared_ptr<const TemporalOrderings> * to,
                                                                            double * duration, double *vir_memory, double *phy_memory);
    static std::map<std::string, std::pair<std::string, int> > parseProblem(std::string problem_file, std::vector<std::string> * goals);
};

#endif // PARSEPDDLFILES_H
