//
//  ObjCClass.hpp
//  snake
//
//  Created by flexih on 2020/1/20.
//  Copyright © 2020 flexih. All rights reserved.
//

#ifndef ObjCClass_hpp
#define ObjCClass_hpp

#include <vector>
#include <string>
#include <map>
#include <set>

namespace snake {
    struct ObjCClass {
        std::string name;
        std::vector<ObjCClass> cats;
        std::set<std::string> protocols;
        std::set<std::string> instanceMethods;
        std::set<std::string> classMethods;
        
        ObjCClass *catWithName(const char *n) {
            for (auto iter = cats.begin(); iter != cats.end(); ++iter) {
                if (iter->name.compare(n) == 0) {
                    return &(*iter);
                }
            }
            cats.push_back(ObjCClass());
            cats.back().name = n;
            return &cats.back();
        }
        
        bool empty() const {
            if (!instanceMethods.empty()) return false;
            if (!classMethods.empty()) return false;
            for (auto &oc : cats) {
                if (!oc.empty()) {
                    return false;
                }
            }
            return true;
        }
    };

    struct ObjCProtol {
        std::string name;

        std::set<std::string> instanceMethods;
        std::set<std::string> classMethods;
    };
}

#endif
