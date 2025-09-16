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

void uart_init(void){
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
}

char* parse_data(char *s){
    char *req_str = "$GPGGA";
    char* start = strstr(s, req_str);
    if (start){
        int len = 1;
        char *c = start + 1;
        while (*c && *c != '$'){
            len++;
            c++;
        }
        char *parsed = malloc(len + 1);
        if (!parsed) return NULL;
        strncpy(parsed, start, len + 1);
        parsed[len] = '\0';
        return parsed;
    }
    return NULL;
}

void read_data(void*){
    uint8_t data[128];
    while(1){
        int length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t*)&length));
        length = uart_read_bytes(UART_NUM, data, length, 100);
        if (length > 0){
            char *c = parse_data((char *)data);
            data[length] = '\0';
            if (c != NULL){
                printf("%s\n", c);
                free(c);
            }
            // printf("%s\n", (char *)data);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // initialize uart
    uart_init();

    // RTOS task to read data from the sensor
    xTaskCreate(read_data, "Read data from sensor", 4096, NULL, 1, NULL);
}
