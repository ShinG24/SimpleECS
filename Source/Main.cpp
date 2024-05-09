
#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Chunk.h"
#include "World.h"
#include "PerformanceCounter.h"
#include "System.h"

constexpr float kFactor{ 1.0f };
constexpr int kNumObjects{ 200 };


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


	static void Update(Transform& t)
	{
		std::cout << "Position: x: " << t.position_.x << " y: " << t.position_.y << " z: " << t.position_.z << std::endl;
		const XMMATRIX S{ XMMatrixScaling(t.scaling_.x, t.scaling_.y, t.scaling_.z) };
		const XMMATRIX R{ XMMatrixRotationRollPitchYaw(t.rotation_.x, t.rotation_.y, t.rotation_.z) };
		const XMMATRIX T{ XMMatrixTranslation(t.position_.x, t.position_.y, t.position_.z) };
		const XMMATRIX W{ S * R * T };
		XMStoreFloat4x4(&t.world_matrix_, W);
	}
	
	void Execute() override
	{
		Foreach<Transform>(&Update);
		/*auto transform_array{ world_->GetComponentArrays<Transform>() };
		for(auto array : transform_array)
		{
			for(auto& t : array)
			{
				const XMMATRIX S{ XMMatrixScaling(t.scaling_.x, t.scaling_.y, t.scaling_.z) };
				const XMMATRIX R{ XMMatrixRotationRollPitchYaw(t.rotation_.x, t.rotation_.y, t.rotation_.z) };
				const XMMATRIX T{ XMMatrixTranslation(t.position_.x, t.position_.y, t.position_.z) };
				const XMMATRIX W{ S * R * T };
				XMStoreFloat4x4(&t.world_matrix_, W);
			}
		}*/
	}

private:
};

class UpdateCamera : public ecs::BaseSystem
{
public:
	UpdateCamera() = default;

	inline static int i = 0;
	static void Update(Transform& t, Camera& c)
	{
		c.focus_ = float3(i, i, i);
		++i;
	}
	void Execute() override
	{
		Foreach<Transform, Camera>(&Update);
	}
};
int main()
{
	ecs::World world;
	world.AddArchetype<Transform>();
	world.AddArchetype<Transform, Camera>();
	world.AddArchetype<Transform, Camera, DirectionLight>();

	Vector<Entity> entities;
	Transform t{};
	t.position_ = float3(0.1f, 0.1f, 0.1f);
	t.scaling_ = float3(1.0f, 1.0f, 1.0f);
	t.rotation_ = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for(int i = 0; i < kNumObjects; ++i)
	{
		if(i % 3 == 0)
		{
			entities.emplace_back(world.AddEntity<Transform>());
		}
		else if(i % 3 == 1)
		{
			entities.emplace_back(world.AddEntity<Transform, Camera>());
		}
		else
		{
			entities.emplace_back(world.AddEntity<Transform, Camera, DirectionLight>());
		}
		t.position_.x = kFactor * i;
		t.position_.y = kFactor * i;
		t.position_.z = kFactor * i;
		world.SetComponentData(entities.at(i), t);
	}

	world.GetSystemManager()->AddSystems<UpdateTransform, UpdateCamera>();
	//world.AddSystems<UpdateTransform>();

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

	Vector<ComponentArray<Camera>> c_arrays{ world.GetComponentArrays<Camera>() };
	Vector<float3> focus;
	for(const auto& array : c_arrays)
	{
		for(const auto& c : array)
		{
			focus.emplace_back(c.focus_);
		}
	}

	//world.RemoveEntity(entities.at(0));
	//Entity entity = world.AddEntity<Transform>();

	return 0;
}
