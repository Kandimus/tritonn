#ifndef __RPMSG_CONNECTOR_HEADER
#define __RPMSG_CONNECTOR_HEADER

extern "C" {

#include <stdint.h>
#include "rpmsg_connector_common.h"

int   candrv_init( void );
int   candrv_cmd( uint32_t m_func, int m_idx, void *data ); /*, uint16_t m_addr */
void  candrv_deinit( void );

}

#endif
