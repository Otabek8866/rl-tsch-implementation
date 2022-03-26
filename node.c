#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "lib/random.h"
#include "sys/node-id.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT 8765
#define SEND_INTERVAL (30 * CLOCK_SECOND)

#define CHECK_INTERVAL (120 * CLOCK_SECOND)


PROCESS(node_process, "RL-TSCH Schedule");
PROCESS(custom_check_process, "CHECK MY QUEUE");
AUTOSTART_PROCESSES(&node_process, &custom_check_process);

static void init_tsch_schedule(void)
{
  struct tsch_slotframe *sf_min;
  tsch_schedule_remove_all_slotframes();
  sf_min = tsch_schedule_add_slotframe(0, TSCH_SCHEDULE_DEFAULT_LENGTH);

  // shared/advertising cell at (0, 0)
  tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX | LINK_OPTION_SHARED,
                         LINK_TYPE_ADVERTISING, &tsch_broadcast_address, 0, 0, 1);

  for (int i = 1; i < TSCH_SCHEDULE_DEFAULT_LENGTH; i++)
  {
    tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX | LINK_OPTION_SHARED,
                           LINK_TYPE_NORMAL, &tsch_broadcast_address, i, 0, 1);
  }
  // tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX,
  //                        LINK_TYPE_NORMAL, &tsch_broadcast_address, 3, 0); /* RPL */
  // tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX,
  //                        LINK_TYPE_NORMAL, &tsch_broadcast_address, 4, 0); /* data */
}

static void rx_packet(struct simple_udp_connection *c,
                      const uip_ipaddr_t *sender_addr,
                      uint16_t sender_port,
                      const uip_ipaddr_t *receiver_addr,
                      uint16_t receiver_port,
                      const uint8_t *data,
                      uint16_t datalen)
{
  uint32_t seqnum;

  if (datalen >= sizeof(seqnum))
  {
    memcpy(&seqnum, data, sizeof(seqnum));

    LOG_INFO("Received from ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_(", seqnum %" PRIu32, seqnum);
    LOG_INFO_("\tdata: %s\n", data);
  }
}

PROCESS_THREAD(node_process, ev, data)
{
  static struct simple_udp_connection udp_conn;
  static struct etimer periodic_timer;
  
  //static struct etimer check_timer;
  
  static uint32_t seqnum;
  uip_ipaddr_t dst;

  PROCESS_BEGIN();

  init_tsch_schedule();
  // NETSTACK_RADIO.off()
  // NETSTACK_MAC.off()

  /* Initialization; `rx_packet` is the function for packet reception */
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, rx_packet);
  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

  //etimer_set(&check_timer, random_rand() % CHECK_INTERVAL);

  if (node_id == 1)
  { /* Running on the root? */
    NETSTACK_ROUTING.root_start();
  }

  NETSTACK_MAC.on();

  if (node_id != 1)
  {
    /* Main loop */
    while (1)
    {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dst))
      {
        /* Send network uptime timestamp to the network root node */
        seqnum++;
        LOG_INFO("Send to ");
        LOG_INFO_6ADDR(&dst);
        LOG_INFO_(", seqnum %" PRIu32 "\n", seqnum);
        simple_udp_sendto(&udp_conn, &seqnum, sizeof(seqnum), &dst);
      }
      etimer_set(&periodic_timer, SEND_INTERVAL);
    }
  }

  PROCESS_END();
}


// printing the success rate of the packets ****************************************

PROCESS_THREAD(custom_check_process, ev, data)
{
  static struct etimer check_timer;

  PROCESS_BEGIN();

  etimer_set(&check_timer, random_rand() % CHECK_INTERVAL);

  if (node_id != 1)
  {
    /* Main loop */
    while (1)
    {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&check_timer));

      //lock_queue_tx();
      queue_packet_status *queue_tx = func_custom_queue_tx();
      LOG_INFO(" Transmission Operations in 120 seconds\n");
      for(int i=0; i < queue_tx->size; i++){
        LOG_INFO("seqnum:%u trans_count:%u timeslot:%u channel_off:%u\n", 
        queue_tx->packets[i].packet_seqno,  
        queue_tx->packets[i].transmission_count, 
        queue_tx->packets[i].time_slot,
        queue_tx->packets[i].channel_offset);
      }
      emptyQueue(queue_tx);
      //unlock_queue_tx();
      
      //lock_queue_rx();
      queue_packet_status *queue_rx = func_custom_queue_rx();
      LOG_INFO(" Receiving Operations in 120 seconds\n");
      for(int i=0; i < queue_rx->size; i++){
        LOG_INFO("seqnum:%u trans_count:%u timeslot:%u channel_off:%u\n", 
        queue_rx->packets[i].packet_seqno,  
        queue_rx->packets[i].transmission_count, 
        queue_rx->packets[i].time_slot,
        queue_rx->packets[i].channel_offset);
      }
      emptyQueue(queue_rx);
      //unlock_queue_rx();
      
      etimer_set(&check_timer, CHECK_INTERVAL);
    }
  }
  PROCESS_END();
}
// printing the success rate of the packets ****************************************
