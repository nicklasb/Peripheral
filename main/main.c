
/* Boat monitoring peripheral
 *
 * LICENSE:
 * This code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <Arduino.h>

#include <sensor_task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**********************
 *  INCLUDES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   APPLICATION MAIN
 **********************/


void app_main()
{
    //initArduino();
    init_sdp_task();

    // Suppose we need this as we are arduino:ing here.
    while (1) {
        vTaskDelay(100);
    }
}



