#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace py = pybind11;

using namespace std;

class Vector2 {
 public:

  /** Constructs a default Vector2 */
  Vector2();

  /** Constructs a Vector2 with the given coordinates */
  Vector2(/** The x coordinate of the vector */
          int x,
          /** The y coordinate of the vector */
          int y);

  /** Adds two vectors together */
  void operator+=(const Vector2& other);

  /**
   * Multiplies two vectors together
   * @return The multiplied vector
   */
  Vector2 operator*(const int other);

  /** The x coordinate of the vector */
  int x;
  /** The y coordinate of the vector */
  int y;
};


// Forward declaration
class GameObject;

/**
 * Class representing a component of a GameObject
 */
class Component {
 public:
  /** Sets the given GameObject as this component's parent */
  void SetParentGameObject(/** The GameObject to be assigned as this component's
                              parent */
                           std::weak_ptr<GameObject> g);

  /** Initializes this component */
  virtual void Init();

  /** Updates this component */
  virtual void Update() = 0;

  /**
   * Gets a shared ptr reference to this component's GameObject
   * @return A shared ptr reference to this component's GameObject
   */
  std::shared_ptr<GameObject> GetGameObject();

 protected:
  /** This component's parent GameObject */
  std::weak_ptr<GameObject> gameObject;
};

class ScriptComponent : public Component {
 public:

  /** Construts a new script component */
  ScriptComponent(/** The name of this script component's python module. */
                  const std::string& module,
                  /** The name of this script component's python class. */
                  const std::string& pyClass);

  void Init() override;

  void Update() override;

  /** The name of this script component's python module. */
  const std::string moduleName;
  /** The name of this script component's python class. */
  const std::string className;

 private:
  /** The wrapper object for this script component. */
  pybind11::object wrapper_;
};

class GameObject : public std::enable_shared_from_this<GameObject> {
 public:
  /**
   * Creates a GameObject
   * @return the created GameObject
   */
  static std::shared_ptr<GameObject>
  Create(/** The x coordinate of this GameObject */
         int posX,
         /** The y coordinate of this GameObject */
         int posY,
         /** The width of this GameObject */
         int width,
         /** The height of this GameObject */
         int height,
         /** Whether this GameObject should use direct screen coordinates */
         bool isUI = false,
         /** Whether this GameObject should persist througt level load */
         bool isPermanent = false);

  /**
   * Creates a non-UI, non-permanent GameObject
   * @return the created GameObject
   */
  static std::shared_ptr<GameObject>
  CreateStandard(/** The x coordinate of this GameObject */
                 int posX,
                 /** The y coordinate of this GameObject */
                 int posY,
                 /** The width of this GameObject */
                 int width,
                 /** The height of this GameObject */
                 int height);

  /** Updates this GameObject */
  void Update();

  /** The position of this GameObject */
  Vector2 position;

  /** The scale of this GameObject */
  Vector2 scale;

  /**
   * Disables this GameObject
   */
  void Disable();

  /** Enables this GameObject */
  void Enable();

  /**
   * Gets whether this GameObject is active
   * @return The GameObject's active state
   */
  bool IsActive() const;

  /**
   * Gets whether this GameObject is colliding with another
   * @return The GameObject's collision state
   */
  bool IsColliding(/** The GameObject to check for collisions */
                   const GameObject& other) const;



  /**
   * Gets the first component of type C in the component map or null
   * if none are found.
   * @return The first component found of type C
   */
  template <class C>
  std::shared_ptr<C> GetComponent() {
    static_assert(std::is_base_of<Component, C>::value,"C must derive from Component");

    auto it = componentMap.find(GetTypeID<C>());
    if (it == componentMap.end()) {
      return std::shared_ptr<C>(NULL);
    }
    return std::static_pointer_cast<C>(it->second);
  }

