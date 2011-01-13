# Plugin 1 for PyScript
from MineServer import *;
global M

def init(MS):
    global M
    M = MS
    print M

def cb_timer200():
    print "200"
    print magical
    print magical()
    set_time(M,0)

def cb_chat(name, msg):
    print name,msg


version = "0.1"
print "Loaded test.py"
