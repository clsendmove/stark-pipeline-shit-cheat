#pragma once

#include <memory>
#include "hash.h"

class RecvTable;
typedef unsigned long DWORD;

class c_netvars {
public:
    int get_netvar(const char* dt_class, const char* var);
    int get_offset(RecvTable* data_table, const char* var);
};

inline auto netvars = std::unique_ptr<c_netvars>();

#define netvar(_name, type, table, name)                                     \
  type& _name() {                                                            \
    static int offset = netvars->get_netvar(table, name);                    \
    return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
  }

#define NETVAR(_name, type, table, name)                                     \
  type& _name() {                                                            \
    static int offset = netvars->get_netvar(table, name);                    \
    return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
  }

//^ litterally the same but differnt to make pasting easier - drev