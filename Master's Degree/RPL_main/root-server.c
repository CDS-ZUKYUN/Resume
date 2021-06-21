
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

// 顯示電耗function
//#include <powertrace.h>

// collect view
#include "net/linkaddr.h"
#include "dev/serial-line.h"

#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif

#include "collect-common.h"
#include "collect-view.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688

#define UDP_EXAMPLE_ID  190

// reply flag set
#define SERVER_REPLY 1

// the per time value set for etimer / ctimer 
#ifndef PERIOD
#define PERIOD 60
#endif

#ifndef PRINT_INTERVAL
#define PRINT_INTERVAL (PERIOD * CLOCK_SECOND)
#endif

#define UIP_DS6_DEFAULT_PREFIX 0xfd00

static struct uip_udp_conn *server_conn;
/*---------------------------------------------------------------------------*/

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process,&collect_common_process);

/*---------------------------------------------------------------------------*/
// collect view 1
/*---------------------------------------------------------------------------*/
void
collect_common_set_sink(void)
{
}
/*---------------------------------------------------------------------------*/
void
collect_common_net_print(void)
{
  printf("I am sink!\n");
}
/*---------------------------------------------------------------------------*/
void
collect_common_send(void)
{
  /* Server never sends */
}
/*---------------------------------------------------------------------------*/
void
collect_common_net_init(void)
{
#if CONTIKI_TARGET_Z1
  uart0_set_input(serial_line_input_byte);
#else
  uart1_set_input(serial_line_input_byte);
#endif
  serial_line_init();

  PRINTF("I am sink!\n");
}
/*---------------------------------------------------------------------------*/
/* float print Preconditions. */
/*---------------------------------------------------------------------------*/
/*
void putLong(long x)
{
    if(x < 0)
    {
        putchar('-');
        x = -x;
    }
    if (x >= 10) 
    {
        putLong(x / 10);
    }
    putchar(x % 10+'0');
}

typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;
*/
/*
void putFloat( float f, tPrecision p )
{
    long i = (long)f ;
    putLong( i ) ;
    f = (f - i) * p ;
    i = abs((long)f) ;
    if( fabs(f) - i >= 0.5f )
    {
        i++ ;
    }
    putchar('.') ;
    putLong( i ) ;
    putchar('\n') ;
}
*/
/*---------------------------------------------------------------------------*/
/* root calculate udp packet receive rate */
//float rec_count = 0;
short rec_count = 0;
static void statistical_acceptance_rate(void)
{

  PRINTF("UDP TOTAL RECEIVE: %i",rec_count);
  //putFloat(rec_count,DEC1);
  PRINTF("\n");

}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
   
  char *appdata;

  uint8_t *uchs_appdata;
  linkaddr_t sender;
  uint8_t seqno;
  uint8_t hops;
  
  if(uip_newdata()) 
    {
     
    appdata = (char *)uip_appdata;
    
    // string_appdata[uip_datalen()] = 0;
    printf("appdata length: %u\n", strlen(appdata));

    if (strlen(appdata) >= 30)
      {
      
      // increase count when get udp data packet.     
      rec_count++;

      PRINTF("DATA recv '%s' from ", appdata);
      PRINTF("%d",
           UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
      PRINTF("\n");

      PRINTF("DATA sending reply\n");
      uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
      uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
      uip_create_unspecified(&server_conn->ripaddr);
      appdata =NULL;

      }
    else
      {
      uchs_appdata = (uint8_t *)uip_appdata;
      sender.u8[0] = UIP_IP_BUF->srcipaddr.u8[15];
      sender.u8[1] = UIP_IP_BUF->srcipaddr.u8[14];
      seqno = *appdata;
      hops = uip_ds6_if.cur_hop_limit - UIP_IP_BUF->ttl + 1;
      collect_common_recv(&sender, seqno, hops,
                          uchs_appdata + 2, uip_datalen() - 2);
      uchs_appdata = NULL;
      }
     
    }


}


/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* ABWM declaration*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  /*per time to print total udp packet (1min)*/
  static struct etimer periodic;
  static struct ctimer backoff_timer;
  

  PROCESS_BEGIN();

  PROCESS_PAUSE();

#if PLATFORM_HAS_BUTTON
  SENSORS_ACTIVATE(button_sensor);
#endif

  PRINTF("UDP server started\n");
  
  
#if UIP_CONF_ROUTER
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
  /* uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr); */
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */


//#if TSCH_TIME_SYNCH
  //foure_timesynch_init(master_synched); /* Inıtialize after rpl set root */
//#endif /* TIMESYNCH_CONF_ENABLED */  

  //print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
  NETSTACK_RDC.off(1);  


  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));
#if 0
  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));
#endif


  // default cout output.
  statistical_acceptance_rate();

  // per time to print total count.
  //ctimer_set(&backoff_timer, SEND_TIME, statistical_acceptance_rate, NULL);

  etimer_set(&periodic, PRINT_INTERVAL);

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();

      if(etimer_expired(&periodic))
        {
        statistical_acceptance_rate();
        etimer_reset(&periodic);
        
        }
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
