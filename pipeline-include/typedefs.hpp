#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

//char
typedef char               i8;
//short
typedef short              i16;
//int
typedef int                i32;
//long long
typedef long long          i64;
//unsigned char
typedef unsigned char      u8;
//unsigned short
typedef unsigned short     u16;
//unsigned int
typedef unsigned int       u32;
//unsigned long long
typedef unsigned long long u64;
//unsigned long
typedef unsigned long      DWORD;

//unsigned long long
typedef u64 uptr;


typedef void* (*cint_fn)(const char* name, int* return_code);

#define registers void *rcx

#define api       __stdcall
#define vf        __fastcall

//using QAngle = glm::vec3;
using Vector = glm::vec3;
//using rect_t = glm::vec4;
//using color = ImColor;
//using matrix3x4 = glm::mat3x4;
//using matrix4x4 = glm::mat4x4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using VectorAligned = glm::aligned_vec3;

#include <Windows.h>
#include <memory>

#define singleton(name, target_class) \
  inline auto name = std::make_unique<target_class>();

#define singleton_feature(name, target_class) \
  namespace f { inline target_class name; }