# Plugin 1 for PyScript
from MineServer import m;
global M

def init():
   global M
   M = m.getMineServer()
#   M.screen.log("Hai Der!")
#    m.screen.log("Hai Der!")
#    print MineServer.MineServer.get().screen()
#    a = MineServer.MineServer.get().screen()
#    a.log("Test")


version = "0.1"
print "Loaded test.py"
