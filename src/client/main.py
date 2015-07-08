import sys, os
from ctypes import *
from ctypes.util import find_library

UINT_MAX   = 2**32 - 1
UINT64_MAX = 2**64 - 1

SIZE_PLAYERNAME = 256
MAX_ANSWER      = 8
PLY_MAX         = 128

MOVE_PONDER_FAILED = 0xfe000000
MOVE_RESIGN        = 0xff000000

black = 0
white = 1

nhand   = 7
nfile   = 9
nrank   = 9
nsquare = 81

promote    = 8
empty      = 0
pawn       = 1
lance      = 2
knight     = 3
silver     = 4
gold       = 5
bishop     = 6
rook       = 7
king       = 8
pro_pawn   = 9
pro_lance  = 10
pro_knight = 11
pro_silver = 12
piece_null = 13
horse      = 14
dragon     = 15

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

flag_time        = 0b0001
flag_history     = 0b0010
flag_rep         = 0b0100
flag_detect_hang = 0b1000
flag_nomake_move = 0b0010 << 4
flag_nofmargin   = 0b0100 << 4

class Record_t(Structure):
    _fields_ = [("info", (c_char * MAX_ANSWER * 8)),
                ("str_name1", (c_char * SIZE_PLAYERNAME)),
                ("str_name2", (c_char * SIZE_PLAYERNAME)),
                ("pf", c_void_p),
                ("games", c_uint),
                ("moves", c_uint),
                ("lines", c_uint)]

class Min_posi_t(Structure):
    _fields_ = [("hand_black", c_uint),
                ("hand_white", c_uint),
                ("turn_to_move", c_char),
                ("asquare", (c_char * nsquare))]

class Trans_entry_t(Structure):
    _fields_ = [("word1", c_uint64),
                ("word2", c_uint64)]

class Trans_table_t(Structure):
    _fields_ = [("prefer", Trans_entry_t),
                ("always", Trans_entry_t * 2)]

bn               = CDLL(find_library('./build/libbonanza'))
c_stdout         = c_void_p.in_dll(bn, '__stdoutp')
game_status      = c_int.in_dll(bn, 'game_status')
str_busy_think   = c_char_p.in_dll(bn, 'str_busy_think')
str_game_ended   = c_char_p.in_dll(bn, 'str_game_ended')
str_error        = c_char_p.in_dll(bn, 'str_error')
str_bad_cmdline  = c_char_p.in_dll(bn, 'str_bad_cmdline')
str_cmdline      = (c_char * 512).in_dll(bn, 'str_cmdline')
time_turn_start  = c_uint.in_dll(bn, 'time_turn_start')
root_turn        = c_int.in_dll(bn, 'root_turn')
record_game      = Record_t.in_dll(bn, 'record_game')
sec_limit        = c_uint.in_dll(bn, 'sec_limit')
sec_limit_up     = c_uint.in_dll(bn, 'sec_limit_up')
sec_limit_depth  = c_uint.in_dll(bn, 'sec_limit_depth')
node_limit       = c_uint64.in_dll(bn, 'node_limit')
node_per_second  = c_uint.in_dll(bn, 'node_per_second')
node_next_signal = c_uint.in_dll(bn, 'node_next_signal')
node_last_check  = c_uint.in_dll(bn, 'node_last_check')
depth_limit      = c_int.in_dll(bn, 'depth_limit')

log2_ntrans_table = c_int.in_dll(bn, 'log2_ntrans_table')
ptrans_table_orig = POINTER(Trans_table_t).in_dll(bn, 'ptrans_table_orig')

min_posi_no_handicap = Min_posi_t.in_dll(bn, 'min_posi_no_handicap')

bn.get_str_error.restype = c_char_p

ptree = bn.tlp_atree_work

class BadCommandline(Exception):
    pass

class BusyThink(Exception):
    pass

def cmd_display(commands):
    bn.out_board(ptree, c_stdout, 0, 0)
    return 1

def cmd_ping(commands):
    print "pong"
    return 1

