//
//  ObjCRuntime.h
//  snake
//
//  Created by flexih on 2020/1/19.
//  Copyright © 2020 flexih. All rights reserved.
//

#ifndef ObjCRuntime_h
#define ObjCRuntime_h

#include <cstddef>
#include <stdint.h>

struct class_ro_t_32 {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
    uint32_t ivarLayout;
    uint32_t name;
    uint32_t baseMethods;
    uint32_t baseProtocols;
    uint32_t ivars;
    uint32_t weakIvarLayout;
    uint32_t baseProperties;
};

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
    uint32_t reserved;
    uint64_t ivarLayout;
    uint64_t name;
    uint64_t baseMethods;
    uint64_t baseProtocols;
    uint64_t ivars;
    uint64_t weakIvarLayout;
    uint64_t baseProperties;
};

struct objc_class {
    uintptr_t isa;
    uintptr_t superclass;
    uintptr_t cache;
    uintptr_t vtable;
    uintptr_t bits;
};

struct method_t {
    uintptr_t name;
    uintptr_t types;
    uintptr_t imp;
};

struct method_list_t {
    unsigned int entsize;
    unsigned int method_count;
    uintptr_t method_list;
};

struct protocol_list_t {
    uintptr_t protocol_count;
    uintptr_t super_protocols;
};

struct protocol_t {
    uintptr_t isa;
    uintptr_t name;
    uintptr_t protocols;
    uintptr_t instanceMethods;
    uintptr_t classMethods;
    uintptr_t optionalInstanceMethods;
    uintptr_t optionalClassMethods;
    uintptr_t instanceProperties;
};

struct category_t {
    uintptr_t name;
    uintptr_t cls;
    uintptr_t instanceMethods;
    uintptr_t classMethods;
    uintptr_t protocols;
    uintptr_t instanceProperties;
};

struct objc_property {
    uintptr_t name;
    uintptr_t attributes;
};

struct objc_property_list {
    uint32_t entsize;
    uint32_t count;
};

#endif /* ObjCRuntime_h */