  /**
   * Adds a component of type C to the component map
   * @return A shared pointer reference to this GameObject
   */
  template <class C>
  std::shared_ptr<GameObject> AddComponent(std::shared_ptr<C> comp) {
    static_assert(std::is_base_of<Component, C>::value,"C must derive from Component");

    static_cast<std::shared_ptr<Component>>(comp)->SetParentGameObject(
        std::weak_ptr<GameObject>(shared_from_this()));
    componentMap[GetTypeID<C>()] = comp;
    comp->Init();

    return shared_from_this();
  }

 private:
  /** Constructs a GameObject */
  GameObject(/** The x coordinate of this GameObject */
             int posX,
             /** The y coordinate of this GameObject */
             int posY,
             /** The width of this GameObject */
             int width,
             /** The height of this GameObject */
             int height,
             /** Whether this GameObject should use direct screen coordinates */
             bool isUI);


  std::unordered_map<int, std::shared_ptr<Component>> componentMap;


  bool active;

  bool isUI;


  static int lastTypeID;

  template <class Key>
  inline static int GetTypeID()
  {
    static const int id = lastTypeID++;
    return id;
  }
};

class PyScriptWrapper
{
 public:
  std::shared_ptr<GameObject> GetGameObject();
  void SetScriptComponent(std::shared_ptr<ScriptComponent> sc);
 private:
  std::weak_ptr<ScriptComponent> scriptComp;
};


Vector2::Vector2() : x(0), y(0) {}

Vector2::Vector2(int x, int y) : x(x), y(y) {}

void Vector2::operator+=(const Vector2& other) {
  x += other.x;
  y += other.y;
}

Vector2 Vector2::operator*(const int other) {
  return Vector2(x * other, y * other);
}

void Component::SetParentGameObject(std::weak_ptr<GameObject> g) {
  gameObject = g;
}

void Component::Init() {}

std::shared_ptr<GameObject> Component::GetGameObject() {
  return gameObject.lock();
}

int GameObject::lastTypeID = 0;

GameObject::GameObject(int posX, int posY, int width, int height, bool isUI)
    : position(posX, posY), scale(width, height), active(true), isUI(isUI) {}

std::shared_ptr<GameObject> GameObject::Create(int posX, int posY, int width,
                                               int height, bool isUI,
                                               bool isPermanent) {
  std::shared_ptr<GameObject> ga = std::shared_ptr<GameObject>(
      new GameObject(posX, posY, width, height, isUI));
//  GameManager::instance().RegisterObject(ga, isPermanent);
  return ga;
}
std::shared_ptr<GameObject> GameObject::CreateStandard(int posX, int posY,
                                                       int width, int height) {
  return Create(posX, posY, width, height, false, false);
}

void GameObject::Update() {
  for (auto it = componentMap.begin(); it != componentMap.end(); ++it) {
    it->second->Update();
  }
}

void GameObject::Disable() { active = false; }

void GameObject::Enable() { active = true; }

bool GameObject::IsActive() const { return active; }

bool GameObject::IsColliding(const GameObject& other) const {
  Vector2 oPos = other.position;
  Vector2 oScale = other.scale;
  return position.x - scale.x / 2 <= oPos.x + oScale.x / 2 &&
         position.x + scale.x / 2 >= oPos.x - oScale.x / 2 &&
         position.y + scale.y / 2 >= oPos.y - oScale.y / 2 &&
         position.y - scale.y / 2 <= oPos.y + oScale.y / 2;
}

std::shared_ptr<GameObject> PyScriptWrapper::GetGameObject()
{
  return scriptComp.lock()->GetGameObject();
}

void PyScriptWrapper::SetScriptComponent(std::shared_ptr<ScriptComponent> sc)
{
  scriptComp = std::weak_ptr<ScriptComponent>(sc);
}

ScriptComponent::ScriptComponent(const std::string& module,
                                 const std::string& pyClass)
    // save for serialization later
    : className(pyClass), moduleName(module) {}

