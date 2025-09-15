#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"

#define UART_NUM UART_NUM_2
#define TX2 17
#define RX2 16
#define BUF_SIZE (1024)

void app_main(void)
{
    // setup UART buffered IO with event queue
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));

    // configure UART parameters
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  // disable RTS/CTS
        .rx_flow_ctrl_thresh = 0,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    // set UART pins (TX: IO17, RX: IO16, no RTS/CTS)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TX2, RX2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // buffer for incoming data
    uint8_t data[128];

    // continuous read loop
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, sizeof(data) - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';  // null terminate

            // find $GPGGA in buffer
            char *start = strstr((char*)data, "$GPGGA");
            if (start) {
                char *end = strstr(start, "\n");  // end of line
                if (end) {
                    size_t sub_len = end - start;
                    if (sub_len < sizeof(data)) {
                        char gpgga[100];
                        if (sub_len >= sizeof(gpgga)) {
                            sub_len = sizeof(gpgga) - 1;  // clamp length
                        }
                        strncpy(gpgga, start, sub_len);
                        gpgga[sub_len] = '\0';  // null terminate

                        printf("%s\n", gpgga);
                    }
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
