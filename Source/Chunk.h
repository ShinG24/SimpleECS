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
	
	// Chunk�̍쐬
	// size ���̃`�����N�Ɋi�[����Entity�̐�
	// ...Components �g�p����\��̃R���|�[�l���g
	template<class ...Components>
	static Chunk Create(u32 size)
	{
		//_ASSERT_EXPR(size > 0, L"0���傫���l���w�肵�Ă�������");

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

	// �S������Components��ێ����Ă��邩
	// �� ����<Transform, Camera, Light> : �ێ�<Transform, Camera, Light, Mesh> ���̏ꍇ��false��Ԃ�
	// �����Ɉ�v���Ă���Ƃ�����true��Ԃ�
	template<class ...Components>
	bool IsSame() const
	{
		// �����̐��ƕێ����Ă���R���|�[�l���g�̐����Ⴄ�Ȃ瓯���Ȃ킯���Ȃ�
		if(sizeof...(Components) != this->archetype_.component_ids_.size()) return false;

		// �������ɓ����^�������w�肳��Ă���ꍇ��false
		// ���if���ň����̐���Archetype���ێ����Ă���Component�̐�����v���Ă��邱�Ƃ��킩��
		// Archetype�͓����^���d�����Ď����Ƃ͂Ȃ��������������^���܂�ł���Ƃ������Ƃ͕ێ����Ă���R���|�[�l���g�̐����Ⴄ
		if(IsArgsHasSameType<Components...>()) return false;

		return this->archetype_.Contains<Components...>();
	}

	// �w�肳�ꂽComponent�����ׂĕێ����Ă��邩�ǂ���
	// �ێ����Ă��邯��Entity������Ȃ� or ��ł��ێ����Ă��Ȃ��ꍇ �� false
	// �C���X�^���X���v���X�A���t�@�Ŏw�肳��Ă��Ȃ��R���|�[�l���g���������Ă��� �� true
	// �� ����<Transform, Light> : �ێ�<Transform, Light, Camera> ���ꂾ��true��Ԃ�
	template<class ...Components>
	bool Contains() const
	{
		return !entity_index_.empty() && archetype_.Contains<Components...>();
	}

	// Component�̃f�[�^���擾
	// T �擾������Component�̌^
	// entity Component��ێ����Ă���Entity��ID
	template<class Component>
	Component GetComponentData(Entity entity)
	{
		_ASSERT_EXPR(entity_index_.contains(entity), "�ێ����Ă��Ȃ�Entity���w�肵�Ȃ��ł�������");

		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u64 size{ sizeof(Component) };
		_ASSERT_EXPR(size == archetype_.component_size_.at(id), L"Archetype�ɕۑ�����Ă���T�C�Y��sizeof(T)�̃T�C�Y���قȂ�܂�");

		const u32 index{ entity_index_.at(entity) };
		_ASSERT_EXPR(size != 0, L"�f�[�^�̃T�C�Y��0�ł���");
		_ASSERT_EXPR(index < size, L"�T�C�Y�����傫�Ȓl�̃C���f�b�N�X���o�܂���");

		const u64 offset_bytes{ component_offsets_.at(id) + index * size };
		Component ret;
		std::memcpy(&ret, &buffer_[offset_bytes], size);

		return ret;
	}

	// �w�肳�ꂽCompoent�̔z����擾
	// T �擾������Component�̌^
	template<class Component>
	ComponentArray<Component> GetComponentArray()
	{
		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u32 size{ size_ - capacity_ };

		_ASSERT_EXPR(archetype_.Contains<Component>(), L"�ێ����Ă��Ȃ��^���w�肳��܂���");

		const u32 offset{ component_offsets_.at(id) };
		void* begin{ &buffer_[offset] };
		ComponentArray<Component> ret(static_cast<Component*>(begin), size);

		return ret;
	}

	// Component�̃f�[�^���Z�b�g
	// T �Z�b�g������Component�̌^
	// entity Component��ێ����Ă���Entity��ID
	// t �Z�b�g������Component�̃f�[�^
	template<class Component>
	void SetComponentData(Entity entity, const Component& t)
	{
		VerifyHolding<Component>();

		const ComponentId id{ GET_COMPONENT_ID(Component) };
		const u32 structure_stride{ sizeof(Component) };
		const u32 index{ entity_index_.at(entity) };

		_ASSERT_EXPR(archetype_.Contains<Component>(), L"�ێ����Ă��Ȃ��^���w�肳��܂���");
		const u32 offset{ component_offsets_.at(id) +  index * structure_stride };
		void* begin{ &buffer_[offset] };

		std::memcpy(begin, &t, structure_stride);
	}

	// Entity��ǉ�
	// entity �ǉ�����entity��ID
	void AddEntity(Entity entity)
	{
		if(entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"���łɎ����Ă���Entity��n���Ȃ��ł�������");
		u32 index{};

		// free_index�ɒl�������Ă��邩�ǂ����ŏ�����ς���
		// �\�Ȍ���f�[�^��A�����������̂�free_index�̒l��D�悵�Ďg��
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

	// Entity�̍폜
	// entity �폜����entity��ID
	void RemoveEntity(Entity entity)
	{
		if(!entity_index_.contains(entity)) _ASSERT_EXPR(FALSE, L"�ێ����Ă��Ȃ�Entity���폜���悤�Ƃ��Ȃ��ł�������");

		const auto it{ entity_index_.find(entity) };
		const u32 index{ it->second };
		entity_index_.erase(it);

		free_indices_.insert(index);
	}

	[[nodiscard]] const Archetype& GetArchetype() const { return archetype_; }

private:

	// Debug��p ���̃N���X���w�肳�ꂽ�R���|�[�l���g��ێ����Ă��邩�m�F
	// �ێ����Ă���ꍇ�͉����Ȃ����ێ����Ă��Ȃ��ꍇ�̓A�T�[�g���o��
	template<class ...Components>
	void VerifyHolding() const
	{
#ifdef _DEBUG
		const bool result{ Contains<Components...>() };
		_ASSERT_EXPR(result, L"�����Ă��Ȃ���");
#endif
	}

private:

	Archetype archetype_{};
	UniquePtr<u8[]> buffer_{};	// ���ۂɃf�[�^��ێ����Ă��郁�������
	u32 size_{};		// �o�C�g�ł͂Ȃ���
	u32 capacity_{};	// �o�C�g�ł͂Ȃ���
	UnorderedMap<Entity, u32> entity_index_{};	// Entity��Component Array Index �ێ����Ă���Entity��ComponentArray�̂ǂ��Ƀf�[�^�������Ă��邩������Index�̃}�b�v
	UnorderedMap<ComponentId, u32> component_offsets_{};	// �e�R���|�[�l���g���i�[����Ă���A�h���X�̃I�t�Z�b�g//buffer_�̐擪����̃I�t�Z�b�g
	Set<u32> free_indices_{};	//����ς݃C���f�b�N�X
};
