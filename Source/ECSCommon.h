#pragma once

#include <Windows.h>


#include "CommonHeader.h"
#pragma comment(lib, "rpcrt4.lib")

//using Entity = u32;
using ComponentId = u64;
using ArchetypeId = u64;

#define GET_COMPONENT_ID(v) typeid(v).hash_code()
#define GET_COMPONENT_NAME(v) typeid(v).name()
#define GENERATE_COMPONENT_ID(v) typeid(v).hash_code()


template<class Head, class ...Tails>
bool IsArgsHasSameTypeImpl(UnorderedSet<ComponentId>& ids)
{
	const ComponentId id{ typeid(Head).hash_code() };
	if(ids.contains(id)) return true;

	ids.insert(id);
	if constexpr(sizeof...(Tails) != 0)
	{
		return IsArgsHasSameTypeImpl<Tails...>(ids);
	}
	return false;
}

// ‰Â•Ï’·ˆø”“à‚É“¯‚¶Œ^‚ª•¡”ŠÜ‚Ü‚ê‚Ä‚¢‚È‚¢‚©Šm”F
// true ŠÜ‚Ü‚ê‚Ä‚¢‚é false ŠÜ‚Ü‚ê‚Ä‚¢‚È‚¢
template<class ...Args>
bool IsArgsHasSameType()
{
	UnorderedSet<ComponentId> ids;
	return IsArgsHasSameTypeImpl<Args...>(ids);
}

inline std::string CreateUUID()
{
	UUID uuid{};
	std::string guid{};

	UuidCreate(&uuid);

	RPC_CSTR string_uuid{ nullptr };
	if(UuidToStringA(&uuid, &string_uuid) == RPC_S_OK)
	{
		guid = reinterpret_cast<char*>(string_uuid);
		RpcStringFreeA(&string_uuid);
	}
	return guid;
}

inline u64 StringToHash(std::string str)
{
	return std::hash<std::string>::_Do_hash(str);
}
