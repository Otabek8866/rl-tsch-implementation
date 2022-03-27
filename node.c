/********** Libraries ***********/
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "lib/random.h"
#include "sys/node-id.h"
#include "q-learning.h"
#include "net/queuebuf.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/********** Global variables ***********/
#define UDP_PORT 8765

// period to send a packet to the udp server
#define SEND_INTERVAL (60 * CLOCK_SECOND)

// period to update Q-values
#define Q_TABLE_INTERVAL (120 * CLOCK_SECOND)

// period to finish setting up Minimal Scheduling 
#define SET_UP_MINIMAL_SCHEDULE (10 * CLOCK_SECOND)

// UDP communication process
PROCESS(node_udp_process, "UDP communicatio process");
// Q-Learning and scheduling process
PROCESS(scheduler_process, "RL-TSCH Scheduler Process");

AUTOSTART_PROCESSES(&node_udp_process, &scheduler_process);

// variable to stop the udp comm for a while
uint8_t udp_com_stop = 1;

// To build the network with Minimal Schedule at first
// uint8_t setup_schedule_finished = 0;

// timer to check if the minimal schedule finished setting-up
struct etimer minimal_schedule_setup_timer; 

// data to send to the server
char custom_payload[PACKETBUF_CONF_SIZE];

// queue length pointer
uint8_t *queue_length;

/********** Scheduler Setup ***********/
// Function starts Minimal Shceduler
static void init_tsch_schedule(void)
{
  // create a single slotframe
  struct tsch_slotframe *sf_min;
  tsch_schedule_remove_all_slotframes();
  sf_min = tsch_schedule_add_slotframe(0, TSCH_SCHEDULE_DEFAULT_LENGTH);

  // shared/advertising cell at (0, 0)
  tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX | LINK_OPTION_SHARED,
                         LINK_TYPE_ADVERTISING, &tsch_broadcast_address, 0, 0, 1);

  // all other cell are initialized as shared/dedicated 
  for (int i = 1; i < TSCH_SCHEDULE_DEFAULT_LENGTH; i++)
  {
    tsch_schedule_add_link(sf_min, LINK_OPTION_TX | LINK_OPTION_RX | LINK_OPTION_SHARED,
                           LINK_TYPE_NORMAL, &tsch_broadcast_address, i, 0, 1);
  }
}

// function to receive udp packets
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

// function to populate the payload
void create_payload() {
  for (int i = 0; i < PACKETBUF_CONF_SIZE; i++){
    custom_payload[i] = i % 26 + 'a';
  }
}

/********** UDP Communication Process - Start ***********/
PROCESS_THREAD(node_udp_process, ev, data)
{
  static struct simple_udp_connection udp_conn;
  static struct etimer periodic_timer;
  
  static uint32_t seqnum;
  uip_ipaddr_t dst;

  PROCESS_BEGIN();
  
  // creating the payload and starting the scheduler
  create_payload();
  init_tsch_schedule();

  // NETSTACK_RADIO.off()
  // NETSTACK_MAC.off()

  /* Initialization; `rx_packet` is the function for packet reception */
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, rx_packet);

  // setting up the timer to create network with Minial Scheduling
  etimer_set(&minimal_schedule_setup_timer, SET_UP_MINIMAL_SCHEDULE);

  if (node_id == 1)
  { /* node_id is 1, then start as root*/
    NETSTACK_ROUTING.root_start();
  }

  // Initialize the mac layer
  NETSTACK_MAC.on();

  // check if the Minimal Scheduling finished
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&minimal_schedule_setup_timer));
  udp_com_stop = 0;
  
  // get the pointer to queue length at the beginning
  queue_length = getCurrentQueueLen();

  // if this is a simple node, start sending upd packets
  if (node_id != 1)
  {
    // start the timer for periodic udp packet sending
    etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
    /* Main UDP comm Loop */
    while (udp_com_stop == 0)
    {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dst))
      {
        /* Send custom payload to the network root node and increase the packet count number*/
        seqnum++;
        LOG_INFO("Send to ");
        LOG_INFO_6ADDR(&dst);
        LOG_INFO_(", application packet number %" PRIu32 "\n", seqnum);
        simple_udp_sendto(&udp_conn, &custom_payload, sizeof(custom_payload), &dst);
      }
      etimer_set(&periodic_timer, SEND_INTERVAL);
    }
  }
  /********** UDP Communication Process - End ***********/
  PROCESS_END();
}


/********** RL-TSCH Scheduler Process - Start ***********/
PROCESS_THREAD(scheduler_process, ev, data)
{
  static struct etimer q_table_update_timer;

  PROCESS_BEGIN();

  etimer_set(&q_table_update_timer, Q_TABLE_INTERVAL);

  /* Main Scheduler Loop */
  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&q_table_update_timer));

    // didn't work, solve this
    //int buffer_size = tsch_queue_global_packet_count();
    LOG_INFO("Current Packet Buffer Size: %u\n", *queue_length);

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
    
    etimer_set(&q_table_update_timer, Q_TABLE_INTERVAL);
  }
  /********** RL-TSCH Scheduler Process - End ***********/
  PROCESS_END();
}
