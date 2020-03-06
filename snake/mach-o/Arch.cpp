//
//  Arch.m
//  snake
//
//  Created by flexih on 2020/1/20.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Arch.h"
#include "ObjCRuntime.h"
#include <sstream>
#include <algorithm>
#include <inttypes.h>
#include "utility.hpp"

#define RO_META               (1<<0)
#define RO_ROOT               (1<<1)
#define RO_HAS_CXX_STRUCTORS  (1<<2)
#define FAST_DATA_MASK 0x00007ffffffffff8UL

namespace snake {
    Arch::Arch(struct mach_header_64 *header, const char *beg): mach_header(*header), arch(beg), baseAddr((uintptr_t)-1) {
        if (mach_header.magic == MH_CIGAM_64) {
            swap_mach_header_64(&mach_header, NX_LittleEndian);
        }
        internalInstanceSelectors.insert(".cxx_destruct");
        internalClassSelectors.insert("load");
        internalClassSelectors.insert("initialize");
    }
    bool Arch::parse() {
        if (mach_header.filetype != MH_EXECUTE) {
            return false;
        }
        parseSections();
        handleObjCSections();
        return true;
    }
    const char *Arch::POINTER(uintptr_t x) {
        for (auto loadcommand : allLoadCommdands) {
            if (loadcommand->cmd == LC_SEGMENT_64) {
                auto segment = (const struct segment_command_64 *)loadcommand;
                if (x >= segment->vmaddr && x <= segment->vmaddr + segment->vmsize) {
                    return arch + x - (segment->vmaddr - baseAddr - segment->fileoff) - baseAddr;
                }
            }
        }
        assert(0);
        return arch + x - baseAddr;
    }
    const char *Arch::OFFSET(uintptr_t x) {
        return arch + x;
    }
    void Arch::handleObjCSections() {
        handleBindinfo();
        handleClasslist();
        handleCategory();
        handleUsedSelectors();
        handleUsedClasses();
        handleProtocolist();
    }
    const struct segment_command_64 *Arch::ObjCSegmentAt(size_t index) {
        if (index < allSegments.size()) {
            return allSegments[index];
        }
        return nullptr;
    }
    const std::string *Arch::bindinfoSymAt(uintptr_t ptr) {
        if (auto iter = bindSyms.find(ptr); iter != bindSyms.end()) {
            return &iter->second;
        }
        return nullptr;
    }
    void Arch::handleBindinfo() {
        auto readULEB128 = [](auto &p){
            uint64_t result = 0;
            int bit = 0;
            do {
                uint64_t slice = *p & 0x7f;
                if (bit >= 64 || slice << bit >> bit != slice) {
                    //"uleb128 too big"
                } else {
                    result |= (slice << bit);
                    bit += 7;
                }
            } while (*p++ & 0x80);
            return result;
        };

        if (dyldinfo.bind_off && dyldinfo.bind_size) {
            const char *bind = OFFSET(dyldinfo.bind_off);
            const char *end = bind + dyldinfo.bind_size;
            uintptr_t addr = baseAddr;
            const char *symbolName = nullptr;
            while (bind <= end) {
                uint8_t byte = *(uint8_t *)bind++;
                uint8_t opcode = byte & BIND_OPCODE_MASK;
                uint8_t immediate = byte & BIND_IMMEDIATE_MASK;
                switch (opcode) {
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB: {
                        (void)readULEB128(bind);
                        break;
                    }
                    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: {
                        auto segmentIndex = immediate;
                        auto val = readULEB128(bind);
                        //segment.vm_addr+offset
                        addr = ObjCSegmentAt(segmentIndex)->vmaddr + val;
                        break;
                    }
                    case BIND_OPCODE_ADD_ADDR_ULEB: {
                        auto val = readULEB128(bind);
                        addr += val;
                        break;
                    }
                    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                        symbolName = bind;
                        bind += strlen(symbolName) + 1;
                        break;
                    case BIND_OPCODE_DO_BIND:
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                        if (symbolName && addr) {
                            bindSyms[addr] = symbolName;
                        }
                        if (opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB) {
                            auto val = readULEB128(bind);
                            addr += val;
                        }
                        if (opcode == BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED) {
                            addr += immediate * sizeof(uintptr_t);
                        }
                        addr += sizeof(uintptr_t);
                        break;
                    case BIND_OPCODE_SET_ADDEND_SLEB: {
                        (void)readULEB128(bind);
                        break;
                    }
                    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: {
                        auto count = readULEB128(bind);
                        auto skip = readULEB128(bind);
                        for (auto i = 0; i < count; ++i) {
                            if (symbolName && addr) {
                                bindSyms[addr] = symbolName;
                            }
                            addr += sizeof(uintptr_t) + skip;
                        }
                        break;
                    }
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                    case BIND_OPCODE_SET_TYPE_IMM:
                    case BIND_OPCODE_DONE:
                    default:
                        break;
                }
            }
        }
    }
    ObjCClass* Arch::ObjCClassForName(const char *name) {
        if (auto iter = allClasses.find(name); iter != allClasses.end()) {
            return &iter->second;
        }
        auto objCClass = ObjCClass();
        objCClass.name = name;
        allClasses[name] = objCClass;
        return &allClasses[name];
    }
    ObjCProtol* Arch::ObjCProtolForName(const char *name) {
        if (auto iter = allProtols.find(name); iter != allProtols.end()) {
            return &iter->second;
        }
        auto objCProtol = ObjCProtol();
        objCProtol.name = name;
        allProtols[name] = objCProtol;
        return &allProtols[name];
    }
    void Arch::handleClasslist() {
        for (auto iter : allSections) {
            if (strncmp(iter->sectname, "__objc_classlist", std::size(iter->sectname)) == 0) {
                auto count = iter->size / sizeof(uintptr_t);
                auto beg = OFFSET(iter->offset);
                std::vector<uintptr_t> classRefs;
                for (auto i = 0; i < count; ++i) {
                    uintptr_t classRef = *(uintptr_t *)(beg + i * sizeof(uintptr_t));
                    classRefs.push_back(classRef);
                    objc_class *oclass = (objc_class *)POINTER(classRef);
                    if (oclass->isa) {
                        classRefs.push_back(oclass->isa);
                    }
                }
                for (auto classRef : classRefs) {
                    objc_class *oclass = (objc_class *)POINTER(classRef);
                    class_ro_t *ro = (class_ro_t *)POINTER(oclass->bits & FAST_DATA_MASK);
                    auto isMeta = ro->flags & RO_META;
                    auto objcClass = ObjCClassForName(POINTER(ro->name));
                    objcmethdlist(objcClass, ro->baseMethods, isMeta);
                    objcprotocollist(objcClass, ro->baseProtocols);
                    if (!isMeta) {
                        if (oclass->superclass) {
                            objc_class *superclass = (objc_class *)POINTER(oclass->superclass);
                            class_ro_t *ro = (class_ro_t *)POINTER(superclass->bits & FAST_DATA_MASK);
                            objcClass->super = POINTER(ro->name);
                        } else if (auto n = bindinfoSymAt(classRef + sizeof(uintptr_t))) {
                            if (n->size() > 14 && n->find("_OBJC_CLASS_$_") == 0) {
                                objcClass->super = n->substr(14);
                            } else {
                                objcClass->super = *n;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    void Arch::objcmethdlist(ObjCClass *objcClass, uintptr_t list, bool isMeta) {
        if (list) {
            auto methodList = (struct method_list_t *)POINTER(list);
            for (auto j = 0; j < methodList->method_count; ++j) {
                auto m = (struct method_t *)&methodList->method_list + j;
                if (isMeta) {
                    objcClass->classMethods.insert(POINTER(m->name));
                } else {
                    objcClass->instanceMethods.insert(POINTER(m->name));
                }
            }
        }
    }
    void Arch::objcprotocollist(ObjCClass *objcClass, uintptr_t list) {
        if (list) {
            auto protocolList = (struct protocol_list_t*)POINTER(list);
            auto baseprotocol = (uintptr_t *)&protocolList->super_protocols;
            for (auto j = 0; j < protocolList->protocol_count; ++j) {
                auto protocol = (struct protocol_t *)POINTER(baseprotocol[j]);
                objcClass->protocols.insert(POINTER(protocol->name));
                objcprotocollist(objcClass, protocol->protocols);
            }
        }
    }
    void Arch::handleUsedSelectors() {
        for (auto iter : allSections) {
            if (strncmp(iter->sectname, "__objc_selrefs", std::size(iter->sectname)) == 0) {
                auto count = iter->size / sizeof(uintptr_t);
                for (auto i = 0; i < count; ++i) {
                    auto beg = OFFSET(iter->offset);
                    uintptr_t methodNamePtr = *(uintptr_t *)(beg + i * sizeof(uintptr_t));
                    usedSelectors.insert(POINTER(methodNamePtr));
                }
                break;
            }
        }
    }
    void Arch::handleUsedClasses() {
        for (auto iter : allSections) {
            if ((strncmp(iter->sectname, "__objc_classrefs", std::size(iter->sectname)) == 0 ||
                 strncmp(iter->sectname, "__objc_superrefs", std::size(iter->sectname)) == 0)) {
                auto count = iter->size / sizeof(uintptr_t);
                auto beg = (uintptr_t *)OFFSET(iter->offset);
                for (auto i = 0; i < count; ++i) {
                    uintptr_t classRef = beg[i];
                    if (classRef) {
                        objc_class *oclass = (objc_class *)POINTER(classRef);
                        class_ro_t *ro = (class_ro_t *)POINTER(oclass->bits & FAST_DATA_MASK);
                        refedClasses.insert(POINTER(ro->name));
                    } else {
                        //pointer(offset+base_addr) in bind info
                        auto offset = iter->offset + i * sizeof(uintptr_t);
                        auto ptr = offset + baseAddr;
                        if (auto n = bindinfoSymAt(ptr)) {
                            if (n->size() > 14 && n->find("_OBJC_CLASS_$_") == 0) {
                                refedClasses.insert(n->substr(14));
                            } else {
                                refedClasses.insert(*n);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    void Arch::handleProtocolist() {
        for (auto iter : allSections) {
            if (strncmp(iter->sectname, "__objc_protolist", std::size(iter->sectname)) == 0) {
                auto count = iter->size / sizeof(uintptr_t);
                auto protocolRefPtr = (uintptr_t *)OFFSET(iter->offset);
                for (auto i = 0; i < count; ++i) {
                    auto protocol = (struct protocol_t *)POINTER(protocolRefPtr[i]);
                    auto objcProtol = ObjCProtolForName(POINTER(protocol->name));
                    
#define _each_methodlist(x, y) do { \
    if (protocol->x) { \
      auto methodList = (struct method_list_t *)POINTER(protocol->x); \
      for (auto j = 0; j < methodList->method_count; ++j) { \
          struct method_t *m = (struct method_t *)&(methodList->method_list) + j; \
          objcProtol->y.insert(POINTER(m->name)); \
      } \
    }\
} while (0)
#define each_methodlist(x) _each_methodlist(x, x)

                    each_methodlist(instanceMethods);
                    each_methodlist(classMethods);
                    _each_methodlist(optionalInstanceMethods, instanceMethods);
                    _each_methodlist(optionalClassMethods, classMethods);
                }
                break;
            }
        }
    }
    void Arch::handleCategory() {
        for (auto iter : allSections) {
            if (strncmp(iter->sectname, "__objc_catlist", std::size(iter->sectname)) == 0) {
                auto count = iter->size / sizeof(uintptr_t);
                auto catRefPtr = (uintptr_t *)OFFSET(iter->offset);
                for (auto i = 0; i < count; ++i) {
                    ObjCClass *objcClass = nullptr;
                    ObjCClass *catClass = nullptr;
                    auto cat = (struct category_t *)POINTER(catRefPtr[i]);
                    if (cat->cls) {
                        objc_class *oclass = (objc_class *)POINTER(cat->cls);
                        class_ro_t *ro = (class_ro_t *)POINTER(oclass->bits & FAST_DATA_MASK);
                        objcClass = ObjCClassForName(POINTER(ro->name));
                    } else if (auto n = bindinfoSymAt(catRefPtr[i] + sizeof(uintptr_t))) {
                        if (auto i = n->find("_OBJC_CLASS_$_"); i != std::string::npos) {
                            objcClass = ObjCClassForName((n->substr(i + 14)).c_str());
                        } else {
                            objcClass = ObjCClassForName(n->c_str());
                        }
                    }
                    catClass = objcClass->catWithName(POINTER(cat->name));
                    objcprotocollist(objcClass, cat->protocols);
                    objcprotocollist(catClass, cat->protocols);
                    objcmethdlist(catClass, cat->instanceMethods, false);
                    objcmethdlist(catClass, cat->classMethods, true);
                }
                break;
            }
        }
    }
    void Arch::handleSymtab() {
        auto nlistRef = (struct nlist_64 *)OFFSET(symtab.symoff);
        auto strlist = OFFSET(symtab.stroff);
        for (auto i = 0; i < symtab.nsyms; ++i) {
            auto n_list = nlistRef[i];
            if (n_list.n_sect != NO_SECT && n_list.n_value > 0) {
                symtabs[n_list.n_value] = strlist + n_list.n_un.n_strx;
            }
        }
    }
    std::vector<std::string> Arch::parseDyld() {
        std::vector<std::string> result;
        size_t offset = sizeof(mach_header);
        for (auto loadcommand : allLoadCommdands) {
            if (loadcommand->cmd == LC_LOAD_DYLIB) {
                auto dylib_command = (struct dylib_command *)loadcommand;
                auto name = OFFSET(offset) + dylib_command->dylib.name.offset;
                result.push_back(name);
            }
            offset += loadcommand->cmdsize;
        }
        return result;
    }
    void Arch::parseSections() {
        size_t offset = 0;
        const char *beg = OFFSET(sizeof(mach_header));
        for (size_t i = 0; i < mach_header.ncmds; ++i) {
            auto loadcommand = (const struct load_command*)(beg + offset);
            switch (loadcommand->cmd) {
                case LC_SEGMENT_64: {
                    auto segment = (const struct segment_command_64 *)loadcommand;
                    allSegments.push_back(segment);
                    for (size_t i = 0; i < segment->nsects; ++i) {
                        auto section = (const struct section_64 *)(segment + 1) + i;
                        allSections.push_back(section);
                    }
                    if (segment->filesize > 0 && segment->vmaddr < baseAddr) {
                        baseAddr = segment->vmaddr;
                    }
                    break;
                }
                case LC_SYMTAB:
                    symtab = *(struct symtab_command *)loadcommand;
                    break;
                case LC_DYLD_INFO:
                case LC_DYLD_INFO_ONLY:
                    dyldinfo = *(struct dyld_info_command *)loadcommand;
                default:
                    break;
            }
            allLoadCommdands.push_back(loadcommand);
            offset += loadcommand->cmdsize;
        }
    }
    std::vector<std::string> Arch::ObjCClasses() const {
        std::vector<std::string> keys;
        keys.reserve(allClasses.size());
        for (auto &pair : allClasses) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    std::vector<std::string> Arch::ObjCProtocols() const {
        std::vector<std::string> keys;
        keys.reserve(allProtols.size());
        for (auto &pair : allProtols) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    std::set<std::string> Arch::ObjCProtocolsUsed() const {
        std::set<std::string> protocols;
        for (auto &className : ObjCClassesUsed()) {
            if (auto iter = allClasses.find(className); iter != allClasses.end()) {
                protocols.insert(iter->second.protocols.begin(), iter->second.protocols.end());
            }
        }
        return protocols;
    }
    std::vector<std::string> Arch::ObjCSelectors() const {
        std::vector<std::string> keys;
        for (auto &pair : allClasses) {
            auto objcClass = pair.second;
            for_each(objcClass.classMethods.begin(), objcClass.classMethods.end(), [&keys, &objcClass](auto &k){
                std::stringstream os;
                os << "+" << "[" << objcClass.name << " " << k << "]";
                keys.push_back(os.str());
            });
            for_each(objcClass.instanceMethods.begin(), objcClass.instanceMethods.end(), [&keys, &objcClass](auto &k){
                std::stringstream os;
                os << "-" << "[" << objcClass.name << " " << k << "]";
                keys.push_back(os.str());
            });
        }
        return keys;
    }
    std::set<std::string> Arch::ObjCClassesUsed() const {
        auto result = refedClasses;
        for (auto &c : refedClasses) {
            const std::string *pName = &c;
            while (1) {
                auto iter = allClasses.find(*pName);
                if (iter != allClasses.end() && !iter->second.super.empty() && !contains(result, iter->second.super)) {
                    result.insert(iter->second.super);
                    pName = &iter->second.super;
                } else {
                    break;
                }
            }
        }
        return result;
    }
    std::vector<std::string> Arch::ObjCClassesUnused() const {
        std::vector<std::string> unused;
        unused.reserve(allClasses.size());
        auto usedClasses = ObjCClassesUsed();
        for (auto &c : allClasses) {
            if (!contains(usedClasses, c.first)) {
                unused.push_back(c.first);
            }
        }
        return unused;
    }
    std::vector<std::string> Arch::ObjCProtocolsUnused() const {
        auto all = ObjCProtocols();
        auto used = ObjCProtocolsUsed();
        std::vector<std::string> result;
        result.reserve(all.size() - used.size());
        std::set_difference(all.begin(), all.end(), used.begin(), used.end(), std::back_inserter(result));
        return result;
    }
    std::map<std::string, ObjCClass> Arch::ObjCSelectorsUnused() const {
        //class.selector-protocol.selector
        //-used.selector-internalSelectors
        auto result = allClasses;
        auto instanceMethodFilter = [&](auto &m) {
            return usedSelectors.find(m) != usedSelectors.end() || internalInstanceSelectors.find(m) != internalInstanceSelectors.end();
        };
        auto classMethodFilter = [&](auto &m) {
            return usedSelectors.find(m) != usedSelectors.end() || internalClassSelectors.find(m) != internalClassSelectors.end();
        };
        for (auto iter = result.begin(); iter != result.end();) {
            auto &objcClass = iter->second;
            for (auto &name : objcClass.protocols) {
                auto protocol = allProtols.at(name);
                discard_if(objcClass.instanceMethods, [&protocol](auto &m) {
                    return contains(protocol.instanceMethods, m);
                });
                discard_if(objcClass.classMethods, [&protocol](auto &m){
                    return contains(protocol.classMethods, m);
                });
                for_each(objcClass.cats.begin(), objcClass.cats.end(), [&](auto &oc){
                    discard_if(oc.instanceMethods, [&](auto &m){
                        return contains(protocol.instanceMethods, m);
                    });
                    discard_if(oc.classMethods, [&](auto &m){
                        return contains(protocol.classMethods, m);
                    });
                });
            }
            discard_if(objcClass.instanceMethods, instanceMethodFilter);
            discard_if(objcClass.classMethods, classMethodFilter);
            for_each(objcClass.cats.begin(), objcClass.cats.end(), [&](auto &oc) {
                discard_if(oc.instanceMethods, instanceMethodFilter);
                discard_if(oc.classMethods, classMethodFilter);
            });
            for (auto i = objcClass.cats.begin(); i != objcClass.cats.end();) {
                if (i->empty()) {
                    i = objcClass.cats.erase(i);
                } else {
                    ++i;
                }
            }
            if (objcClass.empty()) {
                iter = result.erase(iter);
            } else {
                ++iter;
            }
        }
        return result;
    }
}

