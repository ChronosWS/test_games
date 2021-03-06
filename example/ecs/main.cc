#include <cassert>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ecs
{
using Entity = uint64_t;

template <typename T>
std::vector<std::pair<Entity, T>> components_;

template <class Tup, class Func, std::size_t... Is>
constexpr void
StaticForImpl(Tup&& t, Func&& f, std::index_sequence<Is...>)
{
  (f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)), ...);
}

template <class... T, class Func>
constexpr void
StaticFor(std::tuple<T...>& t, Func&& f)
{
  StaticForImpl(t, std::forward<Func>(f),
                std::make_index_sequence<sizeof...(T)>{});
}

template <typename T>
T*
Get(Entity entity)
{
  for (auto& component : components_<T>) {
    if (component.first == entity) {
      return &component.second;
    }
  }
  return nullptr;
}

template <typename T, typename... Args>
void
Assign(Entity entity, Args&&... args)
{
  components_<T>.push_back({entity, T(std::forward<Args>(args)...)});
}

template <typename T>
std::pair<Entity, T>*
GetComponentPointer()
{
  return &components_<T>[0];
}

template <typename... Args>
std::tuple<std::pair<Entity, Args>*...>
Gather()
{
  return std::make_tuple(GetComponentPointer<Args>()...);
}

template <class Tup>
bool
AdvanceFirst(Tup&& tup)
{
  bool all_zero = true;
  bool done = false;
  // TODO: Implement templated FirstOf.
  ecs::StaticFor(tup, [&](auto i, auto*& p) {
    if (done) return;
    if (p->first != 0) {
      all_zero = false;
      done = true;
      ++p;
    }
  });
  return all_zero;
}

template <class Tup>
bool
AllEqual(Tup&& tup)
{
  bool all_equal = true;
  Entity id = 0xFFFFFFFF;
  ecs::StaticFor(tup, [&](auto i, auto*& p) {
    if (id == 0xFFFFFFFF)
      id = p->first;
    else if (id != p->first)
      all_equal = false;
  });
  return id != 0 && all_equal;
}

template <class Tup>
bool
AnyZero(Tup&& tup)
{
  bool any_zero = false;
  // TODO: Implmement templated FirstOf.
  ecs::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first == 0) any_zero = true;
  });
  return any_zero;
}

template <class Tup>
void
AdvanceMin(Tup&& tup)
{
  ecs::Entity min = 0xFFFFFFFF;
  int min_idx = 0;
  ecs::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != 0 && p->first < min) {
      min = p->first;
      min_idx = i;
    }
  });
  ecs::StaticFor(tup, [&](auto i, auto*& p) {
    if (p->first != 0 && min_idx == i) {
      ++p;
    }
  });
}

template <typename F, typename Tuple, size_t... S>
decltype(auto)
ApplyTupleImpl(F&& fn, Tuple&& t, std::index_sequence<S...>)
{
  return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
}

template <typename F, typename Tuple>
decltype(auto)
ApplyFromTuple(F&& fn, Tuple&& t)
{
  std::size_t constexpr tSize =
      std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
  return ApplyTupleImpl(std::forward<F>(fn), std::forward<Tuple>(t),
                        std::make_index_sequence<tSize>());
}

template <typename... Args, typename F>
void
Enumerate(F&& f)
{
  auto tup = Gather<Args...>();
  while (!AnyZero(tup)) {
    if (AllEqual(tup)) {
      ApplyFromTuple(f, tup);
    }
    AdvanceMin(tup);
  }
}

}  // namespace ecs

struct PositionComponent {
  PositionComponent(int x, int y) : x_(x), y_(y){};

  friend std::ostream&
  operator<<(std::ostream& o, const PositionComponent& p)
  {
    return o << "Position(" << p.x_ << "," << p.y_ << ")";
  };

  int x_;
  int y_;
};

struct VelocityComponent {
  VelocityComponent(double dx, double dy) : dx_(dx), dy_(dy){};

