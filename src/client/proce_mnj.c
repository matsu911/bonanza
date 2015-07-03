/* -*- c-basic-offset: 2 -*- */

#include "proce_mnj.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(MNJ_LAN)

extern int CONV cmd_suspend( void );
extern int CONV cmd_new( tree_t * restrict ptree, char **lasts );

static int CONV cmd_mnjignore( tree_t *restrict ptree, char **lasts );
int CONV cmd_mnj( char **lasts );
static int CONV cmd_mnjmove( tree_t * restrict ptree, char **lasts,
			     int num_alter );

/* mnj sd seed addr port name factor stable_depth */
int CONV cmd_mnj( char **lasts )
{
  char client_str_addr[256];
  char client_str_id[256];
  const char *str;
  char *ptr;
  unsigned int seed;
  long l;
  int client_port, sd;
  double factor;

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  l = strtol( str, &ptr, 0 );
  if ( ptr == str || l == LONG_MAX || l < 0 )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  sd = (int)l;


  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  l = strtol( str, &ptr, 0 );
  if ( ptr == str || l == LONG_MAX || l < 0 )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  seed = (unsigned int)l;


  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "localhost"; }
  strncpy( client_str_addr, str, 255 );
  client_str_addr[255] = '\0';


  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "4082"; }
  l = strtol( str, &ptr, 0 );
  if ( ptr == str || l == LONG_MAX || l < 0 || l > USHRT_MAX )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  client_port = (int)l;


  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "bonanza1"; }
  strncpy( client_str_id, str, 255 );
  client_str_id[255] = '\0';

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "1.0"; }
  factor = strtod( str, &ptr );
  if ( ptr == str || factor < 0.0 )
    {
      str_error = str_bad_cmdline;
      return -2;
    }

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { l = -1; }
  else {
    l = strtol( str, &ptr, 0 );
    if ( ptr == str || l == LONG_MAX )
      {
	str_error = str_bad_cmdline;
	return -2;
      }
  }
  if ( l <= 0 ) { mnj_depth_stable = INT_MAX; }
  else          { mnj_depth_stable = (int)l; }

  AbortDifficultCommand;

  resign_threshold  = 65535;
  game_status      |= ( flag_noponder | flag_noprompt );
  if ( mnj_reset_tbl( sd, seed ) < 0 ) { return -1; }

  sckt_mnj = sckt_connect( client_str_addr, (int)client_port );
  if ( sckt_mnj == SCKT_NULL ) { return -2; }

  str_buffer_cmdline[0] = '\0';

  Out( "Sending my name %s", client_str_id );
  MnjOut( "%s %g final%s\n", client_str_id, factor,
	  ( mnj_depth_stable == INT_MAX ) ? "" : " stable" );

  return cmd_suspend();
}

int CONV proce_mnj( tree_t * restrict ptree )
{
  const char *token;
  char *last;
  int iret;

  token = strtok_r( str_cmdline, str_delimiters, &last );
  if ( token == NULL ) { return 1; }

  if ( ! strcmp( token, "new" ) )
    {
      iret = cmd_suspend();
      if ( iret != 1 ) { return iret; }

      mnj_posi_id = 0;
      iret = cmd_new( ptree, &last );
      if ( iret < 0 ) { return iret; }

      moves_ignore[0] = MOVE_NA;
      return analyze( ptree );
    }
  if ( ! strcmp( token, "ignore" ) ) { return cmd_mnjignore( ptree, &last ); }
  if ( ! strcmp( token, "idle" ) )   { return cmd_suspend(); }
  if ( ! strcmp( token, "alter" ) )  { return cmd_mnjmove( ptree, &last, 1 ); }
  if ( ! strcmp( token, "retract" ) )
    {
      long l;
      char *ptr;
      const char *str = strtok_r( NULL, str_delimiters, &last );
      if ( str == NULL )
        {
          str_error = str_bad_cmdline;
          return -1;
        }
      l = strtol( str, &ptr, 0 );
      if ( ptr == str || (long)NUM_UNMAKE < l )
        {
          str_error = str_bad_cmdline;
          return -1;
        }
      
      return cmd_mnjmove( ptree, &last, (int)l );
    }
  if ( ! strcmp( token, "move" ) )  { return cmd_mnjmove( ptree, &last, 0 ); }

  str_error = str_bad_cmdline;
  return -2;
}


static int CONV
cmd_mnjignore( tree_t *restrict ptree, char **lasts )
{
  const char *token;
  char *ptr;
  int i;
  unsigned int move;
  long lid;


  token = strtok_r( NULL, str_delimiters, lasts );
  if ( token == NULL )
    {
      str_error = str_bad_cmdline;
      return -1;
    }
  lid = strtol( token, &ptr, 0 );
  if ( ptr == token || lid == LONG_MAX || lid < 1 )
    {
      str_error = str_bad_cmdline;
      return -1;
    }

  AbortDifficultCommand;

  for ( i = 0; ; i += 1 )
    {
      token = strtok_r( NULL, str_delimiters, lasts );
      if ( token == NULL ) { break; }

      if ( interpret_CSA_move( ptree, &move, token ) < 0 ) { return -1; }

      moves_ignore[i] = move;
    }
  if ( i == 0 )
    {
      str_error = str_bad_cmdline;
      return -1;
    }
  mnj_posi_id     = (int)lid;
  moves_ignore[i] = MOVE_NA;

  return analyze( ptree );
}


static int CONV
cmd_mnjmove( tree_t * restrict ptree, char **lasts, int num_alter )
{
  const char *str1 = strtok_r( NULL, str_delimiters, lasts );
  const char *str2 = strtok_r( NULL, str_delimiters, lasts );
  char *ptr;
  long lid;
  unsigned int move;
  int iret;

  if ( sckt_mnj == SCKT_NULL ||  str1 == NULL || str2 == NULL )
    {
      str_error = str_bad_cmdline;
      return -1;
    }

  lid = strtol( str2, &ptr, 0 );
  if ( ptr == str2 || lid == LONG_MAX || lid < 1 )
    {
      str_error = str_bad_cmdline;
      return -1;
    }

  AbortDifficultCommand;
 
  while ( num_alter )
    {
      iret = unmake_move_root( ptree );
      if ( iret < 0 ) { return iret; }

      num_alter -= 1;
    }

  iret = interpret_CSA_move( ptree, &move, str1 );
  if ( iret < 0 ) { return iret; }
    
  iret = get_elapsed( &time_turn_start );
  if ( iret < 0 ) { return iret; }

  mnj_posi_id = (int)lid;

  iret = make_move_root( ptree, move, ( flag_time | flag_rep | flag_detect_hang ) );
  if ( iret < 0 ) { return iret; }
  
#  if ! defined(NO_STDOUT)
  iret = out_board( ptree, stdout, 0, 0 );
  if ( iret < 0 ) { return iret; }
#  endif

  moves_ignore[0] = MOVE_NA;
  return analyze( ptree );
}

#endif
