//
//  utility.cpp
//  snake
//
//  Created by flexih on 2020/1/30.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "utility.hpp"

namespace snake {
    std::string trimPath(const std::string &path) {
        std::string result;
        for (auto &c : path) {
            if (c != '\\') {
                result.append(1, c);
            }
        }
        return result;
    }
}
