/* -*- c-basic-offset: 2 -*- */

#include "proce_csa.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(CSA_LAN)

extern int CONV cmd_resign( tree_t * restrict ptree, char **lasts );
extern int CONV cmd_quit( void );
extern int CONV cmd_usrmove( tree_t * restrict ptree, const char *str_move,
                             char **last );

int CONV proce_csalan( tree_t * restrict ptree )
{
  const char *token;
  char *last;

  token = strtok_r( str_cmdline, str_delimiters, &last );
    
  if ( token == NULL ) { return 1; }
  if ( *token == ach_turn[client_turn] && is_move( token+1 ) )
    {
      char *ptr;
      long l;

      token = strtok_r( NULL, str_delimiters, &last );
      if ( token == NULL || *token != 'T' )
	{
	  str_error = str_bad_cmdline;
	  return -1;
	}
      
      l = strtol( token+1, &ptr, 0 );
      if ( token+1 == ptr || l == LONG_MAX || l < 1 )
	{
	  str_error = str_bad_cmdline;
	  return -1;
	}

      adjust_time( (unsigned int)l, client_turn );
      Out( "  elapsed: b%u, w%u\n", sec_b_total, sec_w_total );
      return 1;
    }
  if ( *token == ach_turn[Flip(client_turn)] && is_move( token+1 ) )
    {
      return cmd_usrmove( ptree, token+1, &last );
    }
  if ( ! strcmp( token, str_resign ) ) { return cmd_resign( ptree, &last ); }
  if ( ! strcmp( token, "#WIN" )
       || ! strcmp( token, "#LOSE" )
       || ! strcmp( token, "#DRAW" )
       || ! strcmp( token, "#CHUDAN" ) )
    {
      if ( game_status & ( flag_thinking | flag_pondering | flag_puzzling ) )
	{
	  game_status |= flag_suspend;
	  return 2;
	}

      if ( sckt_out( sckt_csa, "LOGOUT\n" ) < 0 ) { return -1; }
      if ( sckt_recv_all( sckt_csa )        < 0 ) { return -1; }

      ShutdownAll();
      
      if ( client_ngame == client_max_game ) { return cmd_quit(); }

      return client_next_game( ptree, client_str_addr, (int)client_port );
    }
  
  return 1;
}

int CONV cmd_connect( tree_t * restrict ptree, char **lasts )
{
  const char *str;
  char *ptr;
  long max_games;

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "gserver.computer-shogi.org"; }
  strncpy( client_str_addr, str, 255 );
  client_str_addr[255] = '\0';

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "4081"; }
  client_port = strtol( str, &ptr, 0 );
  if ( ptr == str || client_port == LONG_MAX || client_port < 0
       || client_port > USHRT_MAX )
    {
      str_error = str_bad_cmdline;
      return -2;
    }

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "bonanza_test"; }
  strncpy( client_str_id, str, 255 );
  client_str_id[255] = '\0';

  str = strtok_r( NULL, " \t", lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "bonanza_test"; }
  strncpy( client_str_pwd, str, 255 );
  client_str_pwd[255] = '\0';

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { client_max_game = INT_MAX; }
  else {
    max_games = strtol( str, &ptr, 0 );
    if ( ptr == str || max_games == LONG_MAX || max_games < 1 )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
    client_max_game = max_games;
  }

  AbortDifficultCommand;

  client_ngame          = 0;

  return client_next_game( ptree, client_str_addr, (int)client_port );
}

int CONV cmd_sendpv( char **lasts )
{
  const char *str = strtok_r( NULL, str_delimiters, lasts );

  if ( str == NULL )
    {
      str_error = str_bad_cmdline;
      return -2;
    }

  if      ( ! strcmp( str, str_off ) ) {  game_status &= ~flag_sendpv; }
  else if ( ! strcmp( str, str_on ) )  {  game_status |=  flag_sendpv; }
  else {
    str_error = str_bad_cmdline;
    return -2;
  }

  return 1;
}

#endif
