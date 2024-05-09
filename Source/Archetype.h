#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"

struct Archetype
{
	friend class Chunk;

public:

	// �V����Archetype���쐬
	// ...Components	ComponentData �����ł���
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

		// TODO unordered_set�̓��l���Z�q�͂ǂ��������������Ă���̂��m�F����
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

	// Archetype�쐬�֐��̎���
	// �ϒ�������p���ēn���ꂽ�S�R���|�[�l���g��ID�A�T�C�Y�A���O���i�[����
	// Head				�ϒ������̐擪�̃f�[�^
	// ...Components	�c��̉ϒ������f�[�^
	template<typename Head, typename ...Components>
	static void CreateImpl(Archetype& archetype)
	{
		// constexpr�ɂ��邱�ƂŃR���p�C�����Ɏ��s���Ă��܂�
		const u32 size{ static_cast<u32>(sizeof(Head)) };
		const u64 id{ typeid(Head).hash_code() };
		const String name{ typeid(Head).name() };

		// id����邱�Ƃ͂��肦�Ȃ��̂ŃA�T�[�g
		// ���̃A�T�[�g�����s����遨�����^�̃R���|�[�l���g�𕡐��g�p���Ă���(�ϒ������ɓ����^�̃R���|�[�l���g���܂܂��)
#ifdef _DEBUG
		for(const auto& val : archetype.component_name_ | std::views::values)
		{
			//_ASSERT_EXPR(val == name, L"�����R���|�[�l���g���w�肳��Ă��܂�");
			//static_assert(val == name, "�����R���|�[�l���g���w�肳��Ă��܂�");
		}
#endif

		// ��̃A�T�[�g���o���ɂ�����̃A�T�[�g���o���ꍇ��hash_code�����@�\�����삷��K�v������
		//_ASSERT_EXPR(!archetype.component_ids_.contains(id), L"typeid::hash_code()��������l���o�͂��܂���");
		//static_assert(!archetype.component_ids_.contains(id), "typeid::hash_code()��������l���o�͂��܂���");

		archetype.component_ids_.insert(id);
		archetype.component_size_.insert({ id, size });
		archetype.component_name_.insert({ id, name });
		archetype.size_ += size;

		// �܂��ϒ�����������ꍇ�͓������e���Ăяo��
		if constexpr(sizeof...(Components) != 0)
		{
			CreateImpl<Components...>(archetype);
		}
	}

	// �w�肳�ꂽ�R���|�[�l���g��ێ����Ă��邩�m�F
	template<typename Head, typename ...Tails>
	bool ContainsImpl() const
	{
		// �^��񂩂�n�b�V���R�[�h������o���A�ێ����Ă��邩�m�F
		const u64 id{ typeid(Head).hash_code() };
		if(component_ids_.contains(id))
		{
			// �ێ����Ă����ꍇ�́A�w�肳�ꂽComponents�̐��ɂ���ď����킯
			// ���ɂ��w�肳�ꂽ�R���|�[�l���g������Ȃ瓯���������J��Ԃ�
			// �Ȃ��Ȃ�true��Ԃ�
			if constexpr(sizeof...(Tails) != 0)
			{
				return ContainsImpl<Tails...>();
			}
			return true;
		}

		// �ێ����Ă��Ȃ��ꍇ�̂ݒʂ�
		return false;
	}

private:

	ArchetypeId archetype_id_;
	UnorderedSet<ComponentId> component_ids_;
	UnorderedMap<ComponentId, u32> component_size_;
	UnorderedMap<ComponentId, String> component_name_;
	
	u32 size_;	//�ێ����Ă���R���|�[�l���g�̃f�[�^�T�C�Y�̍��v
};


