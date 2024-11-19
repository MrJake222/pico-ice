/*
 * MIT License
 *
 * Copyright (c) 2023 tinyVision.ai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// pico-sdk
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

// pico-ice-sdk
#include "ice_usb.h"
#include "ice_fpga.h"
#include "ice_led.h"

// local
#include "local/dhry_rp2040.h"

// GP0  UART0Tx  ICE27
// GP1  UART0Rx  ICE25
// GP2  UART0CTS ICE21
#define UART0_TX_PIN    0
#define UART0_RX_PIN    1
#define UART0_CTS_PIN   2

// GP4  UART1Tx  ICE26
// GP5  UART1Rx  ICE23
// GP6  UART1CTS ICE20
#define UART1_TX_PIN    4
#define UART1_RX_PIN    5
#define UART1_CTS_PIN   6

#define DOC_FORWARD_SPI \
"https://pico-ice.tinyvision.ai/group__ice__usb.html#autotoc_md2"

#define DOC_DEFAULT_FIRMWARE \
"https://github.com/tinyvision-ai-inc/pico-ice/tree/main/Firmware/pico-ice-default"

#define DOC_MRJAKE_FIRMWARE \
"https://github.com/MrJake222/pico-ice"

static int repl_getchar(void)
{
    int c;

    c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
        return c;

    if (c == '\r' || c == '\n') {
        printf("\r\n");
    } else {
        putchar(c);
    }
    return c;
}

static void repl_command_version(void)
{
    printf("pico-ice-sdk %s\r\n", VERSION);
}

static void repl_prompt(void)
{
    // in bold
    printf("\x1b[1mpico-ice>\x1b[m ");
}

int main(void)
{

#if ICE_USB_UART0_CDC
    // Enable the physical UART
    uart_init(uart0, 115200);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_CTS_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(uart0, true, false);
#endif

#if ICE_USB_UART1_CDC
    // Enable the physical UART
    uart_init(uart1, 115200);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_CTS_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(uart1, true, false);
#endif

    // Let the FPGA start
    ice_fpga_init(12);
    ice_fpga_start();

    // Configure USB as defined in tusb_config.h
    ice_usb_init();

    // Enable USB-CDC #0 (serial console)
    stdio_usb_init();

    // Prevent the LEDs from glowing slightly
    ice_led_init();

    // Print repl prompt
    repl_prompt();

    while (true) {
        ice_usb_task();

        int chr = repl_getchar();
        if (chr == PICO_ERROR_TIMEOUT)
            continue;

        // not timeout, something received
        printf("\r\n");

        switch (chr) {
        case 'v':
            repl_command_version();
            break;
        case 'd':
            run_dhrystone_rp2040();
            break;
        default:
            printf("pico-ice MrJake222 firmware\r\n");
            printf("   %s\r\n", DOC_MRJAKE_FIRMWARE);
            printf("\r\n");
            printf("Serial port #0 - this shell, with commands:\r\n");
            printf("  v - print pico-ice-sdk version\r\n");
            printf("  d - perform dhrystone benchmark\r\n");
            printf("\r\n");
#ifdef ICE_USB_UART0_CDC
            printf("Serial port #%d - forwarding to UART0\r\n", ICE_USB_UART0_CDC);
            printf("  UART TX  on RP0 = ICE27\r\n");
            printf("  UART RX  on RP1 = ICE25\r\n");
            printf("  UART CTS on RP2 = ICE21\r\n");
            printf("\r\n");
#endif
#ifdef ICE_USB_UART1_CDC
            printf("Serial port #%d - forwarding to UART1\r\n", ICE_USB_UART1_CDC);
            printf("  UART TX  on RP4 = ICE26\r\n");
            printf("  UART RX  on RP5 = ICE23\r\n");
            printf("  UART CTS on RP6 = ICE20\r\n");
            printf("\r\n");
#endif
#ifdef ICE_USB_SPI_CDC
            printf("Serial port #%d - forwarding to SPI:\r\n", ICE_USB_SPI_CDC);
            printf("  %s\r\n", DOC_FORWARD_SPI);
            printf("\r\n");
#endif
            break;
        }

        // reprint prompt
        repl_prompt();
    }

    return 0;
}
