#ifndef ABORT_DIFFICULT_COMMAND_H
#define ABORT_DIFFICULT_COMMAND_H

/* unacceptable when the program is thinking, or quit pondering */
#define AbortDifficultCommand                                              \
	  if ( game_status & flag_thinking )                               \
	    {                                                              \
	      str_error = str_busy_think;                                  \
	      return -2;                                                   \
	    }                                                              \
	  else if ( game_status & ( flag_pondering | flag_puzzling ) )     \
	    {                                                              \
	      game_status |= flag_quit_ponder;                             \
	      return 2;                                                    \
	    }

#endif
