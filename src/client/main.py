import sys, os
from ctypes import *
from ctypes.util import find_library

MOVE_PONDER_FAILED = 0xfe000000

flag_mated           = 0b0001
flag_resigned        = 0b0010
flag_drawn           = 0b0100
flag_suspend         = 0b1000
mask_game_end        = 0b1111
flag_quit            = 0b0001 << 4
flag_puzzling        = 0b0010 << 4
flag_pondering       = 0b0100 << 4
flag_thinking        = 0b1000 << 4
flag_problem         = 0b0001 << 8
flag_move_now        = 0b0010 << 8
flag_quit_ponder     = 0b0100 << 8
flag_nostdout        = 0b1000 << 8
flag_search_error    = 0b0001 << 12
flag_nonewlog        = 0b0010 << 12
flag_reverse         = 0b0100 << 12
flag_narrow_book     = 0b1000 << 12
flag_time_extendable = 0b0001 << 16
flag_learning        = 0b0010 << 16
flag_nobeep          = 0b0100 << 16
flag_nostress        = 0b1000 << 16
flag_nopeek          = 0b0001 << 20
flag_noponder        = 0b0010 << 20
flag_noprompt        = 0b0100 << 20
flag_sendpv          = 0b1000 << 20
flag_skip_root_move  = 0b0001 << 24      

bn             = CDLL(find_library('./build/libbonanza'))
c_stdout       = c_void_p.in_dll(bn, '__stdoutp')
game_status    = c_int.in_dll(bn, 'game_status')
str_busy_think = c_char_p.in_dll(bn, 'str_busy_think')
str_game_ended = c_char_p.in_dll(bn, 'str_game_ended')
str_error      = c_char_p.in_dll(bn, 'str_error')
str_cmdline    = (c_char * 512).in_dll(bn, 'str_cmdline')

bn.get_str_error.restype = c_char_p

ptree = bn.tlp_atree_work

def display():
    bn.out_board(ptree, c_stdout, 0, 0)
    return 1

def ping():
    print "pong"
    return 1

def move():
    if game_status.value & mask_game_end:
        str_error.value = str_game_ended.value
        return -2
    if game_status.value & flag_thinking:
	str_error.value = str_busy_think.value
	return -2
    elif game_status.value & (flag_pondering | flag_puzzling):
	game_status.value |= flag_quit_ponder
	return 2

    return 1

def procedure(ptree):
    commands = str_cmdline.value.split()
    if commands[0] == '' or commands[0][0] == '#':
        return 1
    if commands[0] == 'display':
        return display()
    if commands[0] == 'ping':
        return ping()
    if commands[0] == 'move':
        return move()
    print commands
    return 1

def show_error():
    bn.out_error("%s", str_error.value)

def main_child(ptree):
    ponder_move = 0
    iret = bn.ponder(ptree)
    if iret < 0:
        return iret
    elif game_status.value & flag_quit:
        return -3
    elif ponder_move == MOVE_PONDER_FAILED:
        pass
    else:
        bn.tlp_end()
        bn.show_prompt()
    iret = bn.next_cmdline(1)
    if iret < 0:
        return iret
    elif game_status.value & flag_quit:
        return -3
    # iret = bn.procedure(ptree)
    iret = procedure(ptree)
    if iret < 0:
        return iret
    elif game_status.value & flag_quit:
        return -3
    return 1
        
if bn.ini(ptree) < 0:
    show_error()
    sys.exit(os.EX_OK)

while True:
    iret = main_child(ptree)
    if iret == -1:
        show_error()
	bn.shutdown_all()
	break
    elif iret == -2:
        show_error()
	bn.shutdown_all()
        continue
    elif iret == -3:
        break

if bn.fin() < 0:
    show_error()
