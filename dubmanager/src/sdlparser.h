#ifndef SDL_PARSER_H
#define SDL_PARSER_H

#include <iostream>
#include <memory>

#include "confignode.h"


std::shared_ptr<ConfigNode> readSDLProjectFile(std::istream& input);
void saveSDLProjectFile(std::shared_ptr<ConfigNode> root, std::ostream& output);



#endif // SDL_PARSER_H

