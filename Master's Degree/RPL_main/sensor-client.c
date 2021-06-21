
#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"

#include <math.h>
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"


#include "dev/serial-line.h"
#if CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#else
#include "dev/uart1.h"
#endif
#include "collect-common.h"
#include "collect-view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

/* we need node id to help distinguish to do or not to do attack action. */
#include "sys/node-id.h"

#include "net/rpl/rpl-private.h"

#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688

#define UDP_EXAMPLE_ID  190

// 週期單位設定
#ifndef PERIOD
#define PERIOD 30
#endif

#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(SEND_INTERVAL)
#define MAX_PAYLOAD_LEN	        80
#define PRINT_SEND_INTERVAL     (2 * SEND_TIME)


short send_count = 0;
short reply_count = 0;
// 計數周期次數
short min_time_count = 0;


// change parent define
#define RPL_ZERO_LIFETIME 0
short suspicious_list [30];
short environment_depth = 0;

short old_parent_nodeID = 0;
short current_parent_nodeID = 0;
short second_blacknodeID = 0;

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process,&collect_common_process);
/*---------------------------------------------------------------------------*/
void
collect_common_set_sink(void)
{
  /* A udp client can never become sink */
}
/*---------------------------------------------------------------------------*/
void collect_common_net_print(void) {}

/*
void collect_common_net_print(void) {

    rpl_dag_t *dag;
    uip_ds6_route_t *r;
    
    // 自身節點rank
    short curr_rank ;
    // 抓親節點
    rpl_parent_t *p = nbr_table_head(rpl_parents);
    
    // ip6 parent
    rpl_parent_t *current_parent;
    
    
    // 抓rpl instant
    rpl_instance_t *instance;
    
    uip_ds6_nbr_t *nbr = rpl_get_nbr(p);

// --------------------------------- //

    // Let's suppose we have only one instance 
    dag = rpl_get_any_dag();
    // 抓自身節點rank
    curr_rank = dag->rank;
    instance = dag->instance;
    // 印出自己 id , rank , nbr amount.
    printf("** node:%u , RPL: rank %u, %u nbr(s) ** \n",node_id, curr_rank, uip_ds6_nbr_num());


    if(dag->preferred_parent != NULL) 
        {
        PRINTF("Preferred parent: ");
        current_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
        PRINT6ADDR(current_parent);
        PRINTF("\n");        
        }

    for(r = uip_ds6_route_head();
      r != NULL;
      r = uip_ds6_route_next(r)) 
         {
         PRINT6ADDR(&r->ipaddr);
         }
    PRINTF("---\n");


  PRINTF("My Neighbor's ipv6 ");
    //PRINT6ADDR(&ABWM_blackhole->ipaddr);
   PRINTF("\n");
   
   PRINT6ADDR(&nbr->ipaddr);
//    old_rank = instance->current_dag->rank;
      
    PRINTF("link: \n");

//    for(r = uip_ds6_route_head();
//        r != NULL;
//        r = uip_ds6_route_next(r)) 
   
     // function 印出鄰居資料
     rpl_print_neighbor_list();

//     nbr_table_next(rpl_parents, p);
     
     //rpl_set_preferred_parent(dag, &nbr->ipaddr);

     PRINTF("nbr list end.\n");
}
*/

