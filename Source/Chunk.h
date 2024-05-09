#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Entity.h"
#include "ComponentArray.h"

#define ALLOW_BLANK 0


class Chunk
{
public:
	Chunk() = default;
	~Chunk() = default;

	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	Chunk(Chunk&& other) noexcept
	{
		this->archetype_ = std::move(other.archetype_);
		this->buffer_ = std::move(other.buffer_);
		this->size_ = std::move(other.size_);
		this->capacity_ = std::move(other.capacity_);
		this->entity_index_ = std::move(other.entity_index_);
		this->component_offsets_ = std::move(other.component_offsets_);
	}
	Chunk& operator=(Chunk&& other) noexcept
	{
		this->archetype_ = std::move(other.archetype_);
		this->buffer_ = std::move(other.buffer_);
		this->size_ = std::move(other.size_);
		this->capacity_ = std::move(other.capacity_);
		this->entity_index_ = std::move(other.entity_index_);
		this->component_offsets_ = std::move(other.component_offsets_);
		return *this;
	}
	
	// Chunkの作成
	// size このチャンクに格納するEntityの数
	// ...Components 使用する予定のコンポーネント
	template<class ...Components>
	static Chunk Create(u32 size)
	{
		//_ASSERT_EXPR(size > 0, L"0より大きい値を指定してください");

		Chunk chunk;
		chunk.archetype_ = Archetype::Create<Components...>();
		chunk.size_ = size;
		chunk.capacity_ = size;
		chunk.buffer_ = std::make_unique<u8[]>(size * chunk.archetype_.size_);
		u32 offset{};
		for(auto it = chunk.archetype_.component_ids_.begin(); it != chunk.archetype_.component_ids_.end(); ++it)
		{
			const ComponentId id{ static_cast<u64>(*it) };
			chunk.component_offsets_.insert({ id, offset });
			offset += size * chunk.archetype_.component_size_.at((id));
		}

		return chunk;
	}

	// 全く同じComponentsを保持しているか
	// 例 引数<Transform, Camera, Light> : 保持<Transform, Camera, Light, Mesh> この場合はfalseを返す
	// 完璧に一致しているときだけtrueを返す
	template<class ...Components>
	bool IsSame() const
	{
		// 引数の数と保持しているコンポーネントの数が違うなら同じなわけがない
		if(sizeof...(Components) != this->archetype_.component_ids_.size()) return false;

		// 引数内に同じ型が複数指定されている場合もfalse
		// 上のif分で引数の数とArchetypeが保持しているComponentの数が一致していることがわかる
		// Archetypeは同じ型を重複して持つことはない→引数が同じ型を含んでいるということは保持しているコンポーネントの数が違う
		if(IsArgsHasSameType<Components...>()) return false;

		return this->archetype_.Contains<Components...>();
	}

	// 指定されたComponentをすべて保持しているかどうか
	// 保持しているけどEntityが一つもない or 一つでも保持していない場合 → false
	// インスタンスがプラスアルファで指定されていないコンポーネントを所持している → true
	// 例 引数<Transform, Light> : 保持<Transform, Light, Camera> これだとtrueを返す
	template<class ...Components>
	bool Contains() const
	{
		return !entity_index_.empty() && archetype_.Contains<Components...>();
	}

	// Componentのデータを取得
	// T 取得したいComponentの型
	// entity Componentを保持しているEntityのID
	template<class Component>
	Component GetComponentData(Entity entity)
	{
		_ASSERT_EXPR(entity_index_.contains(entity), "保持していないEntityを指定しないでください");

		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u64 size{ sizeof(Component) };
		_ASSERT_EXPR(size == archetype_.component_size_.at(id), L"Archetypeに保存されているサイズとsizeof(T)のサイズが異なります");

		const u32 index{ entity_index_.at(entity) };
		_ASSERT_EXPR(size != 0, L"データのサイズが0でした");
		_ASSERT_EXPR(index < size, L"サイズよりも大きな値のインデックスが出ました");

		const u64 offset_bytes{ component_offsets_.at(id) + index * size };
		Component ret;
		std::memcpy(&ret, &buffer_[offset_bytes], size);

		return ret;
	}

	// 指定されたCompoentの配列を取得
	// T 取得したいComponentの型
	template<class Component>
	ComponentArray<Component> GetComponentArray()
	{
		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u32 size{ size_ - capacity_ };

		_ASSERT_EXPR(archetype_.Contains<Component>(), L"保持していない型が指定されました");

		const u32 offset{ component_offsets_.at(id) };
		void* begin{ &buffer_[offset] };
		ComponentArray<Component> ret(static_cast<Component*>(begin), size);

		return ret;
	}

