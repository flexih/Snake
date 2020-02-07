//
//  linkmap.cpp
//  snake
//
//  Created by flexih on 2020/1/22.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Linkmap.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include "utility.hpp"

namespace snake {
    Linkmap::~Linkmap() {
        if (pile) {
            munmap(pile, size);
        }
    }
    bool Linkmap::read(std::string &path) {
        if (path.empty()) return false;
        if (pile) {
            munmap(pile, size);
            pile = nullptr;
        }
        int fd = open(path.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            return false;
        }
        struct stat st;
        fstat(fd, &st);
        pile = mmap(nullptr, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        close(fd);
        size = st.st_size;
        return parse();
    }
    enum {
        Op = 0,
        OpObjectFiles = 3,
        OpSymbols = 5
    };
    bool Linkmap::parse() {
        const char *Path = "Path";
        const char *ObjectFiles = "Object files";
        const char *Symbols = "Symbols";
        const auto FILELENGTH = 4096;
        
        int state = Op;
        char buff[FILELENGTH];
        auto p = (char *)pile;
        auto end = p + size;
        char *n = 0;
        while (p < end && (n = (char *)memchr(p, '\n', end - p)) != nullptr) {
            n[0] = 0;
            if (p[0] == '#') {
                if (strchr(p, ':') != nullptr) {
                    if (sscanf(p, "#%*[ ]%4095[a-zA-Z ]", buff) != 1) {
                        return false;
                    }
                    if (strcmp(Path, buff) == 0) {
                        if (auto lastPathComponent = strrchr(p, '/'); lastPathComponent && lastPathComponent + 1 < n) {
                            archName = std::string(lastPathComponent + 1, n - lastPathComponent - 1);
                        }
                    } else if (strcmp(ObjectFiles, buff) == 0) {
                        if (state >= OpObjectFiles) {
                            return false;
                        }
                        state = OpObjectFiles;
                    } else if (strcmp(Symbols, buff) == 0) {
                        if (state >= OpSymbols) {
                            return false;
                        }
                        state = OpSymbols;
                    } else {
                        state += 1;
                        if (state == OpSymbols) {
                            return false;
                        } else if (state > OpSymbols) {
                            return true;
                        }
                    }
                }
            } else if (n > p) {
                if (state == OpObjectFiles) {
                    int index = 0;
                    if (sscanf(p, "[%d]%*[ ]%4095[^\n]", &index, buff) != 2) {
                        return false;
                    }
                    std::string compoment(buff);
                    size_t i = compoment.rfind('/', std::string::npos);
                    std::string last;
                    auto useParentDirName = false;
                    if (i != std::string::npos && i + 1 < compoment.size()) {
                        last = compoment.substr(i + 1);
                            //xxx.a(xxx.o)
                            //xxx.app-Simulated.xcent
                            //xxx.o
                            //lib/libc.tbd
                            //Foundation.framework/Foundation.tbd
                            //xxx.framework/xxx(xxx.o)
                        if (last.back() == ')') {
                            auto j = last.rfind('(', std::string::npos);
                            if (j == std::string::npos) {
                                return false;
                            }
                            if (auto t = last.rfind(".a", j); t != std::string::npos && t + 2 == j) {
                                addLibName(index, last.substr(0, j));
                            } else {
                                useParentDirName = true;
                            }
                        } else {
                            auto j = last.rfind('.', std::string::npos);
                            if (j == std::string::npos) {
                                useParentDirName = true;
                            } else if (last.compare(j + 1, last.size() - j - 1, "o") == 0) {
                                addLibName(index, archName);
                            } else if (last.compare(j + 1, last.size() - j - 1, "tbd") == 0) {
                                useParentDirName = true;
                            } else {
                                addLibName(index, last);
                            }
                        }
                    } else {
                        addLibName(index, compoment);
                    }
                    if (useParentDirName) {
                        auto j = compoment.rfind('/', i - 1);
                        if (j == std::string::npos) {
                            return false;
                        }
                        auto dirName = compoment.substr(j + 1, i - j - 1);
                        if (dirName.find('.') != std::string::npos) {
                            addLibName(index, dirName);
                        } else {
                            addLibName(index, last);
                        }
                    }
                } else if (state == OpSymbols) {
                    int8_t op;
                    size_t index;
                    size_t sz;
                    if (n[-1] == ']' && sscanf(p, "%*[0-9a-zA-Z]%*[ \t]%zx%*[ \t][%zu]%*[ \t]%[+-]", &sz, &index, &op) == 3) {
                        if (auto pmeth = strchr(p, op); auto split = strchr(pmeth + 1, ' ')) {
                            insert(index, pmeth + 2, split - pmeth - 2, pmeth, n - pmeth, sz);
                        }
                    }
                    else if (sscanf(p, "%*[0-9a-zA-Z]%*[ \t]%zx%*[ \t][%zu]%*[ \t]%*[l_]OBJC_PROTOCOL_$_%4095s", &sz, &index, buff) == 3) {
                        std::string protocolName(buff);
                        if (!contains(protocols, protocolName)) {
                            protocols[std::move(protocolName)] = indexs[index];
                        }
                    }
                }
            }
            p = n + 1;
        }
        return true;
    }
    void Linkmap::insert(size_t index, char *pName, size_t l1, char *pMeth, size_t l2, size_t size) {
        auto className = std::string(pName, l1);
        auto methName = std::string(pMeth, l2);
        auto isInstanceMeth = pMeth[0] == '-';
        std::string catName;
        if (auto i = className.find('('); i != std::string::npos) {
            if (auto j = className.find(')', i + 1); j != std::string::npos) {
                catName = className.substr(i + 1, j - i - 1);
                className = className.substr(0, i);
            }
        }
        if (auto i = methName.find_last_of(' '); i != std::string::npos) {
            methName = methName.substr(i + 1, methName.size() - i - 2);
        }
        LMObjCClass *objcClass = nullptr;
        auto iter = classes.find(className);
        if (iter != classes.end()) {
            objcClass = &iter->second;
        } else {
            auto t = LMObjCClass();
            t.name = className;
            t.libName = indexs[index];
            classes[className] = t;
            objcClass = &classes.find(className)->second;
        }
        if (catName.empty()) {
            if (isInstanceMeth) {
                objcClass->instanceMethods[methName] = size;
            } else {
                objcClass->classMethods[methName] = size;
            }
        } else {
            auto catClass = objcClass->catWithName(catName.c_str());
            if (catClass->libName.empty()) {
                catClass->libName = indexs[index];
            }
            if (isInstanceMeth) {
                catClass->instanceMethods[methName] = size;
            } else {
                catClass->classMethods[methName] = size;
            }
        }
    }
    void Linkmap::addLibName(size_t index, std::string &&name) {
        addLibName(index, name);
    }
    void Linkmap::addLibName(size_t index, std::string &name) {
        indexs[index] = name;
    }
}