/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if (uip_newdata()) 
     {
     reply_count +=1;

     str = uip_appdata;
     str[uip_datalen()] = '\0';
     printf("DATA recv '%s'\n", str);

     }
 
}
/*---------------------------------------------------------------------------*/
void
collect_common_send(void)
{
  static uint8_t seqno;
  struct {
    uint8_t seqno;
    uint8_t for_alignment;
    struct collect_view_data_msg msg;
  } msg;
  /* struct collect_neighbor *n; */
  uint16_t parent_etx;
  uint16_t rtmetric;
  uint16_t num_neighbors;
  uint16_t beacon_interval;
  rpl_parent_t *preferred_parent;
  linkaddr_t parent;
  rpl_dag_t *dag;

  if(client_conn == NULL) {
    /* Not setup yet */
    return;
  }
  memset(&msg, 0, sizeof(msg));
  seqno++;
  if(seqno == 0) {
    /* Wrap to 128 to identify restarts */
    seqno = 128;
  }
  msg.seqno = seqno;

  linkaddr_copy(&parent, &linkaddr_null);
  parent_etx = 0;

  /* Let's suppose we have only one instance */
  dag = rpl_get_any_dag();
  if(dag != NULL) {
    preferred_parent = dag->preferred_parent;
    if(preferred_parent != NULL) {
      uip_ds6_nbr_t *nbr;
      nbr = uip_ds6_nbr_lookup(rpl_get_parent_ipaddr(preferred_parent));
      if(nbr != NULL) {
        /* Use parts of the IPv6 address as the parent address, in reversed byte order. */
        parent.u8[LINKADDR_SIZE - 1] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 2];
        parent.u8[LINKADDR_SIZE - 2] = nbr->ipaddr.u8[sizeof(uip_ipaddr_t) - 1];
        parent_etx = rpl_get_parent_rank((uip_lladdr_t *) uip_ds6_nbr_get_ll(nbr)) / 2;
      }
    }
    rtmetric = dag->rank;
    beacon_interval = (uint16_t) ((2L << dag->instance->dio_intcurrent) / 1000);
    num_neighbors = uip_ds6_nbr_num();
  } else {
    rtmetric = 0;
    beacon_interval = 0;
    num_neighbors = 0;
  }

  /* num_neighbors = collect_neighbor_list_num(&tc.neighbor_list); */
  collect_view_construct_message(&msg.msg, &parent,
                                 parent_etx, rtmetric,
                                 num_neighbors, beacon_interval);

  uip_udp_packet_sendto(client_conn, &msg, sizeof(msg),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
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
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void)
{


  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  seq_id++;
  PRINTF("DATA send to %d 'Hello %d'\n",
         server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
  /*input udpdata to packet. */
  sprintf(buf, "Hello %d from the client", seq_id);
  uip_udp_packet_sendto(client_conn, buf, strlen(buf),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
  send_count++;
  //PRINTF("SEND DONE !\n");

}
/*---------------------------------------------------------------------------*/
static void udp_total_send(void)
{
  PRINTF("Node%i HAS TOTAL SEND: %i",node_id,send_count);
  //putFloat(rec_count,DEC1);
  PRINTF("\n");

}

static void reset_send_reply(void)
{
  send_count = 0;
  reply_count = 0;
}


/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  short i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) 
          {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
          }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  /* set server address */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_client_process, ev, data)
{

// ------------------------------------------//
  // 
  static struct etimer periodic;
  //static struct ctimer backoff_timer;

  // capture current dag
  rpl_dag_t *dag;


  rpl_parent_t *p;

  // UDP丟失數
  short diff = 0;


// RPL 
  
  // 抓當前instance
  rpl_instance_t *instance;
  
  
  // 抓鄰居 
  uip_ds6_nbr_t *nbr;

  // 自身節點rank
  //short self_rank ;
  //short previous_parent_rank;
  //short current_parent_rank;
  
   
  // black list
   rpl_parent_t *blackhole_p =NULL ;
  // rpl_parent_t *spare_parent =NULL;
   //rpl_parent_t *nbr_position =NULL;

  short i;
  short nbr_amount;
  

#if WITH_COMPOWER
  static int print = 0;
#endif

// ------------------------------------------//



  PROCESS_BEGIN();
  PROCESS_PAUSE();

  // 電耗顯示
  //powertrace_start(CLOCK_SECOND * 2);


  set_global_address();
  
  PRINTF("UDP client set up !\n");
  //print_local_addresses();



  // 創建UDP link with server.
  /* new connection with remote host */

  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) 
    {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
    }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	 UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
  
  PRINTF("node%i: DEFAULT UDP TOTAL SEND: %i\n",node_id,send_count);
  // 創建UDP link with server. END
