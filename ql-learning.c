/********** Libraries ***********/
#include "net/mac/tsch/tsch.h"
#include "q-learning.h"

/********** global variables ***********/
// parameters to calculate the reward
float theta1 = 3;
float theta2 = 1.5;
float theta3 = 0.5;

// Q-value updating paramaters
float learning_rate = 0.1;
float discaount_factor = 0.9;

// default state varibale
env_state *current_state;

// Q-table to store q-values, 2 index means -> action is 3, first three slots are active
q_value table[Q_VALUE_LIST_SIZE];
q_value *q_list = &table;

/********** Functions *********/
// Function to calculate th reward
float reward(uint8_t n_tx, uint8_t n_rx, uint8_t n_buff_prev, uint8_t n_buff_new) {
    queue_packet_status *queue_tx = func_custom_queue_tx();
    queue_packet_status *queue_rx = func_custom_queue_rx();
    return (theta1 * (queue_tx->size + queue_tx->size) - theta2 * (n_buff_prev - n_buff_new));
}

// Function to find the action with highest q-value, returns the index of max value
uint8_t get_highest_q_val(void) {
    int max_val_index = 0;
    for (int i=1; i < Q_VALUE_LIST_SIZE; i++) {
        if (q_list[i]->val > q_list[max_val_index]->val) {
            max_val_index = i;
        }
    }
    return max_val_index;
}

// Function to get the current state (buffer_size and energy_level)
env_state *get_current_state(void) {
    current_state->buffer_size = 0;
    current_state->energy_level = 0;
    return current_state;
}

// Updating the q-value table
void update_q_table(uint8_t action, float got_reward) {
    q_list[action]->val = (1 - learning_rate) * q_list[action]->val - 
    learning_rate * (got_reward + discaount_factor * q_list[action]->val);
}

// function to return the main q-list
q_value *get_q_table(void) {
    return q_list;
}