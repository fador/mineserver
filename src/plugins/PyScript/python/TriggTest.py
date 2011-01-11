# Plugin 1 for PyScript
import MineServer;
global M

def init():
   global M
   print MineServer
   print MineServer.getMS
   M = MineServer.getMS()
   print M
   M.screen.log("IT WURKS!?")
#   M.screen.log("Hai Der!")
#    m.screen.log("Hai Der!")
#    print MineServer.MineServer.get().screen()
#    a = MineServer.MineServer.get().screen()
#    a.log("Test")


version = "0.1"
print "Loaded test.py"
