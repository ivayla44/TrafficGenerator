#ifndef TRAFFIC_GENERATOR_PRECOMPILED_H
#define TRAFFIC_GENERATOR_PRECOMPILED_H

////////////////////////////////////////////////////////
// C STD/SYS HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdbool.h>
#include <sys/resource.h>
#include <time.h>


// LINUX
////////////////////////////////////////////////////////
// LIB HEADERS

#include <pcap/pcap.h>
#include "3rdparty/libfyaml/install/include/libfyaml.h"
#include "3rdparty/libconfig/install/include/libconfig.h"
#include "3rdparty/cjson/install/include/cjson/cJSON.h"


////////////////////////////////////////////////////////
// DPDK INCLUDES

#include <rte_config.h>
#include <rte_errno.h>
#include <rte_eal.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_mbuf_core.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_bitops.h>
#include <rte_ip.h>


////MACOS
//////////////////////////////////////////////////////////
//// LIB HEADERS
//
//#include <pcap/pcap.h>
//#include <libfyaml/include/libfyaml.h>
//#include <libconfig/include/libconfig.h>
//#include "/usr/local/opt/cjson/include/cjson/cJSON.h"
//
//////////////////////////////////////////////////////////
//// DPDK INCLUDES
//
//#include "config/rte_config.h"
//#include "lib/eal/include/rte_errno.h"
//#include "lib/eal/include/rte_eal.h"
//#include "lib/eal/include/rte_lcore.h"
//#include "lib/mempool/rte_mempool.h"
//#include "lib/mbuf/rte_mbuf_core.h"
//#include "lib/mbuf/rte_mbuf.h"
//#include "lib/ethdev/rte_ethdev.h"
//#include "lib/net/rte_ether.h"
//#include "lib/eal/include/rte_bitops.h"
//#include "lib/net/rte_ip.h"


#endif //TRAFFIC_GENERATOR_PRECOMPILED_H