void ScriptComponent::Init() {
  // Get module file
  pybind11::module script = pybind11::module::import(moduleName.c_str());
  // Get specific class and call constructor
  pybind11::object sc = script.attr(pybind11::cast(className))();
  // Set up the wrapper
  pybind11::cast<std::shared_ptr<PyScriptWrapper>>(sc)->SetScriptComponent(
      gameObject.lock()->GetComponent<ScriptComponent>());
  wrapper_ = sc;

  // Call the python init
  wrapper_.attr("Init")();
}

void ScriptComponent::Update() { wrapper_.attr("Update")(); }

int add(int i, int j) {
    return i + j;
}


class Pet
{
    public:
        Pet(const std::string &name, int hunger) : name(name), hunger(hunger)
         {
           pybind11::module script = pybind11::module::import("scripts.main");
           sc = script.attr(pybind11::cast("Cao"));
           func = sc.attr("OnCall");
         }

        ~Pet() {}

        void go_for_a_walk() { hunger++; }
        const std::string &get_name() const { return name; }
        int get_hunger() const { return hunger; }

        void call()
        {
                cout << "[CPP] call" << name << endl;
                func(sc);
        }

        virtual void OnCall()
        {
            cout << "[CPP] Oncall" << name << endl;
        }


        std::string name;
        int hunger;
        pybind11::object func;
        pybind11::object sc;
};
/*
PYBIND11_EMBEDDED_MODULE("engine", m)
{

  pybind11::class_<Vector2>(m, "Vector2")
      .def(pybind11::init<>())
      .def(pybind11::init<int, int>())
      .def_readwrite("x", &Vector2::x)
      .def_readwrite("y", &Vector2::y);

  pybind11::class_<GameObject, std::shared_ptr<GameObject>>(m, "GameObject")
      .def("Create", &GameObject::Create)
      .def("Create", &GameObject::CreateStandard)
      .def_readwrite("position", &GameObject::position)
      .def_readwrite("scale", &GameObject::scale);


}
*/


PYBIND11_EMBEDDED_MODULE(engine, m) {
    // optional module docstring
    m.doc() = "pybind11 example plugin";

    // define add function
    m.def("add", &add, "A function which adds two numbers");

    // bindings to Pet class
    py::class_<Pet>(m, "Pet")
        .def(py::init<const std::string &, int>())
        .def("go_for_a_walk", &Pet::go_for_a_walk)
        .def("get_hunger", &Pet::get_hunger)
        .def("get_name", &Pet::get_name)
        .def("call", &Pet::call)
        .def_readwrite("name", &Pet::name)
        .def_readwrite("hunger", &Pet::hunger);

 py::class_<Vector2>(m, "Vector2")
      .def(pybind11::init<>())
      .def(pybind11::init<int, int>())
      .def_readwrite("x", &Vector2::x)
      .def_readwrite("y", &Vector2::y);

  pybind11::class_<GameObject, std::shared_ptr<GameObject>>(m, "GameObject")
      .def("Create", &GameObject::Create)
      .def("Create", &GameObject::CreateStandard)
      .def_readwrite("position", &GameObject::position)
      .def_readwrite("scale", &GameObject::scale);

  pybind11::class_<PyScriptWrapper, std::shared_ptr<PyScriptWrapper>>(m, "ScriptBase")
      .def(pybind11::init<>())
      .def("GetGameObject", &PyScriptWrapper::GetGameObject,pybind11::return_value_policy::reference);
}



int main()
{
    cout << "Hello world!" << endl;
    py::scoped_interpreter gurad{};
    // py::initialize_interpreter();
     //py::module::import("sys").attr("path").attr("append")("scripts");

     try
     {

     py::object mainScript = py::module::import("scripts.main");

     std::shared_ptr<GameObject> go = GameObject::Create(10, 10, 5, 5, false, true);
     go->AddComponent<ScriptComponent>(std::make_shared<ScriptComponent>(std::string("scripts.main"), std::string("GamePlayer")));





     } catch(py::error_already_set& e)
     {
        std::cout << e.what() << std::endl;
     }


     //py::finalize_interpreter();

    return 0;
}
