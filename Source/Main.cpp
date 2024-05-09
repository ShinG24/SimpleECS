
#include "CommonHeader.h"
#include "ECSCommon.h"
#include "Archetype.h"
#include "Chunk.h"
#include "World.h"

std::vector<int> data_size;
std::vector<std::string> str;

// typeid().nameを用いて型名をstringにする
template<typename T>
constexpr void AddStr()
{
	str.emplace_back(typeid(T).name());
}

// テンプレート 可変長引数実験
// Head 先頭の型名 Tails 引数パック
template<typename Head, typename ...Tails>
constexpr void B()
{
	// パッキングされている引数の数を取得
	const auto size = sizeof...(Tails);

	AddStr<Head>();
	data_size.emplace_back(size);

	// パッキングされている引数がある場合は再帰呼び出し
	// この形で渡すと一つずつ引数が減っていく
	// char, short, int, int64		この4つをテンプレート引数として入れたとすると
	// 1周目 B<Head = char,		...Tails = (short, int, int64)
	// 2周目 B<Head = short,	...Tails = (int, int64)
	// 3周目 B<Head = int,		...Tails = (int64)
	// 4周目 B<Head = int64,	...Tails = ()
	// みたいな感じで先頭にある引数がHeadに、それ以降の引数は...Tailsにパッキングされた状態で格納される
	if constexpr ( size != 0 )
		B<Tails...>();
}

template<typename ...Args>
//template<typename ...Args>
void A()
{
	B<Args...>();
}

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

	auto chunks = world.GetChunks<Transform>();
	auto it = chunks.begin();
	auto array = (*it)->GetComponentArray<Transform>();

	Vector<Transform> t_array;
	for(int i =0; i < array.size(); ++i)
	{
		t_array.emplace_back(array[i]);
	}
	return 0;
}
