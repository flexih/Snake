//
//  linkmap.hpp
//  snake
//
//  Created by flexih on 2020/1/22.
//  Copyright © 2020 flexih. All rights reserved.
//

#ifndef linkmap_hpp
#define linkmap_hpp

#include <string>
#include <vector>
#include <map>
#include <set>

namespace snake {
    struct LMObjCClass {
        std::string name;
        std::string libName;
        std::vector<LMObjCClass> cats;
        std::map<std::string, size_t> instanceMethods;
        std::map<std::string, size_t> classMethods;
        
        LMObjCClass *catWithName(const char *n) {
            for (auto iter = cats.begin(); iter != cats.end(); ++iter) {
                if (iter->name.compare(n) == 0) {
                    return &(*iter);
                }
            }
            cats.push_back(LMObjCClass());
            cats.back().name = n;
            return &cats.back();
        }
    };
    class Linkmap {
    public:
        ~Linkmap();
        bool read(std::string &path);
        const auto& allClasses() const {
            return classes;
        }
        const auto& allProtocols() const {
            return protocols;
        }
    private:
        bool parse();
        void filter();
        void insert(size_t index, char *pName, size_t l1, char *pMeth, size_t l2, size_t size);
        void addLibName(size_t index, char *name);
        void *pile {nullptr};
        size_t size {0};
        std::string archName;
        std::map<size_t, std::string> indexs;
        std::map<std::string, LMObjCClass> classes;
        std::map<std::string, std::string> protocols;
    };
}

#endif /* linkmap_hpp */
