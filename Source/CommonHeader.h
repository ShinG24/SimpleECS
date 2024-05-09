#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <set>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 =  int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using namespace DirectX;

using float2 = XMFLOAT2;
using float3 = XMFLOAT3;
using float4 = XMFLOAT4;
using float4x4 = XMFLOAT4X4;

template<class T>
using UniquePtr = std::unique_ptr<T>;

template<class T>
using SharedPtr = std::shared_ptr<T>;

template<class T>
using Vector = std::vector<T>;

template<class Key, class Value>
using UnorderedMap = std::unordered_map<Key, Value>;

template<class T>
using UnorderedSet = std::unordered_set<T>;

template<class T>
using Set = std::set<T>;

using String = std::string;