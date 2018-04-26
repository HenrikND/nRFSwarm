#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "crc_filter.h"

//TODO: Test this 

int8_t rssi_averaging(int8_t rssi, uint8_t i)
{
  static int16_t rssi_vault[NUMBER_OF_NODES][5];
  static uint8_t message_counter = 0; // For the first five values
  int16_t average_rssi = 0;

  if(message_counter < 5)
    message_counter++;

  // Move newest RSSI value in and oldest value out

  for(uint8_t h = 0; h < 4; h++)
  {
    rssi_vault[i][h] = rssi_vault[i][h+1];
    average_rssi += rssi_vault[i][h];
  }

  rssi_vault[i][4] = rssi;
  average_rssi = (average_rssi + rssi) / message_counter;
  
  return average_rssi;
}

int8_t crc_filter(int8_t *RSSI_values, uint16_t id, int8_t rssi, uint8_t crc)
{
  static uint16_t saved_id[NUMBER_OF_NODES];
  static uint8_t  counter_value[NUMBER_OF_NODES];
  static uint8_t saved_values = 0;
  int8_t largest_value = -100;

  // Check if ID already exists or not, update drop out value and RSSI value
  // This is skipped if the CRC value is not OK.

  if(crc == 1)
  {
    for(uint8_t i = 0; i < NUMBER_OF_NODES; i++)
    {
      if(id == saved_id[i])
      {
        RSSI_values[i] = rssi_averaging(rssi, i);
        counter_value[i] = DROP_OUT_VALUE;
        i = NUMBER_OF_NODES; 
      }
      else if(i == NUMBER_OF_NODES - 1)
      {
        saved_id[saved_values] = id;
        RSSI_values[saved_values] = rssi_averaging(rssi, saved_values);
        counter_value[saved_values] = DROP_OUT_VALUE;
        saved_values++;
      }
    }
  }

  // Perform decrementation of counter values, if 0, put RSSI at a low value (-100dBm)
  for(uint8_t k = 0; k < NUMBER_OF_NODES; k++)
  {
    if(counter_value[k] <= 0)
      RSSI_values[k] = 0;
    else
      counter_value[k]--;
  }

  // Return the largest RSSI value
  for(uint8_t m = 0; m < NUMBER_OF_NODES; m++)
  {
    if(largest_value <= RSSI_values[m] && !(RSSI_values[m] == 0))
      largest_value = RSSI_values[m];
  }

  return largest_value;
}
