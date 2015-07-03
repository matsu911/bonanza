/* -*- c-basic-offset: 2 -*- */

#include "proce_tlp.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(TLP)

int CONV cmd_thread( char **lasts )
{
  const char *str = strtok_r( NULL, str_delimiters, lasts );

  if ( str == NULL )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  else if ( ! strcmp( str, "num" ) )
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
      if ( ptr == str || l == LONG_MAX || l < 1 || l > TLP_MAX_THREADS )
        {
          str_error = str_bad_cmdline;
          return -2;
        }

      TlpEnd();

      tlp_max = (int)l;

      if ( game_status & ( flag_thinking | flag_pondering | flag_puzzling ) )
        {
          return tlp_start();
        }
      return 1;
    }

  str_error = str_bad_cmdline;
  return -2;
}

#endif
