#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Entity.h"
#include "Chunk.h"


namespace ecs
{
	class World
	{
		using ChunkPtr = SharedPtr<Chunk>;
	public:
		World() = default;
		~World() = default;

		World(const World&) = delete;
		World& operator=(const World&&) = delete;

		World(World&&) = default;
		World& operator=(World&&) = default;

		// Archetypeの追加 テンプレートでChunkに保持させたいComponentを指定する
		// もしすでに同じコンポーネントを保持しているChunkがあるときはそのChunkのポインターを返す
		// ...Components Chunkに保持させたいComponentたち
		template<typename ...Components>
		ArchetypeId AddArchetype()
		{
			// 全く同じComponentsを保持しているChunkがないか確認
			for(const auto& chunk : chunks_)
			{
				if(chunk.second->IsSame<Components...>()) return chunk.first;
			}

			const ChunkPtr chunk = std::make_shared<Chunk>(Chunk::Create<Components...>(100));
			chunks_[chunk->GetArchetype().GetArchetypeId()] = chunk;
			return chunk->GetArchetype().GetArchetypeId();
		}

		// Entityの追加
		// 追加されたComponentのデータは未定義 TODO 追加されたComponentの初期化方法について考える
		// ...Components Entityに持たせるComponents
		template<class ...Components>
		[[nodiscard]] Entity AddEntity()
		{
			const Entity entity{ entity_manager_.CreateEntity() };

			// 指定されたComponentsと全く同じComponensを保持しているChunkがないか確認
			ChunkPtr chunk{ GetSameChunk<Components...>() };
			
			if(!chunk) chunk = AddChunk<Components...>();
			chunk->AddEntity(entity);

			const ArchetypeId archetype_id{ chunk->GetArchetype().GetArchetypeId() };
			entity_archetype_map_.insert({ entity, archetype_id });
			return entity;
		}

		// Entityの削除
		// entity 削除したいentity
		void RemoveEntity(Entity entity)
		{
			const ArchetypeId archetype_id{ entity_archetype_map_.at(entity) };
			const ChunkPtr chunk{ chunks_.at(archetype_id) };
			chunk->RemoveEntity(entity);
			const auto it{ entity_archetype_map_.find(entity) };
			entity_archetype_map_.erase(it);
			entity_manager_.RemoveEntity(entity);
		}

		// Componentのデータをセット
		// Component セットしたいComponentの型
		// entity そのコンポーネントを保持しているEntityのID
		// data セットするデータ
		template<class Component>
		void SetComponentData(Entity entity, const Component& data)
		{
			const ArchetypeId archetype_id{ entity_archetype_map_.at(entity) };
			const ChunkPtr chunk{ chunks_.at(archetype_id) };
			chunk->SetComponentData<Component>(entity, data);
		}

		// Componentのデータを取得
		// Component 取得したいデータの型
		// entity Componentを保持しているEntityのID
		template<class Component>
		Component GetComponentData(Entity entity)
		{
			const ArchetypeId archetype_id{ entity_archetype_map_.at(entity) };
			const ChunkPtr chunk{ chunks_.at(archetype_id) };
			return chunk->GetComponentData<Component>(entity);
		}

	private:

		template<class ...Components>
		ChunkPtr GetSameChunk()
		{
			for(auto it = chunks_.begin(); it != chunks_.end(); ++it)
			{
				if(it->second->IsSame<Components...>()) return it->second;
			}
			return nullptr;
		}

	private:

		EntityManager entity_manager_{};
		UnorderedMap<Entity, ArchetypeId> entity_archetype_map_{};
		UnorderedMap<ArchetypeId, ChunkPtr> chunks_{};
	};
	
}