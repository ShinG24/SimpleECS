#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"
#include "World.h"

namespace ecs
{
	class BaseSystem
	{
		friend class SystemManager;
	public:
		BaseSystem() = default;
		virtual ~BaseSystem() = default;

		virtual void Execute() {};

	protected:

		template<class T0>
		void Foreach(std::function<void(T0&)> func)
		{
			world_->GetComponentArrays<T0>();
			Vector<SharedPtr<Chunk>> chunk_list{ world_->GetChunkList<T0>() };
			for(auto&& chunk : chunk_list)
			{
				auto args{ chunk->GetComponentArray<T0>() };
				ForeachImpl(chunk.get(), func, args);
			}
		}

		//template<class T0, typename Func>
//		void Foreach(Func&& func)
//		{
//			world_->GetComponentArrays<T0>();
//			Vector<SharedPtr<Chunk>> chunk_list{ world_->GetChunkList<T0>() };
//			for(auto&& chunk : chunk_list)
//			{
//				auto args{ chunk->GetComponentArray<T0>() };
//				ForeachImpl(chunk.get(), func, args);
//			}
//		}

	private:

		void SetWorld(World* world) { world_ = world; }

		template<typename Func, typename... Args>
		static void ForeachImpl( Chunk* pChunk, Func&& func, Args ... args )
		{
			for ( std::uint32_t i = 0; i < pChunk->GetEntityCounts(); ++i )
			{
				func( args[i]... );
			}
		}
		//template<class ...Components>
		//static void ForeachImpl(SharedPtr<Chunk> chunk, std::function<void(Components&&... args)> function, Components&& ...components)
		//{
		//	for(u32 i = 0; chunk->GetEntityCounts(); ++i)
		//	{
		//		function(components[i]...);
		//	}
		//}

	protected:
		World* world_;
	};

	class SystemManager
	{
	public:
		SystemManager(World* world) : world_(world) {}

		void Execute()
		{
			for(auto& system : systems_)
			{
				system->Execute();
			}
		}
		template<class ...Systems>
		void AddSystems()
		{
			_ASSERT_EXPR(world_, L"WorldÇ™nullptrÇ≈ÇµÇΩ");
			AddSystemImpl<Systems...>();
		}

		template<class ...Systems>
		void RemoveSystems();

	private:

		template<class Head, class ...Tails>
		void AddSystemImpl()
		{
			IsBaseOfBaseSystem<Head>();

			const u64 id{ typeid(Head).hash_code() };
			if(!system_ids_.contains(id))
			{
				UniquePtr<Head> system{ std::make_unique<Head>() };
				system->SetWorld(world_);
				systems_.emplace_back(std::move(system));
			}

			if constexpr(sizeof...(Tails) != 0) AddSystemImpl<Tails...>(world_);
		}

	private:

		template<class System>
		static void IsBaseOfBaseSystem()
		{
			static_assert(std::is_base_of_v<BaseSystem, System>, "BaseSystemÇåpè≥ÇµÇΩÉNÉâÉXÇæÇØÇìnÇµÇƒÇ≠ÇæÇ≥Ç¢");
		}

	private:
		World* world_;
		Vector<UniquePtr<BaseSystem>> systems_;
#ifdef _DEBUG
		UnorderedSet<u64> system_ids_;
#endif
	};
}
