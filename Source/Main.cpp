
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

	float4x4 world_matrix_
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
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

class UpdateTransform : public ecs::BaseSystem
{
public:
	UpdateTransform() = default;

	void Execute() override
	{
		Vector<float3> positions;
		Vector<float4x4> world_matrix;
		auto transform_array{ world_->GetComponentArrays<Transform>() };
		for(auto array : transform_array)
		{
			for(auto& t : array)
			{
				const float3 s{ t.scaling_ };
				const float4 r{ t.rotation_ };
				const float3 p{ t.position_ };
				const XMMATRIX S{ XMMatrixScaling(s.x, s.y, s.z) };
				const XMMATRIX R{ XMMatrixRotationRollPitchYaw(r.x, r.y, r.z) };
				const XMMATRIX T{ XMMatrixTranslation(p.x, p.y, p.z) };
				const XMMATRIX W{ S * R * T };
				XMStoreFloat4x4(&t.world_matrix_, W);
			}
		}
	}

private:
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
	t.rotation_ = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float factor = 0.1f;
	for(int i = 0; i < 100; ++i)
	{
		t.position_.x = factor * i;
		t.position_.y = factor * i;
		t.position_.z = factor * i;
		world.SetComponentData(entities.at(i), t);
	}


	world.AddSystems<TestSystem, UpdateTransform>();
	
	world.ExecuteSystems();

	Vector<ComponentArray<Transform>> arrays{ world.GetComponentArrays<Transform>() };

	Vector<Transform> t_array;
	Vector<float4x4> world_matrix_array;
	for(const ComponentArray<Transform>& array : arrays)
	{
		for(const auto t : array)
		{
			t_array.emplace_back(t);
			world_matrix_array.emplace_back(t.world_matrix_);
		}
	}

	world.RemoveEntity(entities.at(0));
	Entity entity = world.AddEntity<Transform>();
	return 0;
}
