#include "contiki.h"
#include "lib/random.h"
#include "sys/timer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "dev/serial-line.h"
#include "net/ipv6/uip-ds6-route.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

// for decrease rank attack
#define RPL_CONF_MIN_HOPRANKINC 0

// Ipv6 prefix
#define UIP_DS6_DEFAULT_PREFIX 0xfd00

static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void tcpip_handler(void) {
    // do nothing
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void print_local_addresses(void) {
    int i;
    uint8_t state;
    PRINTF("Client IPv6 addresses: ");
    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if(uip_ds6_if.addr_list[i].isused && 
            (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
                PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
                PRINTF("\n");
                if(state == ADDR_TENTATIVE) {
                    uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
                }
        }
    }
}
/*---------------------------------------------------------------------------*/
static void set_global_address(void) {

    uip_ipaddr_t ipaddr;
    
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

/*
    The choice of server address determines its 6LowPAN header compression.
    (Our address will be compressed Mode 3 since it is derived from our
    link-local address)
    Obviously the choice made here must also be selected in udp-server.c.

    For correct Wireshark decoding using a sniffer, and the /64 prefix to the
    6LowPAN protocol preferences,
    e.g. set Context 0 to fd00::. At present Wireshark copies Context/128 and
    then overwrites it.
    (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit
    compressed address of fd00::1111:22ff:fe33:xxxx)

    Note the ICMPv6 checksum verification depends on the correct uncompressed
    addresses.
*/
// #if 0
// /* Mode 1 - 64 bits inline */
//     uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
// #elif 1
// /* Mode 2 - 16 bits inline */
//     uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
// #else
// /* Mode 3 - derived from server link-local (MAC) address */
//     uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0x0250, 0xfea8, 0xcd1a);   //redbee-econotag
// #endif
    uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data) {
    //uint8_t power = 31;
    //cc2420_set_txpower(power);


    PROCESS_BEGIN();

    PROCESS_PAUSE();

    set_global_address();

    PRINTF("UDP client process started nbr:%d routes %d\n",
            NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);
    print_local_addresses();

    while(1) {
        PROCESS_YIELD();
        if(ev == tcpip_event) {
            tcpip_handler();
        }
    }

    PROCESS_END();
}
