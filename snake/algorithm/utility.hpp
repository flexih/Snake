//
//  utility.hpp
//  snake
//
//  Created by flexih on 2020/1/30.
//  Copyright © 2020 flexih. All rights reserved.
//

#ifndef utility_hpp
#define utility_hpp

#include <set>
#include <map>

namespace snake {
    template <class T, class Comp, class Alloc, class Predicate>
    void discard_if(std::set<T, Comp, Alloc>& c, Predicate pred) {
        for (auto it{c.begin()}, end{c.end()}; it != end; ) {
            if (pred(*it)) {
                it = c.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    
    template <class Key, class T, class Comp, class Alloc, class Predicate>
    void discard_if(std::map<Key, T, Comp, Alloc>& c, Predicate pred) {
        for (auto it{c.begin()}, end{c.end()}; it != end; ) {
            if (pred(*it)) {
                it = c.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    
    template <class T, class Comp, class Alloc>
    auto contains(std::set<T, Comp, Alloc>& c, T k) {
        return c.find(k) != c.end();
    }
    
    template <class Key, class T, class Comp, class Alloc>
    auto contains(std::map<Key, T, Comp, Alloc>& c, Key k) {
        return c.find(k) != c.end();
    }
}

#endif /* utility_hpp */
