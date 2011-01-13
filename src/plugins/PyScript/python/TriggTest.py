# Plugin 1 for PyScript
from MineServer import *;
import math;
global M, lastblock,ltype

def init():
    global M,lastblock, ltype
    M= get_MS()
    M.screen.log("Loaded TriggTest")
    lastblock = M.map.get_block(0,0,0)
    ltype=7


def cb_timer200():
    global lastblock, ltype
    user = M.get_user("trigg");
    if not user==None:
        print user.location.x, user.location.y, user.location.z
        block = M.map.get_block(int(math.floor(user.location.x)),int(math.floor(user.location.y-1)),int(math.floor(user.location.z)));
        b=block;lb=lastblock
        if((not b.x==lb.x) or (not b.y==lb.y) or (not b.z==lb.z)):
            ntype= block.get_type()
            block.set_type(41)
            lastblock.set_type(ltype)
            lastblock=block
            ltype=ntype

    # Called at BEST every 200 milliseconds, might be later if under
    # strain
    pass

def cb_timer1000():
    M.map.set_time(0)
    pass

def cb_timer10000():
    pass

def cb_block_place(name,block):
    return False
    pass



def cb_chat(name, msg):
    print name,msg
    return True

