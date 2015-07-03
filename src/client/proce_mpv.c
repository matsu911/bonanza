/* -*- c-basic-offset: 2 -*- */

#include "proce_tlp.h"
#include <limits.h>
#include <string.h>
#include "abort_difficult_command.h"

#if defined(MPV)

int CONV cmd_mpv( char **lasts )
{
  const char *str = strtok_r( NULL, str_delimiters, lasts );
  char *ptr;
  long l;

  if ( str == NULL )
    {
      str_error = str_bad_cmdline;
      return -2;
    }
  else if ( ! strcmp( str, "num" ) )
    {
      str = strtok_r( NULL, str_delimiters, lasts );
      if ( str == NULL )
        {
          str_error = str_bad_cmdline;
          return -2;
        }
      l = strtol( str, &ptr, 0 );
      if ( ptr == str || l == LONG_MAX || l < 1 || l > MPV_MAX_PV )
        {
          str_error = str_bad_cmdline;
          return -2;
        }

      AbortDifficultCommand;

      mpv_num = (int)l;

      return 1;
    }
  else if ( ! strcmp( str, "width" ) )
    {
      str = strtok_r( NULL, str_delimiters, lasts );
      if ( str == NULL )
        {
          str_error = str_bad_cmdline;
          return -2;
        }
      l = strtol( str, &ptr, 0 );
      if ( ptr == str || l == LONG_MAX || l < MT_CAP_PAWN )
        {
          str_error = str_bad_cmdline;
          return -2;
        }

      AbortDifficultCommand;

      mpv_width = (int)l;

      return 1;
    }

  str_error = str_bad_cmdline;
  return -2;
}

#endif

