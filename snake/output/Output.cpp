//
//  output.cpp
//  snake
//
//  Created by flexih on 2020/1/22.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Output.hpp"
#include "utility.hpp"

namespace snake {
    Raw Output::raw = snake::Raw();
    Json Output::json = snake::Json();
    std::ostringstream Raw::unusedSelectors(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allClasses = linkmap.allClasses();
        if (allClasses.empty()) {
            stream << "Total Class Count: " << selectorsUnused.size() << std::endl;
            size_t count = 0;
            for (auto &pair : selectorsUnused) {
                count += pair.second.instanceMethods.size() + pair.second.classMethods.size();
                for (auto &i : pair.second.cats) {
                    count += i.instanceMethods.size() + i .classMethods.size();
                }
            }
            stream << "Total Unused Selector: " << count << std::endl;
            stream << std::endl;
            for (auto &pair : selectorsUnused) {
                const auto &objcClass = pair.second;
                if (!(objcClass.classMethods.empty() && objcClass.instanceMethods.empty())) {
                    stream << objcClass.name << std::endl;
                    for_each(objcClass.classMethods.begin(), objcClass.classMethods.end(), [&](auto &m){
                        stream << '+' << '[' << objcClass.name << ' ' << m << ']' << std::endl;
                    });
                    for_each(objcClass.instanceMethods.begin(), objcClass.instanceMethods.end(), [&](auto &m){
                        stream << '-' << '[' << objcClass.name << ' ' << m << ']' << std::endl;
                    });
                    stream << std::endl;
                }
                for_each(objcClass.cats.begin(), objcClass.cats.end(), [&](auto &catClass){
                    auto catClassName = objcClass.name + '(' + catClass.name + ')';
                    stream << catClassName << std::endl;
                    for_each(catClass.classMethods.begin(), catClass.classMethods.end(), [&](auto &m){
                        stream << '+' << '[' << catClassName << ' ' << m << ']' << std::endl;
                    });
                    for_each(catClass.instanceMethods.begin(), catClass.instanceMethods.end(), [&](auto &m){
                        stream << '-' << '[' << catClassName << ' ' << m << ']' << std::endl;
                    });
                    stream << std::endl;
                });
            }
        } else {
            const auto &opLibs = interact(selectorsUnused, linkmap);
            stream << "Total Lib Count: " << opLibs.size() << std::endl;
            stream << "Total Class Count: " << selectorsUnused.size() << std::endl;
            size_t count = 0;
            for (auto &pair : selectorsUnused) {
                count += pair.second.instanceMethods.size() + pair.second.classMethods.size();
                for (auto &i : pair.second.cats) {
                    count += i.instanceMethods.size() + i .classMethods.size();
                }
            }
            stream << "Total Unused Selector: " << count << std::endl;
            stream << std::endl;
            for (auto &pair : opLibs) {
                stream << '#' << ' ' << pair.first << std::endl << std::endl;
                for (auto &classPair : pair.second) {
                    stream << '@' << ' ' << classPair.first << std::endl;
                    for_each(classPair.second.begin(), classPair.second.end(), [&](auto &method){
                        stream << method << std::endl;
                    });
                    stream << std::endl;
                }
                stream << std::endl;
            }
        }
        return stream;
    }
    std::ostringstream Json::unusedSelectors(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allClasses = linkmap.allClasses();
        if (allClasses.empty()) {
            stream << '[';
            auto i = 0;
            for (auto &pair : selectorsUnused) {
                const auto &objcClass = pair.second;
                auto comma = false;
                if (!(objcClass.classMethods.empty() && objcClass.instanceMethods.empty())) {
                    stream << '{' << "\"class\":" << '\"' << objcClass.name << '\"' << ',';
                    stream << "\"selectors\":" << '[';
                    auto j = 0;
                    for_each(objcClass.classMethods.begin(), objcClass.classMethods.end(), [&](auto &m){
                        stream << '\"' << '+' << '[' << objcClass.name << ' ' << m << ']' << '\"';
                        if (++j < objcClass.classMethods.size()) {
                            stream << ',';
                        }
                    });
                    if (j > 0 && objcClass.instanceMethods.size()) {
                        stream << ',';
                    }
                    j = 0;
                    for_each(objcClass.instanceMethods.begin(), objcClass.instanceMethods.end(), [&](auto &m){
                        stream << '\"' << '-' << '[' << objcClass.name << ' ' << m << ']' << '\"';
                        if (++j < objcClass.instanceMethods.size()) {
                            stream << ',';
                        }
                    });
                    stream << ']';
                    stream << '}';
                    comma = true;
                }
                auto q = 0;
                for_each(objcClass.cats.begin(), objcClass.cats.end(), [&](auto &catClass){
                    if (catClass.classMethods.empty() && catClass.instanceMethods.empty()) return;
                    if (comma) {
                        stream << ',';
                        comma = false;
                    }
                    auto catClassName = objcClass.name + '(' + catClass.name + ')';
                    stream << '{' << "\"class\":" << '\"' << catClassName << '\"' << ',';
                    stream << "\"selectors\":" << '[';
                    auto j = 0;
                    for_each(catClass.classMethods.begin(), catClass.classMethods.end(), [&](auto &m){
                        stream << '\"' << '+' << '[' << catClassName << ' ' << m << ']' << '\"';
                        if (++j < catClass.classMethods.size()) {
                            stream << ',';
                        }
                    });
                    if (j > 0 && catClass.instanceMethods.size()) {
                        stream << ',';
                    }
                    j = 0;
                    for_each(catClass.instanceMethods.begin(), catClass.instanceMethods.end(), [&](auto &m){
                        stream << '\"' << '-' << '[' << catClassName << ' ' << m << ']' << '\"';
                        if (++j < catClass.instanceMethods.size()) {
                            stream << ',';
                        }
                    });
                    stream << ']';
                    stream << '}';
                    if (++q < objcClass.cats.size()) {
                        stream << ',';
                    }
                });
                if (++i < selectorsUnused.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        } else {
            const auto &opLibs = interact(selectorsUnused, linkmap);
            stream << '[';
            auto i = 0;
            for (auto &pair : opLibs) {
                stream << '{' << "\"lib\":" << '\"' << pair.first << '\"' << ',';
                stream << "\"class\":" << '[';
                auto j = 0;
                for (auto &classPair : pair.second) {
                    stream << '{' << "\"name\":" << '\"' << classPair.first << '\"' << ',';
                    stream << "\"selector\":" << '[';
                    auto q = 0;
                    for_each(classPair.second.begin(), classPair.second.end(), [&](auto &method){
                        auto tab = method.find('\t');
                        if (tab == std::string::npos) {
                            ++q;
                            return;
                        }
                        stream << '{' << "\"name\":" << '\"' << method.substr(0, tab) << '\"' << ',';
                        stream << "\"size\":" << method.substr(tab + 1) << '}';
                        if (++q < classPair.second.size()) {
                            stream << ',';
                        }
                    });
                    stream << ']';
                    stream << '}';
                    if (++j < pair.second.size()) {
                        stream << ',';
                    }
                }
                stream << ']' << '}';
                if (++i < opLibs.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        }
        return stream;
    }
    const Output::OPLibs Output::interact(std::map<std::string, ObjCClass> &selectorsUnused, Linkmap &linkmap) {
        OPLibs opLibs;
        
        auto getMeths = [&](const std::string &libName, const std::string &className) {
            if (auto iter = opLibs.find(libName); iter != opLibs.end()) {
                if (auto jter = iter->second.find(className); jter != iter->second.end()) {
                    return &jter->second;
                } else {
                    iter->second[className] = {};
                }
            } else {
                opLibs[libName] = {{className, {}}};
            }
            return &opLibs.find(libName)->second.find(className)->second;
        };
        
        auto &allClasses = linkmap.allClasses();
        for (auto &pair : selectorsUnused) {
            auto iter = allClasses.find(pair.first);
            if (iter == allClasses.end()) {
                continue;
            }
            const auto &lmobjcClass = iter->second;
            const auto &objcClass = pair.second;
            std::string libName;
            std::string className;
            {
                libName = lmobjcClass.libName;
                className = objcClass.name;
                size_t size = 0;
                for_each(objcClass.instanceMethods.begin(), objcClass.instanceMethods.end(), [&, getMeths](auto &m){
                    if (auto jter = lmobjcClass.instanceMethods.find(m); jter != lmobjcClass.instanceMethods.end()) {
                        size = jter->second;
                    }
                    if (size == 0) {
                        for (auto i = lmobjcClass.cats.begin(); i != lmobjcClass.cats.end(); ++i) {
                            if (auto qter = i->instanceMethods.find(m); qter != i->instanceMethods.end()) {
                                size = qter->second;
                                libName = i->libName;
                                className = objcClass.name + '(' + i->name + ')';
                                break;
                            }
                        }
                    }
                    std::string method;
                    method.append(1, '-');
                    method.append(1, '[');
                    method.append(className);
                    method.append(1, ' ');
                    method.append(m);
                    method.append(1, ']');
                    if (size) {
                        method.append(1, '\t');
                        method += std::to_string(size);
                    }
                    getMeths(libName, className)->push_back(std::move(method));
                });
            }
            {
                libName = lmobjcClass.libName;
                className = objcClass.name;
                size_t size = 0;
                for_each(objcClass.classMethods.begin(), objcClass.classMethods.end(), [&](auto &m){
                    if (auto jter = lmobjcClass.classMethods.find(m); jter != lmobjcClass.classMethods.end()) {
                        size = jter->second;
                    }
                    if (size == 0) {
                        for (auto i = lmobjcClass.cats.begin(); i != lmobjcClass.cats.end(); ++i) {
                            if (auto qter = i->classMethods.find(m); qter != i->classMethods.end()) {
                                size = qter->second;
                                libName = lmobjcClass.libName;
                                className = objcClass.name + '(' + i->name + ')';
                                break;
                            }
                        }
                    }
                    std::string method;
                    method.append(1, '+');
                    method.append(1, '[');
                    method.append(className);
                    method.append(1, ' ');
                    method.append(m);
                    method.append(1, ']');
                    if (size) {
                        method.append(1, '\t');
                        method += std::to_string(size);
                    }
                    getMeths(libName, className)->push_back(std::move(method));
                });
            }
            for_each(objcClass.cats.begin(), objcClass.cats.end(), [&](auto &catClass){
                className = objcClass.name + '(' + catClass.name + ')';
                for (auto i = lmobjcClass.cats.begin(); i != lmobjcClass.cats.end(); ++i) {
                    if (i->name.compare(catClass.name) != 0) {
                        continue;
                    }
                    libName = i->libName;
                    {
                        size_t size = 0;
                        for_each(catClass.instanceMethods.begin(), catClass.instanceMethods.end(), [&](auto &m){
                            if (auto jter = i->instanceMethods.find(m); jter != i->instanceMethods.end()) {
                                size = jter->second;
                            }
                            std::string method;
                            method.append(1, '-');
                            method.append(1, '[');
                            method.append(className);
                            method.append(1, ' ');
                            method.append(m);
                            method.append(1, ']');
                            if (size) {
                                method.append(1, '\t');
                                method += std::to_string(size);
                            }
                            getMeths(libName, className)->push_back(std::move(method));
                        });
                    }
                    {
                        size_t size = 0;
                        for_each(catClass.classMethods.begin(), catClass.classMethods.end(), [&](auto &m){
                            if (auto jter = i->classMethods.find(m); jter != i->classMethods.end()) {
                                size = jter->second;
                            }
                            std::string method;
                            method.append(1, '-');
                            method.append(1, '[');
                            method.append(className);
                            method.append(1, ' ');
                            method.append(m);
                            method.append(1, ']');
                            if (size) {
                                method.append(1, '\t');
                                method += std::to_string(size);
                            }
                            getMeths(libName, className)->push_back(std::move(method));
                        });
                    }
                    break;
                }
            });
        }
        return opLibs;
    }
    std::ostringstream Raw:: unusedProtocols(std::vector<std::string> &protocolsUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allProtocols = linkmap.allProtocols();
        if (allProtocols.empty()) {
            stream << "Total Unused Protocol Count: " << protocolsUnused.size() << std::endl;
            stream << std::endl;
            for (auto &name : protocolsUnused) {
                stream << name << std::endl;
            }
        } else {
            std::map<std::string, std::vector<std::string>> result;
            std::string unknown = "Unknown";
            result[unknown] = {};
            for (auto &name : protocolsUnused) {
                const auto *lib = &unknown;
                if (auto iter = allProtocols.find(name); iter != allProtocols.end()) {
                    lib = &iter->second;
                }
                if (auto iter = result.find(*lib); iter != result.end()) {
                    iter->second.push_back(name);
                } else {
                    result[*lib] = {name};
                }
            }
            if (result[unknown].empty()) {
                result.erase(unknown);
            }
            stream << "Total Lib Count: " << result.size() << std::endl;
            stream << "Total Unused Protocol Count: " << protocolsUnused.size() << std::endl;
            stream << std::endl;
            for (auto &pair : result) {
                stream << '#' << ' ' << pair.first << std::endl;
                for (auto &name : pair.second) {
                    stream << name << std::endl;
                }
                stream << std::endl;
            }
        }
        return stream;
    }
    std::ostringstream Raw::unusedClasses(std::vector<std::string> &classesUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allClasses = linkmap.allClasses();
        if (allClasses.empty()) {
            stream << "Total Unused Class Count: " << classesUnused.size() << std::endl;
            stream << std::endl;
            for (auto &name : classesUnused) {
                stream << name << std::endl;
            }
        } else {
            std::map<std::string, std::vector<std::string>> result;
            std::string unknown = "Unknown";
            result[unknown] = {};
            for (auto &name : classesUnused) {
                const auto *lib = &unknown;
                if (auto iter = allClasses.find(name); iter != allClasses.end()) {
                    lib = &iter->second.libName;
                }
                if (auto iter = result.find(*lib); iter != result.end()) {
                    iter->second.push_back(name);
                } else {
                    result[*lib] = {name};
                }
            }
            if (result[unknown].empty()) {
                result.erase(unknown);
            }
            stream << "Total Lib Count: " << result.size() << std::endl;
            stream << "Total Unused Class Count: " << classesUnused.size() << std::endl;
            stream << std::endl;
            for (auto &pair : result) {
                stream << '#' << ' ' << pair.first << std::endl << std::endl;
                for (auto &name : pair.second) {
                    stream << name << std::endl;
                }
                stream << std::endl;
            }
        }
        return stream;
    }
    
    std::ostringstream Json::unusedProtocols(std::vector<std::string> &protocolsUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allProtocols = linkmap.allProtocols();
        if (allProtocols.empty()) {
            stream << '[';
            stream << std::endl;
            auto i = 0;
            for (auto &name : protocolsUnused) {
                stream << '\"' << name << '\"';
                if (++i < protocolsUnused.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        } else {
            std::map<std::string, std::vector<std::string>> result;
            std::string unknown = "Unknown";
            result[unknown] = {};
            for (auto &name : protocolsUnused) {
                const auto *lib = &unknown;
                if (auto iter = allProtocols.find(name); iter != allProtocols.end()) {
                    lib = &iter->second;
                }
                if (auto iter = result.find(*lib); iter != result.end()) {
                    iter->second.push_back(name);
                } else {
                    result[*lib] = {name};
                }
            }
            if (result[unknown].empty()) {
                result.erase(unknown);
            }
            stream << '[';
            auto i = 0;
            for (auto &pair : result) {
                stream << '{' << "\"lib\":" << '\"' << pair.first << '\"' << ',';
                stream << "\"protocol\":" << '[';
                auto j = 0;
                for (auto &name : pair.second) {
                    stream << '\"' << name << '\"';
                    if (++j < pair.second.size()) {
                        stream << ',';
                    }
                }
                stream << ']' << '}';
                if (++i < result.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        }
        return stream;
    }
    std::ostringstream Json::unusedClasses(std::vector<std::string> &classesUnused, Linkmap &linkmap) {
        std::ostringstream stream;
        auto &allClasses = linkmap.allClasses();
        if (allClasses.empty()) {
            stream << '[';
            stream << std::endl;
            auto i = 0;
            for (auto &name : classesUnused) {
                stream << '\"' << name << '\"';
                if (++i < classesUnused.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        } else {
            std::map<std::string, std::vector<std::string>> result;
            std::string unknown = "Unknown";
            result[unknown] = {};
            for (auto &name : classesUnused) {
                const auto *lib = &unknown;
                if (auto iter = allClasses.find(name); iter != allClasses.end()) {
                    lib = &iter->second.libName;
                }
                if (auto iter = result.find(*lib); iter != result.end()) {
                    iter->second.push_back(name);
                } else {
                    result[*lib] = {name};
                }
            }
            if (result[unknown].empty()) {
                result.erase(unknown);
            }
            stream << '[';
            auto i = 0;
            for (auto &pair : result) {
                stream << '{' << "\"lib\":" << '\"' << pair.first << '\"' << ',';
                stream << "\"class\":" << '[';
                auto j = 0;
                for (auto &name : pair.second) {
                    stream << '\"' << name << '\"';
                    if (++j < pair.second.size()) {
                        stream << ',';
                    }
                }
                stream << ']' << '}';
                if (++i < result.size()) {
                    stream << ',';
                }
            }
            stream << ']';
        }
        return stream;
    }
    
}
