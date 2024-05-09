
#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Chunk.h"
#include "World.h"

struct Transform
{
	float3 position_;
	float3 scaling_;
	float4 rotation_;
};

struct Camera
{
	float3 focus_;
	float near_;
	float far_;
	float fov_angle_;
};

struct DirectionLight
{
	float3 direction_;
	float3 color_;
	float intensity_;
};


class TestSystem : public ecs::BaseSystem
{
public:
	TestSystem() = default;

	void Execute() override
	{
		std::cout << "Test System" << std::endl;
	}
};

int main()
{
	ecs::World world;
	world.AddArchetype<Transform>();
	world.AddArchetype<Transform, Camera>();
	world.AddArchetype<Transform, Camera, DirectionLight>();

	Vector<Entity> entities;

	for(int i = 0; i < 100; ++i)
	{
		entities.emplace_back(world.AddEntity<Transform>());
	}
	Transform t;
	t.position_ = float3(0.1f, 0.1f, 0.1f);
	t.scaling_ = float3(1.0f, 1.0f, 1.0f);
	t.rotation_ = float4(0.1f, 2.0f, 3.14f, 0.0f);

	float factor = 0.1f;
	for(int i = 0; i < 100; ++i)
	{
		t.position_.x = factor * i;
		t.position_.y = factor * i;
		t.position_.z = factor * i;
		world.SetComponentData(entities.at(i), t);
	}

	Vector<ComponentArray<Transform>> arrays{ world.GetComponentArrays<Transform>() };

	Vector<Transform> t_array;
	for(const ComponentArray<Transform>& array : arrays)
	{
		for(const auto t : array)
		{
			t_array.emplace_back(t);
		}
	}

	world.AddSystems<TestSystem>();
	world.ExecuteSystems();
	return 0;
}
