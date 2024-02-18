#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "font.h"

#define HEIGHT 32
#define WIDTH 128

#define I2C_ADDR _u(0x3C)

#define I2C_CLOCK 400

#define SET_MEM_MODE _u(0x20)
#define SET_COL_ADDR _u(0x21)
#define SET_PAGE_ADDR _u(0x22)
#define SET_X_SCROLL _u(0x26)
#define SET_Y_SCROLL _u(0x2e)

#define SET_START_LINE _u(0x40)

#define SET_CONTRAST _u(0x81)
#define SET_CHARGE_PUMP _u(0x8d)

#define SET_REG_REMAP _u(0xa0)
#define SET_ENTIRE_ON _u(0xa4)
#define SET_ALL_ON _u(0xa5)
#define SET_NORM_DISP _u(0xa6)
#define SET_INV_DISP _u(0xa7)
#define SET_MUX_RATIO _u(0xa8)
#define SET_DISP _u(0xae)
#define SET_COM_OUT_DIR _u(0xc0)

#define SET_DISP_OFFSET _u(0xd3)
#define SET_DISP_CLK_DIV _u(0xd5)
#define SET_PRECHARGE _u(0xd9)
#define SET_COM_PIN_CFG _u(0xda)
#define SET_VCOM_DESEL _u(0xdb)

#define PAGE_HEIGHT _u(0x08)
#define NUM_PAGES (HEIGHT / PAGE_HEIGHT)
#define BUF_LEN (NUM_PAGES * WIDTH)

#define WRITE_MODE _u(0xfe)
#define READ_MODE _u(0xff)

struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};

void send_cmd(uint8_t cmd) {
    uint8_t buffer[2] = {0x80, cmd};

    i2c_write_blocking(i2c_default, I2C_ADDR, buffer, 2, false);
}

void send_commands(uint8_t *commands, int len) {
    for(int i = 0; i < len; i++) send_cmd(commands[i]);
}

void init_screen(void) {
    uint8_t commands[] = {
        SET_DISP,
        SET_MEM_MODE,
        0x00,
        SET_START_LINE,
        SET_REG_REMAP | 0x01,
        SET_MUX_RATIO,
        HEIGHT - 1,
        SET_COM_OUT_DIR | 0x08,
        SET_DISP_OFFSET,
        0x00,
        SET_COM_PIN_CFG,
        0x02,
        0x12,
        0x02,
        SET_DISP_CLK_DIV,
        0x80,
        SET_PRECHARGE,
        0xf1,
        SET_VCOM_DESEL,
        0x30,
        SET_CONTRAST,
        0xff,
        SET_ENTIRE_ON,
        SET_NORM_DISP,
        SET_CHARGE_PUMP,
        0x14,
        SET_Y_SCROLL,
        SET_DISP | 0x01,
    };

    send_commands(commands, (sizeof(commands)/sizeof((commands)[0])));
}

void send_buf(uint8_t buf[], int buflen) {
    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(i2c_default, I2C_ADDR, temp_buf, buflen + 1, false);
    free(temp_buf);
}

void render(uint8_t *buf, struct render_area *area) {
    uint8_t cmds[] = {
        SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    send_commands(cmds, count_of(cmds));
    send_buf(buf, area->buflen);
}

static uint8_t cache[sizeof(font)] = {0};

void calc_render_area_buflen(struct render_area *area) {
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

static void FillCache() {
    for (int i=0;i<sizeof(font);i++)
        cache[i] = font[i];
}

static inline int GetFontIndex(uint8_t ch) {
    if (ch >= 'a' && ch <='z') {
        return  ch - 'a' + 1;
    }
    else if (ch >= '0' && ch <='9') {
        return  ch - '0' + 27;
    } else if (ch >= '!' && ch <= '@') {
        return ch - '!' + 37;
    }
    else return  0; // Not got that char so space.
}

static void WriteChar(uint8_t *buf, int16_t x, int16_t y, uint8_t ch) {
    if (cache[0] == 0) 
        FillCache();

    if (x > WIDTH - 6 || y > HEIGHT - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y/8;

    int idx = GetFontIndex(ch);
    int fb_idx = y * 128 + x;

    for (int i=0;i<6;i++) {
        buf[fb_idx++] = cache[idx * 6 + i];
    }
}

static void WriteString(uint8_t *buf, int16_t x, int16_t y, char *str) {
    // Cull out any string off the screen
    if (x > WIDTH - 6 || y > HEIGHT - 8)
        return;

    while (*str) {
        WriteChar(buf, x, y, *str++);
        x+=6;
    }
}

void init_display(void) {
    stdio_init_all();
    i2c_init(i2c_default, I2C_CLOCK * 1000);

    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    init_screen();
}

void render_display(char *str) {
    struct render_area area = {
        start_col: 0,
        end_col : WIDTH - 1,
        start_page : 0,
        end_page : NUM_PAGES - 1
    };

    calc_render_area_buflen(&area);
    uint8_t buf[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    render(buf, &area);

    WriteString(buf, 2, 2, str);
    render(buf, &area);
}
