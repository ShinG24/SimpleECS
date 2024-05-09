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

		// Archetype�̒ǉ� �e���v���[�g��Chunk�ɕێ���������Component���w�肷��
		// �������łɓ����R���|�[�l���g��ێ����Ă���Chunk������Ƃ��͂���Chunk�̃|�C���^�[��Ԃ�
		// ...Components Chunk�ɕێ���������Component����
		template<typename ...Components>
		ArchetypeId AddArchetype()
		{
			// �S������Components��ێ����Ă���Chunk���Ȃ����m�F
			for(const auto& chunk : chunks_)
			{
				if(chunk.second->IsSame<Components...>()) return chunk.first;
			}

			const ChunkPtr chunk = std::make_shared<Chunk>(Chunk::Create<Components...>(100));
			chunks_[chunk->GetArchetype().GetArchetypeId()] = chunk;
			return chunk->GetArchetype().GetArchetypeId();
		}

		// Entity�̒ǉ�
		// �ǉ����ꂽComponent�̃f�[�^�͖���` TODO �ǉ����ꂽComponent�̏��������@�ɂ��čl����
		// ...Components Entity�Ɏ�������Components
		template<class ...Components>
		[[nodiscard]] Entity AddEntity()
		{
			const Entity entity{ entity_manager_.CreateEntity() };

			// �w�肳�ꂽComponents�ƑS������Componens��ێ����Ă���Chunk���Ȃ����m�F
			ChunkPtr chunk{ GetSameChunk<Components...>() };
			
			if(!chunk) chunk = AddChunk<Components...>();
			chunk->AddEntity(entity);

			const ArchetypeId archetype_id{ chunk->GetArchetype().GetArchetypeId() };
			entity_archetype_map_.insert({ entity, archetype_id });
			return entity;
		}

		// Entity�̍폜
		// entity �폜������entity
		void RemoveEntity(Entity entity)
		{
			const ArchetypeId archetype_id{ entity_archetype_map_.at(entity) };
			const ChunkPtr chunk{ chunks_.at(archetype_id) };
			chunk->RemoveEntity(entity);
			const auto it{ entity_archetype_map_.find(entity) };
			entity_archetype_map_.erase(it);
			entity_manager_.RemoveEntity(entity);
		}

		// Component�̃f�[�^���Z�b�g
		// Component �Z�b�g������Component�̌^
		// entity ���̃R���|�[�l���g��ێ����Ă���Entity��ID
		// data �Z�b�g����f�[�^
		template<class Component>
		void SetComponentData(Entity entity, const Component& data)
		{
			const ArchetypeId archetype_id{ entity_archetype_map_.at(entity) };
			const ChunkPtr chunk{ chunks_.at(archetype_id) };
			chunk->SetComponentData<Component>(entity, data);
		}

		// Component�̃f�[�^���擾
		// Component �擾�������f�[�^�̌^
		// entity Component��ێ����Ă���Entity��ID
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