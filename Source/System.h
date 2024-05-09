#pragma once

#include "CommonHeader.h"
#include "ECSCommon.h"


namespace ecs
{
	class World;
	class BaseSystem
	{
		friend class SystemManager;
	public:
		BaseSystem() = default;
		virtual ~BaseSystem() = default;

		virtual void Execute() {};

	private:

		void SetWorld(World* world) { world_ = world; }

	protected:
		World* world_;
	};

	class SystemManager
	{
	public:
		SystemManager() = default;

		void Execute()
		{
			for(auto& system : systems_)
			{
				system->Execute();
			}
		}
		template<class ...Systems>
		void AddSystems(World* world)
		{
			_ASSERT_EXPR(world, L"World‚ªnullptr‚Å‚µ‚½");
			AddSystemImpl<Systems...>(world);
		}

		template<class ...Systems>
		void RemoveSystems();

	private:

		template<class Head, class ...Tails>
		void AddSystemImpl(World* world)
		{
			IsBaseOfBaseSystem<Head>();

			const u64 id{ typeid(Head).hash_code() };
			if(!system_ids_.contains(id))
			{
				UniquePtr<Head> system{ std::make_unique<Head>() };
				system->SetWorld(world);
				systems_.emplace_back(std::move(system));
			}

			if constexpr(sizeof...(Tails) != 0) AddSystemImpl<Tails...>(world);
		}

	private:

		template<class System>
		static void IsBaseOfBaseSystem()
		{
			static_assert(std::is_base_of_v<BaseSystem, System>, "BaseSystem‚ğŒp³‚µ‚½ƒNƒ‰ƒX‚¾‚¯‚ğ“n‚µ‚Ä‚­‚¾‚³‚¢");
		}

	private:

		Vector<UniquePtr<BaseSystem>> systems_;
#ifdef _DEBUG
		UnorderedSet<u64> system_ids_;
#endif
	};
}
