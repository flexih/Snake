//
//  output.hpp
//  snake
//
//  Created by flexih on 2020/1/22.
//  Copyright © 2020 flexih. All rights reserved.
//

#ifndef output_hpp
#define output_hpp

#include <stdio.h>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include "Linkmap.hpp"
#include "ObjCClass.hpp"

namespace snake {
    class Raw;
    class Json;
    class Output {
    public:
        virtual std::ostringstream unusedSelectors(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap) = 0;
        virtual std::ostringstream unusedProtocols(std::vector<std::string> &protocolsUnused, Linkmap &linkmap) = 0;
        virtual std::ostringstream unusedClasses(std::vector<std::string> &classesUnused, Linkmap &linkmap) = 0;
        virtual std::ostringstream duplicatSelectors(std::vector<std::string> &selectors, Linkmap &linkmap) = 0;
        //(lib->(class->[meth]))
        typedef std::map<const std::string, std::map<std::string, std::vector<std::string>>> OPLibs;
        const OPLibs interact(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap);
        static Raw raw;
        static Json json;
    };
    
    class Raw: Output {
    public:
        std::ostringstream unusedSelectors(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap);
        std::ostringstream unusedProtocols(std::vector<std::string> &protocolsUnused, Linkmap &linkmap);
        std::ostringstream unusedClasses(std::vector<std::string> &classesUnused, Linkmap &linkmap);
        std::ostringstream duplicatSelectors(std::vector<std::string> &selectors, Linkmap &linkmap);
    };

    class Json: Output {
    public:
        std::ostringstream unusedSelectors(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap);
        std::ostringstream unusedProtocols(std::vector<std::string> &protocolsUnused, Linkmap &linkmap);
        std::ostringstream unusedClasses(std::vector<std::string> &classesUnused, Linkmap &linkmap);
        std::ostringstream duplicatSelectors(std::vector<std::string> &selectors, Linkmap &linkmap);
    };
}

#endif /* output_hpp */
