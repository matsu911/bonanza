/* -*- c-basic-offset: 2 -*- */

#include "proce_dfpn.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(DFPN)
int CONV cmd_dfpn( tree_t * restrict ptree, char **lasts )
{
  const char *str = strtok_r( NULL, str_delimiters, lasts );

  if ( str == NULL )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  else if ( ! strcmp( str, "hash" ) )
    {
      char *ptr;
      long l;

      str = strtok_r( NULL, str_delimiters, lasts );
      if ( str == NULL )
	{
	  str_error = str_bad_cmdline;
	  return -2;
	}
      l = strtol( str, &ptr, 0 );
      if ( ptr == str || l == LONG_MAX || l < 1 )
	{
	  str_error = str_bad_cmdline;
	  return -2;
	}

      AbortDifficultCommand;

      dfpn_hash_log2 = (unsigned int)l;
      return dfpn_ini_hash();
    }
  else if ( ! strcmp( str, "go" ) )
    {
      AbortDifficultCommand;

      return dfpn( ptree, root_turn, 1 );
    }
  else if ( ! strcmp( str, "connect" ) )
    {
      char str_addr[256];
      char str_id[256];
      char *ptr;
      long l;
      int port;

      str = strtok_r( NULL, str_delimiters, lasts );
      if ( ! str || ! strcmp( str, "." ) ) { str = "127.0.0.1"; }
      strncpy( str_addr, str, 255 );
      str_addr[255] = '\0';

      str = strtok_r( NULL, str_delimiters, lasts );
      if ( ! str || ! strcmp( str, "." ) ) { str = "4083"; }
      l = strtol( str, &ptr, 0 );
      if ( ptr == str || l == LONG_MAX || l < 0 || l > USHRT_MAX )
	{
	  str_error = str_bad_cmdline;
	  return -2;
	}
      port = (int)l;

      str = strtok_r( NULL, str_delimiters, lasts );
      if ( ! str || ! strcmp( str, "." ) ) { str = "bonanza1"; }
      strncpy( str_id, str, 255 );
      str_id[255] = '\0';

      AbortDifficultCommand;
      
      dfpn_sckt = sckt_connect( str_addr, port );
      if ( dfpn_sckt == SCKT_NULL ) { return -2; }

      str_buffer_cmdline[0] = '\0';
      DFPNOut( "Worker: %s\n", str_id );

      return 1;
    }

  str_error = str_bad_cmdline;
  return -2;
}
#endif
