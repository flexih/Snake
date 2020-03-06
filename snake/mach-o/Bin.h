//
//  Bin.h
//  snake
//
//  Created by flexih on 2020/1/20.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Arch.h"
#include <vector>
#include <string>

namespace snake {
    class Bin {
    public:
        ~Bin();
        bool read(std::string &path);
        const Arch* arch() const {
            return archs.empty() ? nullptr : &archs.front();
        }
    private:
        bool parseArch(char *p);
        std::vector<Arch> archs;
        void *pile {nullptr};
        size_t size {0};
    };
}
