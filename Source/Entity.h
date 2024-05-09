#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"

using EntityId = u32;

struct Entity
{
private:
	friend class EntityManager;
	Entity() = default;

public:

	bool operator==(const Entity& other) const
	{
		return this->id_ == other.id_ && this->version_ == other.version_;
	}


	EntityId GetId() const { return id_; }

private:
	u32 version_;	// Entity��Version ����id���g����\��������̂ŁA����Ŗ{���ɓ�������m�F����
	EntityId id_;			// Entity��id guid ���̒l���g���Ă�Ԃ͓����l�͏o�����Ȃ�
};
namespace std{
    template<>
    struct hash<Entity>{
        size_t operator () ( const Entity &p ) const noexcept { return p.GetId();}
    };
}

class EntityManager
{
public:
	EntityManager() = default;

	[[nodiscard]] Entity CreateEntity()
	{
		Entity entity{};

		// ������ꂽEntity������ꍇ�͂������g��
		if(!free_entities_.empty())
		{
			const auto it = free_entities_.begin();
			entity = (*it);
			++entity.version_;	//�o�[�W�����̒ǉ�
			free_entities_.erase(it);
		}
		else
		{
			entity.id_ = entity_counts_;
			entity.version_ = 0;
			++entity_counts_;
		}

		entity_map_.insert(entity);
		return entity;
	}

	void RemoveEntity(Entity entity)
	{
		if(!entity_map_.contains(entity)) _ASSERT_EXPR(FALSE, L"������Entity���w�肳��܂���");
		const auto it = entity_map_.find(entity);
		entity_map_.erase(it);

		free_entities_.insert(entity);
	}

private:

	UnorderedSet<Entity> entity_map_;
	UnorderedSet<Entity> free_entities_;
	u32 entity_counts_;
};