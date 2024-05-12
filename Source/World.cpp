#include "World.h"
#include "System.h"


namespace ecs
{
	World::World()
	{
		system_manager_ = std::make_unique<SystemManager>(this);
	}

	void World::ExecuteSystems()
	{
		system_manager_->Execute();
	}

}
 