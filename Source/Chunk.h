#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Entity.h"
#include "ComponentArray.h"


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
	template<typename ...Components>
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
	template<typename ...Components>
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

	// 指定されたComponentをすべて保持しているかどうか 一つでも保持していない場合はfalse
	// インスタンスが指定されていないComponentを保持していてもtrue
	// 例 引数<Transform, Light> : 保持<Transform, Light, Camera> これだとtrueを返す
	template<typename ...Components>
	bool Contains() const
	{
		return archetype_.Contains<Components...>();
	}

	// Componentのデータを取得
	// T 取得したいComponentの型
	// entity Componentを保持しているEntityのID
	template<typename T>
	T GetComponentData(Entity entity)
	{
		//static_assert(archetype_.Contains<T>(), "保持していない型を指定しないでください");
		//static_assert(entity_index_.contains(entity), "保持していないEntityを指定しないでください");
		_ASSERT_EXPR(entity_index_.contains(entity), "保持していないEntityを指定しないでください");

		const u64 id{ typeid(T).hash_code() };
		const u64 size{ sizeof(T) };
#ifdef _DEBUG
		_ASSERT_EXPR(size == archetype_.component_size_.at(id), L"Archetypeに保存されているサイズとsizeof(T)のサイズが異なります");
#endif

		const u32 index{ entity_index_.at(entity) };
		_ASSERT_EXPR(size != 0, L"データのサイズが0でした");
		_ASSERT_EXPR(index < size, L"サイズよりも大きな値のインデックスが出ました");

		const u64 offset_bytes{ component_offsets_.at(id) + index * size };
		T ret;
		std::memcpy(&ret, &buffer_[offset_bytes], size);

		return ret;
	}

	// 指定されたCompoentの配列を取得
	// T 取得したいComponentの型
	template<typename T>
	ComponentArray<T> GetComponentArray()
	{
		const ComponentId id{ typeid(T).hash_code() };
		const u32 structure_stride{ sizeof(T) };
		const u32 size{ size_ - capacity_ };

		_ASSERT_EXPR(archetype_.Contains<T>(), L"保持していない型が指定されました");

		const u32 offset{ component_offsets_.at(id) };
		void* start{ &buffer_[offset] };
		ComponentArray<T> ret(static_cast<T*>(start), size);

		return ret;
	}

	// Componentのデータをセット
	// T セットしたいComponentの型
	// entity Componentを保持しているEntityのID
	// t セットしたいComponentのデータ
	template<typename T>
	void SetComponentData(Entity entity, const T& t)
	{
		VerifyHolding<T>();

		const ComponentId id{ typeid(T).hash_code() };
		const u32 structure_stride{ sizeof(T) };
		const u32 index{ entity_index_.at(entity) };

		_ASSERT_EXPR(archetype_.Contains<T>(), L"保持していない型が指定されました");
		const u32 offset{ component_offsets_.at(id) +  index * structure_stride };
		void* start{ &buffer_[offset] };

		std::memcpy(start, &t, structure_stride);
	}

	// Entityを追加
	// entity 追加するentityのID
	void AddEntity(Entity entity)
	{
		if(entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"すでに持っているEntityを渡さないでください");
		u32 index{};

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
		entity_index_.insert({ entity, index });

		--capacity_;
	}

	// Entityの削除
	// entity 削除するentityのID
	void RemoveEntity(Entity entity)
	{
		if(!entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"保持していないEntityを削除しようとしないでください");

		const auto it{ entity_index_.find(entity) };
		const u32 index{ it->second };
		entity_index_.erase(it);

		free_indices_.insert(index);
	}

	[[nodiscard]] const Archetype& GetArchetype() const { return archetype_; }
private:

	// Debug専用 このクラスが指定されたコンポーネントを保持しているか確認
	// 保持している場合は何もないが保持していない場合はアサートが出る
	template<typename ...Components>
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
	UnorderedMap<Entity, u32> entity_index_{};	// Entity→Component Array Index
	UnorderedMap<ComponentId, u32> component_offsets_{};	// 各コンポーネントが格納されているアドレスのオフセット//buffer_の先頭からのオフセット
	Set<u32> free_indices_{};	//解放済みインデックス
};
