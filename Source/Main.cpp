
#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Chunk.h"
#include "World.h"
#include "PerformanceCounter.h"

constexpr float kFactor{ 1.0f };
constexpr int kNumObjects{ 20000 };
constexpr int kTestCounts{ 1000 };
int test_index_ecs{ 0 };
int test_index_com{ 0 };
double performance_counters_ecs[kTestCounts]{ 0 };
double performance_counters_com[kTestCounts]{ 0 };

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
		u32 clock_index{ PerformanceCounter::Begin() };
		Vector<float3> positions;
		Vector<float4x4> world_matrix;
		auto transform_array{ world_->GetComponentArrays<Transform>() };
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
		}

		performance_counters_ecs[test_index_ecs] = PerformanceCounter::End(clock_index);
	}

private:
};

class GameObject;
class BaseComponent
{
public:
	BaseComponent() = default;

	virtual void Update() {};
	SharedPtr<GameObject> owner_;
};

class TransformCom : public BaseComponent
{
public:
	TransformCom(int index)
	{
		position_.x = kFactor * index;
		position_.y = kFactor * index;
		position_.z = kFactor * index;
		scaling_ = float3(1.0f, 1.0f, 1.0f);
		rotation_ = float4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	void Update() override
	{
		const float3 s{ scaling_ };
		const float4 r{ rotation_ };
		const float3 p{ position_ };
		const XMMATRIX S{ XMMatrixScaling(s.x, s.y, s.z) };
		const XMMATRIX R{ XMMatrixRotationRollPitchYaw(r.x, r.y, r.z) };
		const XMMATRIX T{ XMMatrixTranslation(p.x, p.y, p.z) };
		const XMMATRIX W{ S * R * T };
		XMStoreFloat4x4(&world_matrix_, W);
	}
	
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
class GameObject
{
public:
	GameObject(int i)
	{
		components_.emplace_back(std::make_shared<TransformCom>(i));
	}

	void Update()
	{
		for(auto& c : components_)
		{
			c->Update();
		}
	}

	Vector<SharedPtr<BaseComponent>> components_;
};
int main()
{
	Vector<SharedPtr<GameObject>> game_objects_;
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
		if(i % 2 == 0)
		{
			entities.emplace_back(world.AddEntity<Transform>());
		}
		else
		{
			entities.emplace_back(world.AddEntity<Transform, Camera>());
		}
		t.position_.x = kFactor * i;
		t.position_.y = kFactor * i;
		t.position_.z = kFactor * i;
		world.SetComponentData(entities.at(i), t);

		game_objects_.emplace_back(std::make_shared<GameObject>(i));
	}

	world.AddSystems<UpdateTransform>();

	for(int i = 0; i < kTestCounts; ++i)
	{
		world.ExecuteSystems();
		++test_index_ecs;
	}

	//Vector<ComponentArray<Transform>> arrays{ world.GetComponentArrays<Transform>() };

	//Vector<Transform> t_array;
	//Vector<float4x4> world_matrix_array;
	//for(const ComponentArray<Transform>& array : arrays)
	//{
	//	for(const auto t : array)
	//	{
	//		t_array.emplace_back(t);
	//		world_matrix_array.emplace_back(t.world_matrix_);
	//	}
	//}

	
	for(int i = 0; i < kTestCounts; ++i)
	{
		u32 clock_index{ PerformanceCounter::Begin() };
		for(int j = 0; j < kNumObjects; ++j)
		{
			game_objects_.at(j)->Update();
		}
		performance_counters_com[test_index_com] = PerformanceCounter::End(clock_index);
		++test_index_com;
	}

	//world.RemoveEntity(entities.at(0));
	//Entity entity = world.AddEntity<Transform>();

	double sum_ecs{ 0 };
	double sum_com{ 0 };
	double max_ecs{ 0 };
	double max_com{ 0 };
	double min_ecs{ 1000.0 };
	double min_com{ 1000.0 };
	for(int i = 0; i < kTestCounts; ++i)
	{
		if(max_ecs < performance_counters_ecs[i]) max_ecs = performance_counters_ecs[i];
		if(min_ecs > performance_counters_ecs[i]) min_ecs = performance_counters_ecs[i];
		if(max_com < performance_counters_com[i]) max_com = performance_counters_com[i];
		if(min_com > performance_counters_com[i]) min_com = performance_counters_com[i];
		sum_ecs += performance_counters_ecs[i];
		sum_com += performance_counters_com[i];
		std::cout << "ECS : " << performance_counters_ecs[i] << "[ms]" << std::endl;
		std::cout << "COM : " << performance_counters_com[i] << "[ms]" << std::endl;
	}

	std::cout << "Average" << std::endl;
	std::cout << "ECS : " << sum_ecs / kTestCounts << std::endl;
	std::cout << "COM : " << sum_com / kTestCounts << std::endl;
	std::cout << "MAX" << std::endl;
	std::cout << "ECS : " << max_ecs / kTestCounts << std::endl;
	std::cout << "COM : " << max_com / kTestCounts << std::endl;
	std::cout << "MIN" << std::endl;
	std::cout << "ECS : " << min_ecs / kTestCounts << std::endl;
	std::cout << "COM : " << min_com / kTestCounts << std::endl;

	return 0;
}
