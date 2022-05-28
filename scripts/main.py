import engine
print("hello from script")


'''
class PlayerNode(engine.ScriptNode):
    def Init(self):
        print("[PYTHON] init node :",self)
        print("[PYTHON] node is :",self.GetNode())

    def Update(self):
        print("[PYTHON] update node ")




class GameSprite(engine.Sprite):
    def Init(self):
        print("[PYTHON] init sprite :",self)

    def Update(self):
        print("[PYTHON] update sprite ")





class GamePlayer(engine.GameScript):
    def Init(self):
        print("[PYTHON] init",self)
        print(self.GetGameObject())
        print(self.GetGameObject().position)

    def Update(self):
        print("[PYTHON] update")
        #print("[PYTHON] update",self.GetGameObject())


'''


class Player(engine.Sprite):
    def Init(self):
        print("[PYTHON] init sprite :",self)
        print(self.position)

    def Update(self):
        print("[PYTHON] update sprite ")
        print(self.position)