def cmd_beep(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        game_status.value &= ~flag_nobeep
    elif commands[0] == 'off':
        game_status.value |= flag_nobeep
    else:
        raise BadCommandline
    return 1

def cmd_peek(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        game_status.value &= ~flag_nopeek
    elif commands[0] == 'off':
        game_status.value |=  flag_nopeek
    else:
        raise BadCommandline
    return 1

def cmd_hash(commands):
    if len(commands) == 0:
        raise BadCommandline
    if game_status.value & flag_thinking:
        raise BusyThink
    elif game_status.value & (flag_pondering | flag_puzzling):
	game_status.value |= flag_quit_ponder
	return 2
    log2_ntrans_table.value = int(commands[0])
    bn.memory_free(ptrans_table_orig)
    return bn.ini_trans_table()

def cmd_stdout(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        game_status.value &= ~flag_nostdout
    elif commands[0] == 'off':
        game_status.value |=  flag_nostdout
    else:
        raise BadCommandline
    return 1

def cmd_move(commands):
    if game_status.value & mask_game_end:
        str_error.value = str_game_ended.value
        return -2
    if game_status.value & flag_thinking:
        raise BusyThink
    elif game_status.value & (flag_pondering | flag_puzzling):
	game_status.value |= flag_quit_ponder
	return 2
    if len(commands) == 0:
        iret = bn.get_elapsed(pointer(time_turn_start))
        if iret < 0:
            return iret
        return bn.com_turn_start(ptree, 0)
    try:
        for i in range(0, int(commands[0])):
            if game_status.value & (flag_move_now | mask_game_end):
                break
	    iret = bn.get_elapsed(pointer(time_turn_start))
	    if iret < 0: return iret
	    iret = bn.com_turn_start(ptree, 0)
	    if iret < 0: return iret
        return 1
    except:
        pass
    for command in commands:
        move = c_uint(0)
        iret = bn.interpret_CSA_move(ptree, pointer(move), command)
        if iret < 0: return iret
        iret = bn.get_elapsed(pointer(time_turn_start))
        if iret < 0: return iret
        iret = bn.make_move_root(ptree, move, (flag_history | flag_time | flag_rep | flag_detect_hang))
        if iret < 0: return iret
    return 1

def cmd_new(commands):
    if game_status.value & flag_thinking:
        raise BusyThink
    elif game_status.value & (flag_pondering | flag_puzzling):
	game_status.value |= flag_quit_ponder
	return 2
    if len(commands) == 0:
        pmp = pointer(min_posi_no_handicap)
    else:
        min_posi = Min_posi_t()
        memset(pointer(min_posi.asquare), empty, nsquare)
        min_posi.hand_black = min_posi.hand_white = 0
        iret = bn.read_board_rep1(str1, pointer(min_posi))
        if iret < 0: return iret
        if len(commands) == 1:
            min_posi.turn_to_move = black
        elif len(commands) == 2:
	    if commands[1][0] == '-':
                min_posi.turn_to_move = white
	    elif commands[1][0] == '+':
                min_posi.turn_to_move = black
	    else:
                raise BadCommandline
        pmp = pointer(min_posi)
    iret = bn.ini_game(ptree, pmp, flag_history, None, None)
    if iret < 0: return iret
    return bn.get_elapsed(pointer(time_turn_start))

def cmd_move_now(commands):
    if game_status.value & flag_thinking:
        game_status.value |= flag_move_now
    return 1

def cmd_quit(commands):
    game_status.value |= flag_quit
    return 1

def cmd_suspend(commands):
    if game_status.value & (flag_pondering | flag_puzzling):
        game_status.value |= flag_quit_ponder
        return 2
    game_status.value |= flag_suspend
    return 1

def cmd_resign(commands):
    if len(commands) == 0 or commands[0] == 'T':
        if game_status.value & flag_thinking:
            raise BusyThink
        elif game_status.value & (flag_pondering | flag_puzzling):
            game_status.value |= flag_quit_ponder
            return 2
        if game_status.value & mask_game_end: return 1
        game_status.value |= flag_resigned
        bn.update_time(root_turn)
        bn.out_CSA(ptree, pointer(record_game), MOVE_RESIGN)
    else:
        try:
            resign_threshold.value = int(commands[0])
        except:
            raise BadCommandline
    return 1

def cmd_read(commands):
    if len(commands) == 0:
        raise BadCommandline
    flag    = flag_history | flag_rep | flag_detect_hang
    moves   = UINT_MAX
    if len(commands) > 1:
        if commands[1] == 't':
            flag |= flag_time
        elif commands[1] == 'nil':
            raise BadCommandline
        try:
            moves = int(commands[2]) - 1
        except:
            raise BadCommandline
    iret = bn.read_record(ptree, commands[0], moves, flag)
    if iret < 0: return iret
    iret = bn.get_elapsed(pointer(time_turn_start))
    if iret < 0: return iret
    return 1

def cmd_limit(commands):
    if len(commands) == 0:
        raise BadCommandline
    if game_status.value & flag_thinking:
        raise BusyThink
    elif game_status.value & (flag_pondering | flag_puzzling):
	game_status.value |= flag_quit_ponder
	return 2
    try:
        if commands[0] == "depth":
            sec_limit_up.value = UINT_MAX
            node_limit.value   = UINT64_MAX
            depth_limit.value  = int(commands[1])
        elif commands[0] == "nodes":
            sec_limit_up.value = UINT_MAX
            depth_limit.value  = PLY_MAX
            node_limit.value   = int(commands[1])
        elif commands[0] == "time":
            if len(commands) == 1:
                raise BadCommandline
            if commands[1] == 'extendable':
                game_status.value |= flag_time_extendable
            elif commands[1] == 'strict':
                game_status.value &= ~flag_time_extendable
            else:
                l1 = int(commands[1])
                l2 = int(commands[2])
	        if not (l1 | l2): l2 = 1
	        depth_limit.value  = PLY_MAX
	        node_limit.value   = UINT64_MAX
	        sec_limit.value    = l1 * 6
	        sec_limit_up.value = l2
                try:
                    sec_limit_depth.value = int(commands[3])
                except:
                    sec_limit_depth.value = UINT_MAX
        else:
            raise BadCommandline
    except:
        raise BadCommandline
    return 1

def cmd_ponder(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        game_status.value &= ~flag_noponder
    elif commands[0] == 'off':
        if game_status.value & (flag_pondering | flag_puzzling):
	    game_status.value |= flag_quit_ponder;
        game_status.value |= flag_noponder
    else:
        raise BadCommandline
    return 1

def cmd_newlog(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        game_status.value &= ~flag_nonewlog
    elif commands[0] == 'off':
        game_status.value |=  flag_nonewlog
    else:
        raise BadCommandline
    return 1

def cmd_book(commands):
    if len(commands) == 0:
        raise BadCommandline
    if commands[0] == 'on':
        return bn.book_on()
    elif commands[0] == 'off':
        return bn.book_off()
    elif commands[0] == 'narrow':
        game_status.value |= flag_narrow_book
    elif commands[0] == 'wide':
        game_status.value &= ~flag_narrow_book
    else:
        raise BadCommandline
    return 1

def procedure(ptree):
    commands = str_cmdline.value.split()
    if len(commands) == 0 or commands[0][0] == '#':
        return 1
    if commands[0] == 's':
        return cmd_move_now(commands[1:])
    if commands[0] == 'book':
        return cmd_book(commands[1:])
    if commands[0] == 'display':
        return cmd_display(commands[1:])
    if commands[0] == 'new':
        return cmd_new(commands[1:])
    if commands[0] == 'newlog':
        return cmd_newlog(commands[1:])
    if commands[0] == 'read':
        return cmd_read(commands[1:])
    if commands[0] == 'beep':
        return cmd_beep(commands[1:])
    if commands[0] == 'peek':
        return cmd_peek(commands[1:])
    if commands[0] == 'ponder':
        return cmd_ponder(commands[1:])
    if commands[0] == 'stdout':
        return cmd_stdout(commands[1:])
    if commands[0] == 'hash':
        return cmd_hash(commands[1:])
    if commands[0] == 'limit':
        return cmd_limit(commands[1:])
    if commands[0] == 'ping':
        return cmd_ping(commands[1:])
    if commands[0] == 'move':
        return cmd_move(commands[1:])
    if commands[0] == 'resign':
        return cmd_resign(commands[1:])
    if commands[0] == 'suspend':
        return cmd_suspend(commands[1:])
    if commands[0] == 'quit':
        return cmd_quit(commands[1:])
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
    try:
        # iret = bn.procedure(ptree)
        iret = procedure(ptree)
    except BadCommandline:
        str_error.value = str_bad_cmdline.value
        return -2
    except BusyThink:
        str_error.value = str_busy_think.value
	return -2
    if iret < 0:
        return iret
    elif game_status.value & flag_quit:
        return -3
    return 1

def main():
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

main()
