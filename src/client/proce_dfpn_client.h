#ifndef PROCE_DFPN_CLIENT_H
#define PROCE_DFPN_CLIENT_H

#include "shogi.h"

#if defined(DFPN_CLIENT)
int CONV cmd_dfpn_client( tree_t * restrict ptree, char **lasts );
#endif

#endif /* PROCE_DFPN_CLIENT_H */
