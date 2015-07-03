#ifndef PROCE_MNJ_H
#define PROCE_MNJ_H

#include "shogi.h"

#if defined(MNJ_LAN)
int CONV proce_mnj( tree_t * restrict ptree );
int CONV cmd_mnj( char **lasts );
#endif

#endif /* PROCE_MNJ_H */
