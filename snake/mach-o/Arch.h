//
//  Arch.h
//  snake
//
//  Created by flexih on 2020/1/20.
//  Copyright © 2020 flexih. All rights reserved.
//

#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>
#include <vector>
#include <map>
#include "ObjCClass.hpp"

namespace snake {
    class Arch {
    public:
        Arch(struct mach_header_64 *mach_header, const char *beg);
        bool parse();
        std::vector<std::string> ObjCClasses() const;
        std::vector<std::string> ObjCProtocols() const;
        std::vector<std::string> ObjCSelectors() const;
        std::vector<std::string> ObjCClassesUnused() const;
        std::vector<std::string> ObjCProtocolsUnused() const;
        std::map<std::string, ObjCClass> ObjCSelectorsUnused() const;
    private:
        void parseSections();
        void handleBindinfo();
        void handleClasslist();
        void handleUsedSelectors();
        void handleProtocolist();
        void handleUsedClasses();
        void handleCategory();
        void handleSymtab();
        std::vector<std::string> parseDyld();
        void handleObjCSections();
        std::set<std::string> ObjCProtocolsUsed() const;
        std::set<std::string> ObjCClassesUsed() const;
        const char *POINTER(uintptr_t x);
        const char *OFFSET(uintptr_t x);
        ObjCClass* ObjCClassForName(const char *name);
        ObjCProtol* ObjCProtolForName(const char *name);
        void objcprotocollist(ObjCClass *oclass, uintptr_t protocolList);
        void objcmethdlist(ObjCClass *objcClass, uintptr_t list, bool isMeta);
        const struct segment_command_64 *ObjCSegmentAt(size_t index);
        const std::string *bindinfoSymAt(uintptr_t ptr);

        const char *arch;
        uintptr_t baseAddr;
        struct mach_header_64 mach_header;
        struct segment_command_64 linkedit;
        struct symtab_command symtab;
        struct dyld_info_command dyldinfo;

        std::vector<const struct load_command *> allLoadCommdands;
        std::vector<const struct segment_command_64 *> allSegments;
        std::vector<const struct section_64 *> allSections;

        std::map<std::string, ObjCClass> allClasses;
        std::map<std::string, ObjCProtol> allProtols;
        std::set<std::string> usedSelectors;
        std::set<std::string> refedClasses;
        std::set<std::string> internalInstanceSelectors;
        std::set<std::string> internalClassSelectors;

        std::map<uintptr_t, std::string> bindSyms;
        std::map<uintptr_t, std::string> symtabs;
    };
}