	// Componentのデータをセット
	// T セットしたいComponentの型
	// entity Componentを保持しているEntityのID
	// t セットしたいComponentのデータ
	template<class Component>
	void SetComponentData(Entity entity, const Component& t)
	{
		VerifyHolding<Component>();

		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u32 structure_stride{ sizeof(Component) };
		const u32 index{ entity_index_.at(entity) };

		_ASSERT_EXPR(archetype_.Contains<Component>(), L"保持していない型が指定されました");
		const u32 offset{ component_offsets_.at(id) +  index * structure_stride };
		void* begin{ &buffer_[offset] };

		std::memcpy(begin, &t, structure_stride);
	}

	// Entityを追加
	// entity 追加するentityのID
	void AddEntity(Entity entity)
	{
		if(entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"すでに持っているEntityを渡さないでください");
		u32 index{};

#if ALLOW_BLANK
		// free_indexに値が入っているかどうかで処理を変える
		// 可能な限りデータを連続させたいのでfree_indexの値を優先して使う
		if(!free_indices_.empty())
		{
			const auto it{ free_indices_.begin() };
			index = (*it);
			free_indices_.erase(it);
		}
		else
		{
			index = size_ - capacity_;
		}
#else
		index = size_ - capacity_;
		index_entity_map_.insert({ index, entity });
#endif
		entity_index_.insert({ entity, index });

		--capacity_;
	}

	// Entityの削除
	// entity 削除するentityのID
	void RemoveEntity(Entity entity)
	{
		if(!entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"保持していないEntityを削除しようとしないでください");

		const auto it{ entity_index_.find(entity) };
		const u32 free_index{ it->second };
		entity_index_.erase(it);

#if ALLOW_BLANK
		free_indices_.insert(index);
#else

		// TODO Entityの削除処理について考える
		// 以下の処理を行うことで確実にデータが連続した状態になるので更新処理などは早くなる
		// ただし、一番うしろのデータが誰かわからないのでfor分を回して探す必要がある O(n)の処理負荷
		// 多少のメモリを犠牲にするならindexからEntityを見つけるmapを作る

		// 一番最後に割り当てたデータを空いたところに移動させる
		// sizeとcapacityから一番後ろのindexを割り出し
		const u32 end_index{ size_ - (capacity_ + 1) };

#if 0
		// entityとindexを紐づけている値を更新
		for(auto& value : entity_index_)
		{
			if(value.second == end_index)
			{
				value.second = index;
				break;
			}
		}
#else

		// index_entity_mapからBufferの最後尾を割り当てられているEntityを取得
		// EntityとIndexを紐づけているデータを更新
		// index_entity_mapから最後尾のデータを削除
		// index_entity_mapのfree_index番目のentityを更新
		const auto ie_it{ index_entity_map_.find(end_index) };
		const Entity end_entity{ (*ie_it).second };
		entity_index_.at(end_entity) = free_index;
		index_entity_map_.erase(ie_it);
		index_entity_map_.at(free_index) = end_entity;

#endif



		// comopnentのデータを入れ替え
		for(const auto component_offset : component_offsets_)
		{
			const u32 size{ archetype_.component_size_.at(component_offset.first) };
			const u32 begin_offset_bytes{ component_offset.second };

			void* new_location{ &buffer_[begin_offset_bytes + size * free_index] };
			const void* old_location{ &buffer_[begin_offset_bytes + size * end_index] };

			std::memcpy(new_location, old_location, size);
		}

		++capacity_;
#endif
	}

	[[nodiscard]] const Archetype& GetArchetype() const { return archetype_; }

private:

	// Debug専用 このクラスが指定されたコンポーネントを保持しているか確認
	// 保持している場合は何もないが保持していない場合はアサートが出る
	template<class ...Components>
	void VerifyHolding() const
	{
#ifdef _DEBUG
		const bool result{ Contains<Components...>() };
		_ASSERT_EXPR(result, L"持っていないよ");
#endif
	}

private:

	Archetype archetype_{};
	UniquePtr<u8[]> buffer_{};	// 実際にデータを保持しているメモリ空間
	u32 size_{};		// バイトではなく個数
	u32 capacity_{};	// バイトではなく個数
	UnorderedMap<Entity, u32> entity_index_{};	// Entity→Component Array Index 保持しているEntityとComponentArrayのどこにデータが入っているかを示すIndexのマップ
	UnorderedMap<ComponentId, u32> component_offsets_{};	// 各コンポーネントが格納されているアドレスのオフセット//buffer_の先頭からのオフセット

#if ALLOW_BLANK
	UnorderedSet<u32> free_indices_{};
#else
	UnorderedMap<u32, Entity> index_entity_map_{};	// 割り当てられてるIndexからEntityを特定するためのmap
#endif
};
