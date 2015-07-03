#ifndef PROCE_CSA_H
#define PROCE_CSA_H

#include "shogi.h"

#if defined(CSA_LAN)
int CONV proce_csalan( tree_t * restrict ptree );
int CONV cmd_connect( tree_t * restrict ptree, char **lasts );
int CONV cmd_sendpv( char **lasts );
#endif

#endif /* PROCE_CSA_H */

