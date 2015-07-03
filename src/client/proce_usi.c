#include "proce_usi.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(USI)

extern int CONV cmd_move_now( void );
extern int CONV cmd_quit( void );

static int CONV usi_posi( tree_t * restrict ptree, char **lasts );
static int CONV usi_go( tree_t * restrict ptree, char **lasts );
static int CONV usi_ignore( tree_t * restrict ptree, char **lasts );

int CONV proce_usi( tree_t * restrict ptree )
{
  const char *token;
  char *lasts;
  int iret;

  token = strtok_r( str_cmdline, str_delimiters, &lasts );
  if ( token == NULL ) { return 1; }

  if ( ! strcmp( token, "usi" ) )
    {
      USIOut( "id name %s\n", str_myname );
      USIOut( "id author Kunihito Hoki\n" );
      USIOut( "usiok\n" );
      return 1;
    }

  if ( ! strcmp( token, "isready" ) )
    {
      USIOut( "readyok\n", str_myname );
      return 1;
    }

  if ( ! strcmp( token, "echo" ) )
    {
      USIOut( "%s\n", lasts );
      return 1;
    }

  if ( ! strcmp( token, "ignore_moves" ) )
    {
      return usi_ignore( ptree, &lasts );
    }

  if ( ! strcmp( token, "genmove_probability" ) )
    {
      if ( get_elapsed( &time_start ) < 0 ) { return -1; }
      return usi_root_list( ptree );
    }

  if ( ! strcmp( token, "go" ) )
    {
      iret = usi_go( ptree, &lasts );
      moves_ignore[0] = MOVE_NA;
      return iret;
    }

  if ( ! strcmp( token, "stop" ) )     { return cmd_move_now(); }
  if ( ! strcmp( token, "position" ) ) { return usi_posi( ptree, &lasts ); }
  if ( ! strcmp( token, "quit" ) )     { return cmd_quit(); }
  
  str_error = str_bad_cmdline;
  return -1;
}


static int CONV
usi_ignore( tree_t * restrict ptree, char **lasts )
{
  const char *token;
  char str_buf[7];
  int i;
  unsigned int move;

  AbortDifficultCommand;

  for ( i = 0; ; i += 1 )
    {
      token = strtok_r( NULL, str_delimiters, lasts );
      if ( token == NULL ) { break; }
      
      if ( usi2csa( ptree, token, str_buf ) < 0 )            { return -1; }
      if ( interpret_CSA_move( ptree, &move, str_buf ) < 0 ) { return -1; }

      moves_ignore[i] = move;
    }

  moves_ignore[i] = MOVE_NA;

  return 1;
}


static int CONV
usi_go( tree_t * restrict ptree, char **lasts )
{
  const char *token;
  char *ptr;
  int iret;
  long l;

  AbortDifficultCommand;

  if ( game_status & mask_game_end )
    {
      str_error = str_game_ended;
      return -1;
    }
  
  token = strtok_r( NULL, str_delimiters, lasts );

  if ( ! strcmp( token, "book" ) )
    {
      AbortDifficultCommand;
      if ( usi_book( ptree ) < 0 ) { return -1; }

      return 1;
    }


  if ( ! strcmp( token, "infinite" ) )
    {
      usi_byoyomi     = UINT_MAX;
      depth_limit     = PLY_MAX;
      node_limit      = UINT64_MAX;
      sec_limit_depth = UINT_MAX;
    }
  else if ( ! strcmp( token, "byoyomi" ) )
    {
      token = strtok_r( NULL, str_delimiters, lasts );
      if ( token == NULL )
	{
	  str_error = str_bad_cmdline;
	  return -1;
	}

      l = strtol( token, &ptr, 0 );
      if ( ptr == token || l > UINT_MAX || l < 1 )
	{
	  str_error = str_bad_cmdline;
	  return -1;
	}
      
      usi_byoyomi     = (unsigned int)l;
      depth_limit     = PLY_MAX;
      node_limit      = UINT64_MAX;
      sec_limit_depth = UINT_MAX;
    }
  else {
    str_error = str_bad_cmdline;
    return -1;
  }

      
  if ( get_elapsed( &time_turn_start ) < 0 ) { return -1; }

  iret = com_turn_start( ptree, 0 );
  if ( iret < 0 ) {
    if ( str_error == str_no_legal_move ) { USIOut( "bestmove resign\n" ); }
    else                                  { return -1; }
  }
  
  return 1;
}


static int CONV
usi_posi( tree_t * restrict ptree, char **lasts )
{
  const char *token;
  char str_buf[7];
  unsigned int move;
    
  AbortDifficultCommand;
    
  moves_ignore[0] = MOVE_NA;

  token = strtok_r( NULL, str_delimiters, lasts );
  if ( strcmp( token, "startpos" ) )
    {
      str_error = str_bad_cmdline;
      return -1;
    }
    
  if ( ini_game( ptree, &min_posi_no_handicap,
		 flag_history, NULL, NULL ) < 0 ) { return -1; }
    
  token = strtok_r( NULL, str_delimiters, lasts );
  if ( token == NULL ) { return 1; }

  if ( strcmp( token, "moves" ) )
    {
      str_error = str_bad_cmdline;
      return -1;
    }
    
  for ( ;; )  {

    token = strtok_r( NULL, str_delimiters, lasts );
    if ( token == NULL ) { break; }
      
    if ( usi2csa( ptree, token, str_buf ) < 0 )            { return -1; }
    if ( interpret_CSA_move( ptree, &move, str_buf ) < 0 ) { return -1; }
    if ( make_move_root( ptree, move, ( flag_history | flag_time
					| flag_rep
					| flag_detect_hang ) ) < 0 )
      {
	return -1;
      }
  }
    
  if ( get_elapsed( &time_turn_start ) < 0 ) { return -1; }
  return 1;
}

#endif
