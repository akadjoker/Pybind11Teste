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
  Vector2 operator*(const int other);

  /** The x coordinate of the vector */
  int x;
  /** The y coordinate of the vector */
  int y;
};


// Forward declaration
class GameObject;


class Component {
 public:
  void SetParentGameObject(GameObject *g);
  virtual void Init();
  virtual void Update() = 0;
  GameObject *GetGameObject();
 protected:
  GameObject* gameObject;
};

class ScriptComponent : public Component {
 public:
  ScriptComponent(const std::string& module,const std::string& pyClass);
  void Init() override;
  void Update() override;
  const std::string moduleName;
  const std::string className;
 private:
  pybind11::object obj;
};

class GameObject{
 public:

  Vector2 position;
  Vector2 scale;

  GameObject(float x, float y, float w, float h)
  {
  position.x=x;
  position.y=y;
  scale.x=w;
  scale.y=h;
  };
  void Update();
  void Disable();
  void Enable();
  bool IsActive() const;
  void AddComponent(Component *comp)
  {
    comp->SetParentGameObject(this);
    cmpts.push_back(comp);
    comp->Init();
  }
 bool active;
 std::vector<Component*> cmpts;

};

class GameScript
{
 public:
  GameObject* GetGameObject();
  void SetScriptComponent(ScriptComponent* sc);
 private:
  ScriptComponent* scriptComp;
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

void Component::SetParentGameObject(GameObject* g) {
  gameObject = g;
}

void Component::Init() {}

GameObject* Component::GetGameObject()
{
  return gameObject;
}



void GameObject::Update()
{

    for(int i=0;i<cmpts.size();i++)
    {
        Component *cmp = cmpts[i];
        cmp->Update();
    }
}

void GameObject::Disable() { active = false; }

void GameObject::Enable() { active = true; }

bool GameObject::IsActive() const { return active; }



GameObject* GameScript::GetGameObject()
{
  return scriptComp->GetGameObject();
}

void GameScript::SetScriptComponent(ScriptComponent* sc)
{
  scriptComp = sc;
}

ScriptComponent::ScriptComponent(const std::string& module,const std::string& pyClass): className(pyClass), moduleName(module) {}

void ScriptComponent::Init()
{
  pybind11::module script = pybind11::module::import(moduleName.c_str());
  pybind11::object sc = script.attr(pybind11::cast(className))();
  GameScript *scriptCmp = pybind11::cast<GameScript*>(sc);
  scriptCmp->SetScriptComponent(this);
  obj = sc;
  obj.attr("Init")();
}
void ScriptComponent::Update() { obj.attr("Update")(); }


class ScriptNode;

class Node
{
    public:

          Node(const std::string &name) : name(name)
         {

         InitScript();

         }

         void InitScript();

        ~Node() {}
        void Init()
        {
                cout << "[CPP] Init" << name << endl;
                obj.attr("Init")();

        }
        void Update()
        {
                cout << "[CPP] Update" << name << endl;
                obj.attr("Update")();

        }

        std::string name;
        pybind11::object obj;

};

class ScriptNode
{
 public:
  Node* GetNode()
  {
    return scriptNode;
  }
  void SetNode(Node* sc)
  {
    scriptNode = sc;
  }
 private:
   Node* scriptNode;
   pybind11::object obj;
};

void Node::InitScript()
{
  pybind11::module script = pybind11::module::import("scripts.main");
  pybind11::object sc = script.attr(pybind11::cast("PlayerNode"))();
  ScriptNode *scriptCmp = pybind11::cast<ScriptNode*>(sc);
  scriptCmp->SetNode(this);
  obj = sc;

}

class Sprite
{
 public:
  Sprite()
  {
  }

 void loadScript()
 {

  pybind11::module script = pybind11::module::import("scripts.main");
  pybind11::object sc = script.attr(pybind11::cast("Player"))();
  obj = sc;

 }

        void Init()
        {
                cout << "[CPP] Init" << name << endl;
                obj.attr("Init")();

        }
        void Update()
        {
                cout << "[CPP] Update" << name << endl;
                obj.attr("Update")();

        }

        std::string name;
          Vector2 position;
  Vector2 scale;
 private:

   pybind11::object obj;
};




PYBIND11_EMBEDDED_MODULE(engine, m) {
    // optional module docstring
    m.doc() = "pybind11 example plugin";

         py::class_<Sprite, std::unique_ptr<Sprite,py::nodelete>>(m, "Sprite")
        .def(py::init<>())
        .def_readwrite("name", &Sprite::name,pybind11::return_value_policy::reference)
        .def_readwrite("position", &Sprite::position)
        .def_readwrite("scale", &Sprite::scale);

      py::class_<Vector2>(m, "Vector2")
      .def(pybind11::init<>())
      .def(pybind11::init<int, int>())
      .def_readwrite("x", &Vector2::x)
      .def_readwrite("y", &Vector2::y);

    // define add function
//    m.def("add", &add, "A function which adds two numbers");
/*
    py::class_<Node, std::unique_ptr<Node,py::nodelete>>(m, "Node")
        .def(py::init<const std::string &>())
        .def_readwrite("name", &Node::name,pybind11::return_value_policy::reference);


        py::class_<Sprite, std::unique_ptr<Sprite,py::nodelete>>(m, "Sprite")
        .def(py::init<>())
        .def_readwrite("name", &Sprite::name,pybind11::return_value_policy::reference);


py::class_<ScriptNode, std::unique_ptr<ScriptNode,py::nodelete>>(m, "ScriptNode")
        .def(py::init<>())
        .def("GetNode", &ScriptNode::GetNode,pybind11::return_value_policy::reference);



  pybind11::class_<GameObject, std::unique_ptr<GameObject,py::nodelete>>(m, "GameObject")
      .def(pybind11::init<float, float,float, float>())
      .def_readwrite("position", &GameObject::position)
      .def_readwrite("scale", &GameObject::scale);

  pybind11::class_<GameScript,  std::unique_ptr<GameScript,py::nodelete>>(m, "GameScript")
      .def(pybind11::init<>(),py::return_value_policy::reference)
      .def("GetGameObject", &GameScript::GetGameObject,pybind11::return_value_policy::reference);
      */
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
/*
     GameObject* go = new GameObject(10, 10, 5, 5);
     //go->AddComponent(new ScriptComponent(std::string("scripts.main"), std::string("GamePlayer")));


     go->Update();


     Node * d= new Node("gato");

     d->Init();
     d->Update();

*/

     Sprite * spr = new Sprite();
     spr->loadScript();
     spr->Init();
     spr->Update();





     } catch(py::error_already_set& e)
     {
        std::cout << e.what() << std::endl;
     }


     //py::finalize_interpreter();

    return 0;
}
