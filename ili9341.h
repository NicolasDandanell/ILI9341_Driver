#ifndef __ILI9341_H__
#define __ILI9341_H__

#define SCREEN_WIDTH 	240
#define SCREEN_HEIGHT	320
#define SCREEN_PIXELS_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_BYTES_SIZE (SCREEN_PIXELS_SIZE * 2)

// 75 * 2^10 = 76.800 :)
#define SCREEN_BYTES_MEMCPY_NUMBER 75
#define SCREEN_BYTES_MEMCPY_EXPONENT 10

// // We can send 8 rows at the time (Max SPI transaction size 4094 Bytes)
#define SCREEN_MAX_TRANSMISSION_BUFFER (SCREEN_WIDTH * (SCREEN_HEIGHT / 40) * 2)
#define MAX_TRANSMISSION_BUFFER_TIMES_TO_SEND 40

typedef enum DataOrCommand {
	COMMAND = 0,
	DATA 	= 1
} DataOrCommand;

struct FontxFile;
struct Image;

bool setupScreen();

bool fillEntireBufferWithColour(uint16_t colour);
bool fillEntireBufferWithImage(struct Image* image);

bool fillBufferAreaWithColour(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour);
bool fillBufferAreaWithImage(uint16_t x1, uint16_t y1, struct Image* image);

bool frameArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t frameThickness, uint16_t frameColour, uint16_t areaColour);

bool writeText(char* text, uint8_t spacing, bool normalizedWidth, struct FontxFile* fx, uint16_t x, uint16_t y, uint16_t textColour);

// Graphic functions
bool loadingBar(uint16_t centerX, uint16_t centerY, intptr_t variable);
bool brewingAnimation(uint16_t centerX, uint16_t centerY, uint8_t stage);
bool processLoadingCircle(uint16_t centerX, uint16_t centerY, intptr_t variable);
bool barAdjuster(uint16_t centerX, uint16_t centerY, intptr_t variable);

bool sendEntireBuffer();
bool sendBufferArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#define ILI9341_NOP                                         0x00
#define ILI9341_RESET                                       0x01
#define ILI9341_READ_DISPLAY_IDENTIFICATION_INFORMATION		0x04
#define ILI9341_READ_DISPLAY_STATUS                         0x09
#define ILI9341_READ_DISPLAY_POWER_MODE                     0x0A
#define ILI9341_READ_DISPLAY_MADCTL                         0x0B
#define ILI9341_READ_DISPLAY_PIXEL_FORMAT                   0x0C
#define ILI9341_READ_DISPLAY_IMAGE_FORMAT                   0x0D
#define ILI9341_READ_DISPLAY_SIGNAL_MODE                    0x0E
#define ILI9341_READ_DISPLAY_SELF_DIAGNOSTIC_RESULT         0x0F
#define ILI9341_ENTER_SLEEP_MODE                            0x10
#define ILI9341_SLEEP_OUT                                   0x11
#define ILI9341_PARTIAL_MODE_ON                             0x12
#define ILI9341_NORMAL_DISPLAY_MODE_ON                      0x13
#define ILI9341_DISPLAY_INVERSION_OFF                       0x20
#define ILI9341_DISPLAY_INVERSION_ON                        0x21
#define ILI9341_SET_GAMMA                                   0x26
#define ILI9341_DISPLAY_OFF                                 0x28
#define ILI9341_DISPLAY_ON                                  0x29
#define ILI9341_COLUMN_ADDR                                 0x2A
#define ILI9341_PAGE_ADDR                                   0x2B
#define ILI9341_WRITE_RAM                                   0x2C
#define ILI9341_COLOR_SET                                   0x2D
#define ILI9341_MEMORY_READ                                 0x2E
#define ILI9341_PARTIAL_AREA                                0x30
#define ILI9341_VERTICAL_SCROLLING_DEFINITION               0x33
#define ILI9341_TEARING_EFFECT_LINE_OFF                     0x34
#define ILI9341_TEARING_EFFECT_LINE_ON                      0x35
#define ILI9341_MEMORY_ACCESS_CONTROL                       0x36
#define ILI9341_VERTICAL_SCROLLING_START_ADDRESS            0x37
#define ILI9341_IDLE_MODE_OFF                               0x38
#define ILI9341_IDLE_MODE_ON                                0x39
#define ILI9341_PIXEL_FORMAT                                0x3A
#define ILI9341_WMC                                         0x3C
#define ILI9341_RMC                                         0x3E
#define ILI9341_SET_TEAR_SCANLINE                           0x44
#define ILI9341_WDB                                         0x51
#define ILI9341_READ_DISPLAY_BRIGHTNESS                     0x52
#define ILI9341_WCD                                         0x53
#define ILI9341_READ_CTRL_DISPLAY                           0x54
#define ILI9341_WCABC                                       0x55
#define ILI9341_RCABC                                       0x56
#define ILI9341_WCABCMB                                     0x5E
#define ILI9341_RCABCMB                                     0x5F
#define ILI9341_RGB_INTERFACE                               0xB0
#define ILI9341_FRAME_RATE_CONTROL							0xB1
#define ILI9341_FRAME_CTRL_NM                               0xB2
#define ILI9341_FRAME_CTRL_IM                               0xB3
#define ILI9341_FRAME_CTRL_PM                               0xB4
#define ILI9341_BPC                                         0xB5
#define ILI9341_DISPLAY_FUNCTION_CONTROL                    0xB6
#define ILI9341_ENTRY_MODE_SET                              0xB7
#define ILI9341_BACKLIGHT_CONTROL_1                         0xB8
#define ILI9341_BACKLIGHT_CONTROL_2                         0xB9
#define ILI9341_BACKLIGHT_CONTROL_3                         0xBA
#define ILI9341_BACKLIGHT_CONTROL_4                         0xBB
#define ILI9341_BACKLIGHT_CONTROL_5                         0xBC
#define ILI9341_BACKLIGHT_CONTROL_6                         0xBD
#define ILI9341_BACKLIGHT_CONTROL_7                         0xBE
#define ILI9341_BACKLIGHT_CONTROL_8                         0xBF
#define ILI9341_POWER1                                      0xC0
#define ILI9341_POWER2                                      0xC1
#define ILI9341_VCOM1                                       0xC5
#define ILI9341_VCOM2                                       0xC7
#define ILI9341_POWERA                                      0xCB
#define ILI9341_POWERB                                      0xCF
#define ILI9341_READ_ID1                                    0xDA
#define ILI9341_READ_ID2                                    0xDB
#define ILI9341_READ_ID3                                    0xDC
#define ILI9341_POSITIVE_GAMMA_CORRECTION                   0xE0
#define ILI9341_NEGATIVE_GAMMA_CORRECTION                   0xE1
#define ILI9341_DTCA                                        0xE8
#define ILI9341_DTCB                                        0xEA
#define ILI9341_POWER_SEQ                                   0xED
#define ILI9341_3GAMMA_EN                                   0xF2
#define ILI9341_INTERFACE                                   0xF6
#define ILI9341_PRC                                         0xF7

#endif  /* __ILI9341__ */