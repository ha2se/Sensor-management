#include <Arduino.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdi_sensor.h"

/* ================= CONFIG SDI-12 ================= */
#define UART_PORT UART_NUM_2
#define BAUD_RATE 1200

#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16
#define TX_ENABLE GPIO_NUM_21
#define BREAK_ENABLE GPIO_NUM_22

#define TX_ON gpio_set_level(TX_ENABLE, 0)
#define TX_OFF gpio_set_level(TX_ENABLE, 1)
#define BREAK_ON gpio_set_level(BREAK_ENABLE, 0)
#define BREAK_OFF gpio_set_level(BREAK_ENABLE, 1)

/* ================= PRIVATE ================= */
static void sdi12_break()
{
    TX_OFF;
    BREAK_ON;
    vTaskDelay(pdMS_TO_TICKS(20));
    BREAK_OFF;
    vTaskDelay(pdMS_TO_TICKS(10));
}

static void sdi12_send(const char *cmd)
{
    TX_ON;
    uart_write_bytes(UART_PORT, cmd, strlen(cmd));
    uart_wait_tx_done(UART_PORT, portMAX_DELAY);
    TX_OFF;
}

static int sdi12_read(char *buffer, int maxlen, int timeout_ms)
{
    uart_flush_input(UART_PORT);
    int len = uart_read_bytes(UART_PORT,
                              (uint8_t *)buffer,
                              maxlen - 1,
                              pdMS_TO_TICKS(timeout_ms));
    buffer[len] = '\0';
    return len;
}

/* ================= PUBLIC ================= */
void sdi12_init()
{
    gpio_set_direction(TX_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_direction(BREAK_ENABLE, GPIO_MODE_OUTPUT);
    TX_OFF;
    BREAK_OFF;

    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_7_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_REF_TICK,
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_line_inverse(UART_PORT, UART_SIGNAL_TXD_INV);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, 256, 256, 0, NULL, 0);
}

float readTemperature()
{
    char rx[64];
    int len;

    sdi12_break();
    sdi12_send("0M!\r");
    len = sdi12_read(rx, sizeof(rx), 3000);

    if (len < 2)
        return -999.0f;

    vTaskDelay(pdMS_TO_TICKS(1200));

    sdi12_break();
    sdi12_send("0D0!\r");
    len = sdi12_read(rx, sizeof(rx), 3000);

    if (len < 2 || rx[0] != '0')
        return -999.0f;

    float temperature = 0.0f;
    sscanf(rx + 1, "%f", &temperature);
    return temperature;
}
