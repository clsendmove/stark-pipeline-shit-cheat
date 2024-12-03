#pragma once

#include "typedefs.hpp"

#include <Windows.h>
#include <Psapi.h>
#include <vector>

namespace memory {

    template <typename T> inline T find_interface(const char* module, const char* interface_name) {
        // get a handle to the module
        if (const auto handle = GetModuleHandleA(module)) {
            // get the exported function from the handle
            if (const auto interface_func = reinterpret_cast<cint_fn>(GetProcAddress(handle, "CreateInterface"))) {

                return reinterpret_cast<T>(interface_func(interface_name, nullptr));
            }
        }
        return T();
    }

    template <typename T> inline T find_vfunc(void* vmt, i32 index) {
        return (*static_cast<T**>(vmt))[index]; // create an array of pointers
    }

    // find_vfunc but simplified, for use in hooks
    constexpr void* vfunc_ptr(void* vmt, i32 index) {
        return (*static_cast<void***>(vmt))[index]; // create an array of pointers
    }

    // who knows
    inline uptr rel2abs(uptr address) {
        return static_cast<uptr>(address + 4 + *reinterpret_cast<i32*>(address));
    }

   inline uintptr_t find_pattern(const char* module, const char* pattern) {
       auto sigToBytes = [](const char* const sig) -> std::vector<i32>
           {
               std::vector<i32> bytes{};

               i8* const start{ const_cast<i8*>(sig) };
               i8* const end{ const_cast<i8*>(sig) + strlen(sig) };

               for (i8* current{ start }; current < end; ++current)
               {
                   if (*current == '?')
                   {
                       ++current;

                       if (*current == '?') {
                           ++current;
                       }

                       bytes.push_back(-1);
                   }

                   else {
                       bytes.push_back(strtoul(current, &current, 16));
                   }
               }

               return bytes;
           };

       const HMODULE hmod{ GetModuleHandleA(module)};

       if (!hmod) {
           return 0;
       }

        MODULEINFO mod_info{};

       if (!K32GetModuleInformation(GetCurrentProcess(), hmod, &mod_info, sizeof(MODULEINFO))) {
           return 0;
       }

       const DWORD image_size{ mod_info.SizeOfImage };

       if (!image_size) {
           return 0;
       }

       const std::vector<i32> pattern_bytes{ sigToBytes(pattern) };
       const u8* const image_bytes{ reinterpret_cast<u8*>(hmod) };
       const size_t signature_size{ pattern_bytes.size() };
       const i32* signature_bytes{ pattern_bytes.data() };

       for (DWORD i{}; i < image_size - signature_size; ++i)
       {
           bool byte_sequence_found{ true };

           for (DWORD j{}; j < signature_size; ++j)
           {
               if (image_bytes[i + j] != signature_bytes[j] && signature_bytes[j] != -1) {
                   byte_sequence_found = false;
                   break;
               }
           }

           if (byte_sequence_found) {
               return reinterpret_cast<uintptr_t>(&image_bytes[i]);
           }
       }

       return 0x0;
    }

    template <typename T, const u16 offset = 0> inline T find_pattern(const char* module, const char* pattern) {
        if (const auto handle = GetModuleHandleA(module)) { // get a handle to the dll
            // convert our inputted signature from chars to a byte array
            static auto sig2byte = [](const char* pattern) {
                // get the start and end of our pattern
                char* start = const_cast<char*>(pattern);
                char* end = const_cast<char*>(pattern) + strlen(pattern);

                // store our bytes into an array
                auto  bytes = std::vector<i32>{};

                // iterate through the characters
                for (char* current = start; current < end; ++current) {
                    if (*current == '?') {
                        ++current;
                        // check if the next char is also a wild card
                        if (*current == '?') {
                            ++current;
                        }
                        bytes.push_back(-1);
                    }
                    else
                        bytes.push_back(strtoul(current, &current, 16));
                }
                return bytes;
                };

            auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
            auto nt_header =
                reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<u8*>(handle) + dos_header->e_lfanew);

            DWORD image_size = nt_header->OptionalHeader.SizeOfImage;

            if (!image_size) // image is faulty
                return {};

            std::vector<i32> pattern_bytes = sig2byte(pattern);
            u8* scan = reinterpret_cast<u8*>(handle);

            u32              size = pattern_bytes.size();
            i32* data = pattern_bytes.data();

            // iterate through our image
            for (auto i = 0ul; i < image_size - size; ++i) {
                bool found = true;

                for (auto j = 0ul; j < size; ++j) {
                    if (scan[i + j] != data[j] && data[j] != -1) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return reinterpret_cast<T>(&scan[i] + offset);
                }
            }
        }
        return T();
    }

} // namespace memory

#define CALL_VFUNC(type, index, ...) \
    memory::find_vfunc<type(__thiscall*)(void*, __VA_ARGS__)>(this, index)