/*
#if WITH_COMPOWER
  powertrace_sniff(POWERTRACE_ON);
#endif
*/

  // 設定計時器 30, 60
  etimer_set(&periodic, SEND_INTERVAL);
  //ctimer_set(&backoff_timer, 2*SEND_TIME, reset_send_reply, NULL);

  // 初始化可疑清單
  for ( i=0; i<30 ; i++){
        suspicious_list [i] = 0;        
        //PRINTF("%u\n",suspicious_list [i]);   
       } 
  ABWM_blackhole = NULL;
  ABWM_blackhole_second = NULL;

  while(1) 
    {

    PROCESS_YIELD();

    if(ev == tcpip_event) 
 	tcpip_handler();
    

    if(etimer_expired(&periodic)) 
      {
      
      /*   測試區域 -   */

      // 環境深度
      //printf("current envir_depth: %u\n",environment_depth);
      
      // 每30秒顯示 UDP send/reply amount.
      PRINTF("send count %i, reply count %i \n",send_count,reply_count);

      // 計數/顯示經過多久
      min_time_count += 1;
      PRINTF("time count %i \n",min_time_count);


      /*   測試區域 |    */
      
      
      // 抓RPL dag訊息
      dag = rpl_get_any_dag();
 
      if(dag->preferred_parent != NULL) 
        {
        PRINTF("Parent: ");
        PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
        PRINTF("\n");

        

        printf("RPL parent_nodeID-rank %3u-%3u\n",
        nbr_table_get_lladdr(rpl_parents,dag->preferred_parent)->u8[7],
                             dag->preferred_parent->rank);
        // 裝入 parent NodeID
        current_parent_nodeID = nbr_table_get_lladdr(rpl_parents,dag->preferred_parent)->u8[7];

        // 裝入 parent rank        
        //current_parent_rank = dag->preferred_parent->rank;

        // 裝入 鄰居數量 
        nbr_amount = uip_ds6_nbr_num();

        printf("neighbor amounts %u\n",nbr_amount);
              

        if ( old_parent_nodeID == 0 )
          {
          old_parent_nodeID = current_parent_nodeID;
          printf("old_parent_nodeID %3u\n",  old_parent_nodeID);
          }

        if (current_parent_nodeID != old_parent_nodeID)
           {
           printf("parent change!! \n");
           send_count = 0;
           reply_count = 0;
           old_parent_nodeID = current_parent_nodeID;
           
           }



        }


      etimer_reset(&periodic);
      
      
      // 控制送UDP
      //if (min_time_count > 6)
         send_packet(); 
      

      // 判斷丟失率
      diff = send_count - reply_count;

      //  接收率 <= 20%, ABWM Ignite.  // 自然丟失穩定
      if (send_count >5 && reply_count<1 && current_parent_nodeID!=0 || diff >=10)
          {
          if (diff >=10)
             {
              send_count  =0;
              reply_count =0;
                    
              if (suspicious_list [current_parent_nodeID] >=1  )
                  suspicious_list [current_parent_nodeID] -=1 ;
              
              continue;
             }
          /***********/
          
         
          send_count  =0;
          reply_count =0;    
          
          
         
    
          p = nbr_table_head(rpl_parents);
    
          printf("parent_node_id: %3u rank: %u.\n",
                  nbr_table_get_lladdr(rpl_parents, p)->u8[7],  dag->preferred_parent->rank);
         
          
          
          // 路線root不變
          if (p->rank == 256)
              {
              PRINT6ADDR(p->rank);
              PRINTF("\n");
              PRINTF("remain root-link.\n");
              continue;
              }
          

          //黑洞質疑處理
          suspicious_list [current_parent_nodeID] += 1;
     
          if (suspicious_list [current_parent_nodeID] > 2)
             {
             blackhole_p = dag->preferred_parent;

             PRINTF("blackhole ip: ");            
             PRINT6ADDR(rpl_get_parent_ipaddr(blackhole_p));
             PRINTF("\n");    

             if (ABWM_blackhole ==NULL)
                 ABWM_blackhole = blackhole_p;
             if (ABWM_blackhole_second == NULL && ABWM_blackhole != blackhole_p)
                 {
                 ABWM_blackhole_second = blackhole_p;
                 PRINTF("blackhole_second ip: ");            
                 PRINT6ADDR(rpl_get_parent_ipaddr(ABWM_blackhole_second));
                 PRINTF("\n");
                 second_blacknodeID = current_parent_nodeID;
                 }
             } 

          

          if (suspicious_list [current_parent_nodeID] > 4)
             {
             ABWM_blackhole = NULL;
             ABWM_blackhole_second = NULL;
             suspicious_list [current_parent_nodeID] = 0;
             suspicious_list [second_blacknodeID] = 0; 
             }

          printf("current_parent_nodeID: %u doubt %u time. \n", current_parent_nodeID, suspicious_list [current_parent_nodeID]);

          // rank比較核心演算法
          PRINTF("--ABWM IGNITE.-- \n");
          

/*      environment depth level 1   start         */
// 接收率有問題，level 1: depth = 1

          // 列印鄰居信息 為了省RAM 關掉
          //collect_common_net_print();


/*

          for (i=0 ; i < nbr_amount; i++)
             {
              nbr_position = nbr_table_next(rpl_parents, p);

              if (nbr_position->rank <= current_parent_rank )
                 {
                 printf("spare's node does exist!\n");
                 if (spare_parent == NULL)
                     spare_parent = nbr_position;
                 break;
                 }


             }
          
             

             if (spare_parent == NULL)
                {
                printf("OVER DEPTH LEVEL 1.\n");
                }
             // 直接竄改cache
*/





             dag->preferred_parent->rank = 65535;
             // save ram
             //collect_common_net_print();


            }   
/*      environment depth level 1  end         */



/*

            if (self_rank > nbr_position->rank > current_parent_rank)
                {
                if (spare_parent== NULL)
                    spare_parent = nbr_position;
                 else
                    {
                    if ((spare_parent->rank) < (nbr_position->rank))
                        continue;
                    if ((spare_parent->rank) > (nbr_position->rank))
                       spare_parent = nbr_position;
                    }
                
                }
            }
*/

    /*      

          //PRINTF("UDP client process started nbr:%d routes %d\n",
            //NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);
           
          instance = dag->instance;
          current_parent = instance->current_dag->preferred_parent;
          
          //nbr = rpl_get_nbr(current_parent);
              // 清除鏈路
          //p = &nbr->ipaddr;

          nbr_table_unlock(rpl_parents, current_parent);
          rpl_nullify_parent(current_parent);
          rpl_remove_parent(current_parent);
          nbr_table_remove(rpl_parents, current_parent);

          rpl_remove_routes_by_nexthop(current_parent,dag);
              PRINTF("spare node:");
              PRINT6ADDR(spare_parent);
              PRINTF("\n");
              //spare_parent = p;
          nbr_table_lock(rpl_parents, spare_parent);
          dag->preferred_parent = spare_parent;
          
          dao_output(current_parent, RPL_ZERO_LIFETIME);
              
             //rpl_set_default_route(instance, p);
             //rpl_set_preferred_parent(dag, p);
   */    
          
      }
    }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/  