  friend std::ostream&
  operator<<(std::ostream& o, const VelocityComponent& v)
  {
    return o << "Velocity(" << v.dx_ << "," << v.dy_ << ")";
  };

  double dx_;
  double dy_;
};

struct AccelerationComponent {
  AccelerationComponent(double ddx, double ddy) : ddx_(ddx), ddy_(ddy){};

  friend std::ostream&
  operator<<(std::ostream& o, const AccelerationComponent& a)
  {
    return o << "Acceleration(" << a.ddx_ << "," << a.ddy_ << ")";
  };

  double ddx_;
  double ddy_;
};

int
main()
{
  // TODO: Assumes entity lists are sorted, note the sorted insertion.
  ecs::Assign<PositionComponent>(1, 2, 3);
  ecs::Assign<VelocityComponent>(2, 3.0f, 34.3f);
  ecs::Assign<PositionComponent>(3, 10, 20);
  ecs::Assign<VelocityComponent>(3, 10.3f, 20.15f);
  ecs::Assign<VelocityComponent>(4, 10.3f, 20.15f);
  ecs::Assign<PositionComponent>(6, 1, 2);
  ecs::Assign<VelocityComponent>(6, 10.3f, 20.15f);
  ecs::Assign<AccelerationComponent>(6, 23.4f, 12.0f);

  // TODO: Current impl needs 0 at end of list to work fix that.
  ecs::Assign<PositionComponent>(0, 0, 0);
  ecs::Assign<VelocityComponent>(0, 0.0f, 0.0f);
  ecs::Assign<AccelerationComponent>(0, 0.0f, 0.0f);

  assert(ecs::Get<VelocityComponent>(1) == nullptr);
  ecs::Get<VelocityComponent>(2)->dx_ = 40.0f;

  auto* position = ecs::Get<PositionComponent>(1);
  std::cout << "1: " << *position << std::endl;

  auto* velocity = ecs::Get<VelocityComponent>(2);
  std::cout << "2: " << *velocity << std::endl;

  auto* position3 = ecs::Get<PositionComponent>(3);
  std::cout << "3: " << *position3 << std::endl;

  auto* velocity3 = ecs::Get<VelocityComponent>(3);
  std::cout << "3: " << *velocity3 << std::endl;

  auto* position6 = ecs::Get<PositionComponent>(6);
  std::cout << "6: " << *position6 << std::endl;

  auto* velocity6 = ecs::Get<VelocityComponent>(6);
  std::cout << "6: " << *velocity6 << std::endl;

  auto* acceleration6 = ecs::Get<AccelerationComponent>(6);
  std::cout << "6: " << *acceleration6 << std::endl;

  // Runs on entities 1, 3 and 6
  ecs::Enumerate<PositionComponent>([](auto& position_component) {
    std::cout << "ENTITY ID" << std::endl;
    std::cout << position_component->first << std::endl;
    std::cout << position_component->second << std::endl;
  });

  // Runs on entity 3 and 6
  ecs::Enumerate<PositionComponent, VelocityComponent>(
      [](auto& position_component, auto& velocity_component) {
        std::cout << "ENTITY IDS: "
                  << " ";
        std::cout << position_component->first << ", ";
        std::cout << velocity_component->first << std::endl;
        std::cout << position_component->second << std::endl;
        std::cout << velocity_component->second << std::endl;
        velocity_component->second.dy_ = 34.34;
      });

  // Runs on entity 6
  ecs::Enumerate<PositionComponent, VelocityComponent, AccelerationComponent>(
      [](auto& position_component, auto& velocity_component,
         auto& acceleration_component) {
        std::cout << "ENTITY IDS: "
                  << " ";
        std::cout << position_component->first << ", ";
        std::cout << velocity_component->first << std::endl;
        std::cout << position_component->second << std::endl;
        std::cout << velocity_component->second << std::endl;
        std::cout << acceleration_component->second << std::endl;
      });

  std::cout << ecs::Get<VelocityComponent>(6)->dy_ << std::endl;

  return 0;
}
