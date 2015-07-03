/* -*- c-basic-offset: 2 -*- */

#include "proce_dfpn_client.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(DFPN_CLIENT)

int CONV cmd_dfpn_client( tree_t * restrict ptree, char **lasts )
{
  const char *str;
  char *ptr;
  int iret;

  AbortDifficultCommand;

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "127.0.0.1"; }
  strncpy( dfpn_client_str_addr, str, 255 );
  dfpn_client_str_addr[255] = '\0';

  str = strtok_r( NULL, str_delimiters, lasts );
  if ( ! str || ! strcmp( str, "." ) ) { str = "4083"; }
  dfpn_client_port = strtol( str, &ptr, 0 );
  if ( ptr == str || dfpn_client_port == INT_MAX || dfpn_client_port < 0
       || dfpn_client_port > USHRT_MAX )
    {
      str_error = str_bad_cmdline;
      return -2;
    }

  Out( "DFPN Server: %s %d\n", dfpn_client_str_addr, dfpn_client_port );

  iret = ini_game( ptree, &min_posi_no_handicap, flag_history, NULL, NULL );
  if ( iret < 0 ) { return iret; }

  if ( dfpn_client_sckt == SCKT_NULL )
    {
      str_error = "Check network status.";
      return -1;
    }

  return get_elapsed( &time_turn_start );
}

#endif
