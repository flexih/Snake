//
//  Bin.m
//  snake
//
//  Created by flexih on 2020/1/20.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Bin.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>

namespace snake {
    Bin::~Bin() {
        if (pile) {
            munmap(pile, size);
        }
    }
    bool Bin::read(std::string &path) {
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
        pile = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
        close(fd);
        size = st.st_size;
        return parseArch((char *)pile);
    }
    bool Bin::parseArch(char *p) {
        uint32_t magic = *(uint32_t *)p;
        switch (magic) {
            case FAT_MAGIC:
            case FAT_CIGAM:
            case FAT_MAGIC_64:
            case FAT_CIGAM_64: {
                struct fat_header fat_arch = *(struct fat_header *)p;
                if (magic == FAT_CIGAM || magic == FAT_CIGAM_64) {
                    swap_fat_header(&fat_arch, NX_LittleEndian);
                }
                p += sizeof(fat_arch);
                auto isArch64 = magic == FAT_MAGIC_64 || magic == FAT_CIGAM_64;
                for (auto i = 0; i < fat_arch.nfat_arch; ++i) {
                    if (isArch64) {
                        auto fat_arch_64 = *(struct fat_arch_64 *)p;
                        swap_fat_arch_64(&fat_arch_64, 1, NX_LittleEndian);
                        auto q = fat_arch_64.offset + (char *)pile;
                        if (parseArch(q)) {
                            return true;
                        }
                        p += sizeof(fat_arch_64);
                    } else {
                        auto fat_arch = *(struct fat_arch *)p;
                        swap_fat_arch(&fat_arch, 1, NX_LittleEndian);
                        auto q = fat_arch.offset + (char *)pile;
                        if (parseArch(q)) {
                            return true;
                        }
                        p += sizeof(fat_arch);
                    }
                }
                break;
            }
            case MH_MAGIC_64:
            case MH_CIGAM_64: {
                Arch arch = Arch((struct mach_header_64 *)p, (const char *)p);
                if (!arch.parse()) return false;
                archs.push_back(std::move(arch));
                return true;
            }
            default:
                break;
        }
        return false;
    }
};