// 程式保留區

/* 第2計時器
      if (ctimer_expired(&backoff_timer))
        {
        ctimer_reset(&backoff_timer);
        
        //ctimer_set(&backoff_timer, SEND_TIME, reset_send_reply, NULL);
        //PRINTF("send count %i, reply count %i \n",send_count,reply_count);

        }
*/

/*
// 處裡黑洞2
      if (dag->preferred_parent == blackhole_p)
          {
           nbr_table_unlock(rpl_parents, blackhole_p);
           PRINT6ADDR(rpl_get_parent_ipaddr(blackhole_p));
           rpl_nullify_parent(blackhole_p);
           rpl_remove_parent(blackhole_p);
           dao_output(blackhole_p, RPL_ZERO_LIFETIME);   

           nbr_table_lock(rpl_parents, spare_parent);

           dag->preferred_parent = spare_parent;
           nbr_table_remove(rpl_parents, blackhole_p);
          }
*/
 // 處理黑洞1
/*
    if (blackhole_p !=NULL)
        {
        nbr_table_remove(rpl_parents, blackhole_p);
        dao_output(blackhole_p, RPL_ZERO_LIFETIME); 
        }
*/     

// 鄰居rank比對
/*        
        p = nbr_table_head(rpl_parents);
        for (i=0 ; i< nbr_amount;i++)
            {
            p = nbr_table_next(rpl_parents, p);
            if  (p->rank ==(dag->preferred_parent->rank))
               printf("exist!!!\n");
            }
*/      

