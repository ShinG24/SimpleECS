#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"

struct Archetype
{
	friend class Chunk;

public:

	// 新たなArchetypeを作成
	// ...Components	ComponentData いくつでも可
	template<typename ...Components>
	constexpr static Archetype Create()
	{
		Archetype archetype{};
		archetype.archetype_id_ = StringToHash(CreateUUID());
		CreateImpl<Components...>(archetype);

		return archetype;
	}

	bool operator==(const Archetype& other) const
	{
		if(other.component_ids_.size() != this->component_ids_.size()) return false;

		// TODO unordered_setの等値演算子はどういう挙動をしているのか確認する
		if(other.component_ids_ != this->component_ids_) return false;

		return true;
	}

	template<typename ...Components>
	bool Contains() const
	{
		return ContainsImpl<Components...>();
	}

	ArchetypeId GetArchetypeId() const { return archetype_id_; }

private:

	// Archetype作成関数の実体
	// 可変長引数を用いて渡された全コンポーネントのID、サイズ、名前を格納する
	// Head				可変長引数の先頭のデータ
	// ...Components	残りの可変長引数データ
	template<typename Head, typename ...Components>
	static void CreateImpl(Archetype& archetype)
	{
		// constexprにすることでコンパイル時に実行してしまう
		const u32 size{ static_cast<u32>(sizeof(Head)) };
		const u64 id{ typeid(Head).hash_code() };
		const String name{ typeid(Head).name() };

		// idが被ることはありえないのでアサート
		// このアサートが発行される→同じ型のコンポーネントを複数使用している(可変長引数に同じ型のコンポーネントが含まれる)
#ifdef _DEBUG
		for(const auto& val : archetype.component_name_ | std::views::values)
		{
			//_ASSERT_EXPR(val == name, L"同じコンポーネントが指定されています");
			//static_assert(val == name, "同じコンポーネントが指定されています");
		}
#endif

		// 上のアサートが出ずにこちらのアサートが出た場合はhash_codeを作る機能を自作する必要がある
		//_ASSERT_EXPR(!archetype.component_ids_.contains(id), L"typeid::hash_code()が被った値を出力しました");
		//static_assert(!archetype.component_ids_.contains(id), "typeid::hash_code()が被った値を出力しました");

		archetype.component_ids_.insert(id);
		archetype.component_size_.insert({ id, size });
		archetype.component_name_.insert({ id, name });
		archetype.size_ += size;

		// まだ可変長引数がある場合は同じ内容を呼び出す
		if constexpr(sizeof...(Components) != 0)
		{
			CreateImpl<Components...>(archetype);
		}
	}

	// 指定されたコンポーネントを保持しているか確認
	template<typename Head, typename ...Tails>
	bool ContainsImpl() const
	{
		// 型情報からハッシュコードを割り出し、保持しているか確認
		const u64 id{ typeid(Head).hash_code() };
		if(component_ids_.contains(id))
		{
			// 保持していた場合は、指定されたComponentsの数によって処理わけ
			// 他にも指定されたコンポーネントがあるなら同じ処理を繰り返す
			// ないならtrueを返す
			if constexpr(sizeof...(Tails) != 0)
			{
				return ContainsImpl<Tails...>();
			}
			return true;
		}

		// 保持していない場合のみ通る
		return false;
	}

private:

	ArchetypeId archetype_id_;
	UnorderedSet<ComponentId> component_ids_;
	UnorderedMap<ComponentId, u32> component_size_;
	UnorderedMap<ComponentId, String> component_name_;
	
	u32 size_;	//保持しているコンポーネントのデータサイズの合計
};


