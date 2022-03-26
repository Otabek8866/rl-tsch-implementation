#ifndef Q_LEARNING_HEADER
#define Q_LEARNING_HEADER


/********** Libraries **********/
#include "contiki.h"

/******** Configuration *******/
// Size of Q-value table (default is the default slotframe size)
#ifndef Q_VALUE_LIST_SIZE
#define Q_VALUE_LIST_SIZE TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#endif

/************ Types ***********/
// structure to store q-values
typedef struct {
    float val;
    uint8_t slot_number;
} q_value;

// structure to store a state of the node
typedef struct {
    uint8_t buffer_size;
    float energy_level;
} env_state;

/********** Functions *********/
// Function to calculate th reward
float reward(uint8_t n_tx, uint8_t n_rx, uint8_t n_buff, uint8_t n_buff_new);

// Function to find the action with highest q-value, , returns the index of max value
uint8_t get_highest_q_val(void);

// Function to get the current state (buffer_size and energy_level)
env_state *get_current_state(void);

// Updating the q-value table
void update_q_table(uint8_t action, float got_reward);

// function to return the main q-list
q_value *get_q_table(void);

#endif /* Q_LEARNING_HEADER */
