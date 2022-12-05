#include <freertos/FreeRTOS.h>
#include <driver/spi_master.h>
#include <global_variables.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <settings.h>
#include <colours.h>
#include <ili9341.h>
#include <string.h>
#include <images.h>
#include <fonts.h>
#include <math.h>

// #include <esp_heap_caps.h>

static uint16_t screenBuffer[SCREEN_PIXELS_SIZE]; // [SCREEN_HEIGHT * SCREEN_WIDTH]

typedef struct TFT_t {
    uint16_t _model;
    uint16_t _width;
    uint16_t _height;
    uint16_t _offsetx;
    uint16_t _offsety;
    uint16_t _font_direction;
    uint16_t _font_fill;
    uint16_t _font_fill_color;
    uint16_t _font_underline;
    uint16_t _font_underline_color;
    int16_t _dc;
    int16_t _bl;
    spi_device_handle_t _SPIHandle;
} TFT_t;

TFT_t dev;

// SPI transmission functions
// ---------------------------

bool spi_master_write_bytes_screen(const uint8_t* data, size_t dataLength)
{
    spi_transaction_t SPITransaction;

    if ( dataLength > 0 ) {
        memset( &SPITransaction, 0, sizeof(spi_transaction_t) );
        SPITransaction.length = dataLength * 8;
        SPITransaction.tx_buffer = data;
        return !spi_device_transmit(dev._SPIHandle, &SPITransaction);
    }
    ERROR("Tried to send 0 bytes to screen!");
    return false;
}

bool sendByte(DataOrCommand doc, uint8_t byte)
{
    // DC pin should by default in DATA mode, and changed to COMMAND only while sending, and then reset!

    if (doc == COMMAND) { 
        gpio_set_level(dev._dc, COMMAND);
        if (spi_master_write_bytes_screen(&byte, 1)) {
            gpio_set_level(dev._dc, DATA);
            return true;
        } else {
            ERROR("Could not write 8 bit command to ILI9341 screen!");
            gpio_set_level(dev._dc, DATA);
            return false;
        }
    } else {
        if (spi_master_write_bytes_screen(&byte, 1)) {
            return true;
        } else {
            ERROR("Could not write 8 bit data to ILI9341 screen!");
            return false;
        }
    }
}

// Utility
// --------

bool setScreenWriteArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    static uint8_t data[4];

    sendByte(COMMAND, ILI9341_COLUMN_ADDR);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    if (!spi_master_write_bytes_screen(data, 4)) {
        ERROR("Could not set column adress on screen");
        return false;
    }

    sendByte(COMMAND, ILI9341_PAGE_ADDR);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    if (!spi_master_write_bytes_screen(data, 4)) {    
        ERROR("Could not set page adress on screen");
        return false;
    }

    sendByte(COMMAND, ILI9341_WRITE_RAM);
    return true;
}

uint16_t reverseBytes(uint16_t num)
{
    return (((num & 0xff00) >> 8) | ((num & 0x00ff) << 8));
}

// Setup
// ------

bool setupScreenIO();

bool setupScreen()
{
    // STATUS("Free size: %i with largest block: %i while trying to malloc %i", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT), SCREEN_BYTES_SIZE);

    // screenBuffer = { 0 };
    //screenBuffer =  malloc(SCREEN_BYTES_SIZE);
    if (screenBuffer == NULL) {
        ERROR("Screen buffer not allocated!");
        return false;
    }
    initFonts();

    setupScreenIO();

    sendByte(COMMAND, ILI9341_POWER1);  //Power Control 1
    sendByte(DATA, 0x23);

    sendByte(COMMAND, ILI9341_POWER2);  //Power Control 2
    sendByte(DATA, 0x10);

    sendByte(COMMAND, ILI9341_VCOM1);  //VCOM Control 1
    sendByte(DATA, 0x3E);
    sendByte(DATA, 0x28);

    sendByte(COMMAND, ILI9341_VCOM2);  //VCOM Control 2
    sendByte(DATA, 0x86);

    sendByte(COMMAND, ILI9341_MEMORY_ACCESS_CONTROL);  //Memory Access Control
    sendByte(DATA, 0x08); // Bottom right start, RGB color filter panel

    sendByte(COMMAND, ILI9341_PIXEL_FORMAT);  //Pixel Format Set
    sendByte(DATA, 0x55);  //65K color: 16-bit/pixel

    sendByte(COMMAND, ILI9341_DISPLAY_INVERSION_OFF);  // Display Inversion OFF

    sendByte(COMMAND, ILI9341_FRAME_RATE_CONTROL);  //Frame Rate Control
    sendByte(DATA, 0x00);
    sendByte(DATA, 0x18);

    sendByte(COMMAND, ILI9341_DISPLAY_FUNCTION_CONTROL);  //Display Function Control
    sendByte(DATA, 0x08);
    sendByte(DATA, 0xA2);  // REV:1 GS:0 SS:0 SM:0
    sendByte(DATA, 0x27);
    sendByte(DATA, 0x00);

    sendByte(COMMAND, ILI9341_SET_GAMMA);  //Gamma Set
    sendByte(DATA, 0x01);

    sendByte(COMMAND, ILI9341_POSITIVE_GAMMA_CORRECTION);  //Positive Gamma Correction
    sendByte(DATA, 0x0F);
    sendByte(DATA, 0x31);
    sendByte(DATA, 0x2B);
    sendByte(DATA, 0x0C);
    sendByte(DATA, 0x0E);
    sendByte(DATA, 0x08);
    sendByte(DATA, 0x4E);
    sendByte(DATA, 0xF1);
    sendByte(DATA, 0x37);
    sendByte(DATA, 0x07);
    sendByte(DATA, 0x10);
    sendByte(DATA, 0x03);
    sendByte(DATA, 0x0E);
    sendByte(DATA, 0x09);
    sendByte(DATA, 0x00);

    sendByte(COMMAND, ILI9341_NEGATIVE_GAMMA_CORRECTION);  //Negative Gamma Correction
    sendByte(DATA, 0x00);
    sendByte(DATA, 0x0E);
    sendByte(DATA, 0x14);
    sendByte(DATA, 0x03);
    sendByte(DATA, 0x11);
    sendByte(DATA, 0x07);
    sendByte(DATA, 0x31);
    sendByte(DATA, 0xC1);
    sendByte(DATA, 0x48);
    sendByte(DATA, 0x08);
    sendByte(DATA, 0x0F);
    sendByte(DATA, 0x0C);
    sendByte(DATA, 0x31);
    sendByte(DATA, 0x36);
    sendByte(DATA, 0x0F);

    sendByte(COMMAND, ILI9341_SLEEP_OUT);  //Sleep Out
    vTaskDelay(milliseconds(120));

    sendByte(COMMAND, ILI9341_DISPLAY_ON);  //Display ON

    LOG_BLUE("DONE WITH SCREEN SETUP!\n");
    return true;
}

bool setupScreenIO()
{
    gpio_config_t gpio_conf;                        // Configuration struct    
    gpio_conf.intr_type = GPIO_INTR_DISABLE;        // Disable interrupt
    gpio_conf.mode = GPIO_MODE_OUTPUT;              // Set as output mode
    gpio_conf.pin_bit_mask = (
        1ULL << SCREEN_CS_PIN | 
        1ULL << SCREEN_RESET_PIN |
        1ULL << SCREEN_DC_PIN
    );                                              // Bit mask of the pins that you want to set, e.g.GPIO18/19
    gpio_conf.pull_down_en = 0;                     // Disable pull-down mode
    gpio_conf.pull_up_en = 0;                       // Disable pull-up mode
    gpio_config(&gpio_conf);

    gpio_set_level(SCREEN_RESET_PIN, 1);
    vTaskDelay(milliseconds(10));

    dev._model = 0x9341;
    dev._width = 240;
    dev._height = 320;
    dev._offsetx = 0;
    dev._offsety = 0;
    dev._font_direction = 0;
    dev._font_fill = false;
    dev._font_underline = false;

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 60000000,     // Was: SPI_MASTER_FREQ_40M --> (40000000)
        .spics_io_num = SCREEN_CS_PIN,
        .queue_size = 7,                // Was 7
        .flags = SPI_DEVICE_NO_DUMMY
    };

    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &dev._SPIHandle));

    dev._dc = SCREEN_DC_PIN;
    dev._bl = SLEEP_PIN;

    STATUS("Done with Screen IO!");

    return true;
}

// Buffer Transmission
// --------------------

bool sendEntireBuffer()
{
    setScreenWriteArea(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);

    uint32_t counter = 0;

    for (uint8_t i = 0; i < MAX_TRANSMISSION_BUFFER_TIMES_TO_SEND; ++i)
    {
        if (!spi_master_write_bytes_screen((uint8_t*) screenBuffer + counter, SCREEN_MAX_TRANSMISSION_BUFFER)) {
            ERROR("Could not send colour buffer to screen");
            return false;
        }
        counter += SCREEN_MAX_TRANSMISSION_BUFFER;
    }

    return true;
}

bool sendBufferArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    setScreenWriteArea(x1, y1, x2, y2);

    uint8_t* bufferArea = malloc((x2 - x1 + 1) * (y2 - y1 + 1) * 2);

    if (bufferArea == NULL) {
        ERROR("Could not allocate temporary buffer area!");
        DEBUG("Tried to allocate %i and largest free block was %i", ((x2 - x1 + 1) * (y2 - y1 + 1) * 2), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        DEBUG("Coordinates were: %i, %i, %i, %i", x1, y1, x2, y2);
        return false;
    }

    for (uint16_t h = 0; h < (y2 - y1); ++h)
    {
        memcpy(bufferArea + ((x2 - x1 + 1) * 2 * h), 
            screenBuffer + ((y1 + h) * SCREEN_WIDTH) + x1, 
            ((x2 - x1) * 2) + 2);
    }


    uint8_t bufferSegments = (uint8_t)(((x2 - x1) * (y2 - y1) * 2) / 4094) + 1;

    for (uint8_t i = 0; i < bufferSegments; ++i)
    {
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / bufferSegments) * i], ((x2 - x1) * (y2 - y1) * 2) / bufferSegments);
    }

    /*

    // Check how many MAX_TRANSMISSION needet to send area    
    if ((x2 - x1) * (y2 - y1) * 2 < 4094) {
        spi_master_write_bytes_screen(bufferArea, (x2 - x1) * (y2 - y1) * 2);
    } else if ((x2 - x1) * (y2 - y1) * 2 < 8188) {
        spi_master_write_bytes_screen(bufferArea, (x2 - x1) * (y2 - y1)); // * 2 / 2 == * 1 
        spi_master_write_bytes_screen(&bufferArea[(x2 - x1) * (y2 - y1)], (x2 - x1) * (y2 - y1)); // * 2 / 2 == * 1 
    } else if ((x2 - x1) * (y2 - y1) * 2 < 12282) {
        spi_master_write_bytes_screen(bufferArea, ((x2 - x1) * (y2 - y1) * 2) / 3); // Send one third of total bytes at the time
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / 3) * 1], ((x2 - x1) * (y2 - y1) * 2) / 3); // Start at one third point
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / 3) * 2], ((x2 - x1) * (y2 - y1) * 2) / 3); // Start at two thirds point
    } else if ((x2 - x1) * (y2 - y1) * 2 < 16376) {
        spi_master_write_bytes_screen(bufferArea, ((x2 - x1) * (y2 - y1) * 2) / 4); // Send one fourth of total bytes at the time
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / 4) * 1], ((x2 - x1) * (y2 - y1) * 2) / 4); // Start at one fourth point
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / 4) * 2], ((x2 - x1) * (y2 - y1) * 2) / 4); // Start at two fourths point
        spi_master_write_bytes_screen(&bufferArea[(((x2 - x1) * (y2 - y1) * 2) / 4) * 3], ((x2 - x1) * (y2 - y1) * 2) / 4); // Start at three fourths point
    } else {
        // Implement later
        ERROR("Buffer area this large not implemented... Was %i", (x2 - x1) * (y2 - y1) * 2);
        return false;
    }*/

    free(bufferArea);
    return true;
}

// Screen display
// ---------------

bool fillEntireBufferWithColour(uint16_t colour)
{
    // Check if colour is black (0x0000) for if so, we can use memset to reset the screen buffer 
    if (colour == 0x0000) {
        memset(screenBuffer, 0, SCREEN_BYTES_SIZE);
    } else {
        uint16_t correctedColour = reverseBytes(colour);
        for (uint8_t i = 0; i < SCREEN_BYTES_MEMCPY_NUMBER; ++i)
        {
            screenBuffer[i] = correctedColour;
        }
        for (uint8_t i = 0; i < SCREEN_BYTES_MEMCPY_EXPONENT; ++i)
        {
            memcpy(&screenBuffer[75 * (uint16_t)pow(2, i)], screenBuffer, 75 * (uint16_t)pow(2, i) * 2);
        }
    }

    return true;    
}

bool fillEntireBufferWithImage(struct Image* image) 
{
    if (image->width != SCREEN_WIDTH || image->height != SCREEN_HEIGHT) {
        ERROR("Image does not fit screen!");
        return false;
    }

    uint32_t counter = 0;

    for (uint16_t h = 0; h < SCREEN_HEIGHT; ++h)
    {
        for(uint16_t w = 0; w < SCREEN_WIDTH; ++w)
        {
            screenBuffer[counter] = reverseBytes(image->data[counter]);
            ++counter;
        }
    }
    return true;
}

bool fillBufferAreaWithColour(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour)
{
     if (x2 > SCREEN_WIDTH || y2 > SCREEN_HEIGHT) {
        ERROR("Area outside screen bounds");
        return false;
    } else if (x1 > x2 || y1 > y2) {
        ERROR("Invalid buffer area");
        return false;
    }

    uint16_t correctedColour = reverseBytes(colour);

    uint32_t counter = (y1 * SCREEN_WIDTH) + x1;

    for (uint16_t h = 0; h < y2 - y1; ++h)
    {
        for (uint16_t w = 0; w < x2 - x1; ++w)
        {
            screenBuffer[counter] = correctedColour;
            ++counter;
        }
        counter += (SCREEN_WIDTH - (x2 - x1));
    };

    // Not implemented yet 
    return true;
}

bool fillBufferAreaWithImage(uint16_t x1, uint16_t y1, struct Image* image)
{
    if (x1 + image->width > SCREEN_WIDTH || y1 + image->height > SCREEN_HEIGHT) {
        ERROR("Image outside screen bounds");
        return false;
    }

    uint32_t counter = (y1 * SCREEN_WIDTH) + x1;

    for (uint16_t h = 0; h < image->height; ++h)
    {
        for (uint16_t w = 0; w < image->width; ++w)
        {
            screenBuffer[counter] = reverseBytes(image->data[(h * image->width) + w]);
            ++counter;
        }
        counter += (SCREEN_WIDTH - image->width);
    }

    return true;
}

bool frameArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t frameThickness, uint16_t frameColour, uint16_t areaColour)
{
    fillBufferAreaWithColour(x1, y1, x2, y2, areaColour);

    uint16_t width = x2 - x1;

    // Top and bottom
    // ---------------

    for (uint16_t w = x1; w < x1 + width; ++w)
    {
        screenBuffer[(SCREEN_WIDTH * y1) + w] = reverseBytes(frameColour);
    }

    // Top
    for (uint16_t h = y1 + 1; h < y1 + frameThickness; ++h)
    {
        memcpy(&screenBuffer[(SCREEN_WIDTH * h) + x1], screenBuffer + (SCREEN_WIDTH * y1) + x1, width * 2);
    }

    // Bottom
    for (uint16_t h = y2 - frameThickness; h < y2; ++h)
    {
        memcpy(&screenBuffer[(SCREEN_WIDTH * h) + x1], screenBuffer + (SCREEN_WIDTH * y1) + x1, width * 2);
    }

    // Left and Right
    // ---------------

    for (uint16_t w = 0; w < frameThickness; ++w)
    {
        screenBuffer[(SCREEN_WIDTH * (y1 + frameThickness)) + x1 + w] = reverseBytes(frameColour);
        screenBuffer[(SCREEN_WIDTH * (y1 + frameThickness)) + x1 + width - w - 1] = reverseBytes(frameColour);
    }

    for (uint16_t h = y1 + frameThickness + 1; h < y2 - frameThickness; ++h)
    {
        memcpy(&screenBuffer[(SCREEN_WIDTH * h) + x1], screenBuffer + (SCREEN_WIDTH * (y1 + frameThickness)) + x1, width * 2);
    }

    return true;
}

bool writeText(char* text, uint8_t spacing, bool normalizedWidth, struct FontxFile *fx, uint16_t x, uint16_t y, uint16_t textColour)
{
    // STATUS("\nWrite text called!\n");

    if (text == NULL) {
        ERROR("writeText called with empty or unintialized string!");
        return false;
    } else if (!fx->opened) {
        if (!openFont(fx)){
            LOG_ERROR("Could not open font %s", fx->fontName);
        }
    }

    // Colour pre-processing (Only black implemented for now)
    
    bool darkText;
    
    darkText = (textColour == BLACK);

    // Get mysterious offset - no idea why it is needed
    uint8_t mysteriousOffset = 18;

    uint16_t charDesiredWidth = 0;
    if (normalizedWidth) {
        charDesiredWidth = fx->numbersNormalizedWidth;
    }

    // Char processing
    // ----------------

    uint16_t textLenght = strlen(text);
    uint16_t textWidth = spacing * (textLenght - 1); // Spacing
    // LOG("Text size: %i", textLenght);

    if (textLenght > WRITE_TEXT_BUFFER) {
        LOG_ERROR("Text size longer than buffer! Increase buffer from %i to at least %i to write thi text.", WRITE_TEXT_BUFFER, textLenght);
    }

    CharInfo chars[WRITE_TEXT_BUFFER];

    for (uint16_t c = 0; c < textLenght; ++c)
    {
        chars[c].ascii = text[c];
        getChar(fx, text[c], &chars[c].xPos, &chars[c].width, &chars[c].xOffset);
        if (normalizedWidth && (chars[c].ascii >= 48 && chars[c].ascii <= 57)) {
            textWidth += fx->numbersNormalizedWidth;
        } else {
            textWidth += chars[c].width;
        }
    }

    // Position processing
    // --------------------

    uint16_t startX = x - (textWidth / 2);

    uint32_t counter = (y * SCREEN_WIDTH) + startX;

    // Plotting into buffer
    // ---------------------

    // FOR DEBUG
    //uint8_t debugCounter = 0; 

    for (uint8_t h = 0; h < fx->fontHeight; ++h)
    {        
        // Add char data to frame row
        for (uint16_t c = 0; c < textLenght; ++c)
        {
            if (normalizedWidth && (chars[c].ascii >= 48 && chars[c].ascii <= 57) && charDesiredWidth - chars[c].width > 0) {
                if ((charDesiredWidth - chars[c].width) % 2 == 1) {
                    counter += ((charDesiredWidth - chars[c].width) / 2) + 1;
                } else {
                    counter += (charDesiredWidth - chars[c].width) / 2;
                }
            }

            for (uint16_t w = 0; w < chars[c].width; ++w)
            {
                uint8_t shade =  fx->charDataPath[(fx->charDataWidth * h) + chars[c].xPos + mysteriousOffset + w];

                if (shade == 0) {
                    // Do nothing
                } else if (shade == 255) {
                    screenBuffer[counter] = reverseBytes(textColour);
                } else {
                    uint16_t colour;

                    // Dark colours shade lighter | Light colours shade darker
                    if (darkText) {
                        colour = 
                        // Red
                        ( ((uint16_t)(((WHITE >> 11) & 0x001F) * (1.0 - (shade / 255.0))) << 11) & 0xF800 ) |
                        // Green
                        ( ((uint16_t)(((WHITE >> 5) & 0x003F) * (1.0 - (shade / 255.0))) << 5) & 0x07E0 ) |
                        // Blue
                        ( ((uint16_t)(((WHITE >> 0) & 0x001F) * (1.0 - (shade / 255.0))) << 0) & 0x001F );
                    } else { 
                        colour = 
                        // Red
                        ( ((uint16_t)(((textColour >> 11) & 0x001F) * (shade / 255.0)) << 11) & 0xF800 ) |
                        // Green
                        ( ((uint16_t)(((textColour >> 5) & 0x003F) * (shade / 255.0)) << 5) & 0x07E0 ) |
                        // Blue
                        ( ((uint16_t)(((textColour >> 0) & 0x001F) * (shade / 255.0)) << 0) & 0x001F );
                    }

                    // Do not use this debug unless you want some serious masochism...
                    /*if (debugCounter < 10) {

                        if (darkText) {
                            DEBUG("Shade %i leads to hex %#04X with %s setting\nFraction: %g\nR: %i --> %g | %i\nG: %i --> %g | %i\nB: %i --> %g | %i\n",
                                shade, colour, (darkText ? "Dark" : "Light"), (1.0 - (shade / 255.0)), 
                                ((textColour >> 11) & 0x001F), (((WHITE >> 11) & 0x001F) * (1.0 - (shade / 255.0))), (uint8_t)(((WHITE >> 11) & 0x001F) * (1.0 - (shade / 255.0))), // R
                                ((textColour >> 5) & 0x003F), (((WHITE >> 5) & 0x003F) * (1.0 - (shade / 255.0))), (uint8_t)(((WHITE >> 5) & 0x003F) * (1.0 - (shade / 255.0))),   // G
                                ((textColour >> 0) & 0x001F), (((WHITE >> 0) & 0x001F) * (1.0 - (shade / 255.0))), (uint8_t)(((WHITE >> 0) & 0x001F) * (1.0 - (shade / 255.0))) );  // B
                        } else {
                            DEBUG("Shade %i leads to hex %#04X with %s setting\nFraction: %g\nR: %i --> %g | %i\nG: %i --> %g | %i\nB: %i --> %g | %i\n",
                                shade, colour, (darkText ? "Dark" : "Light"), (shade / 255.0), 
                                ((textColour >> 11) & 0x001F), (((textColour >> 11) & 0x001F) * (shade / 255.0)), (uint8_t)(((textColour >> 11) & 0x001F) * (shade / 255.0)), // R
                                ((textColour >> 5) & 0x003F), (((textColour >> 5) & 0x003F) * (shade / 255.0)), (uint8_t)(((textColour >> 5) & 0x003F) * (shade / 255.0)),   // G
                                ((textColour >> 0) & 0x001F), (((textColour >> 0) & 0x001F) * (shade / 255.0)), (uint8_t)(((textColour >> 0) & 0x001F) * (shade / 255.0)) );  // B
                        }
                        ++debugCounter;
                    }*/
                    screenBuffer[counter] = reverseBytes(colour); // reverseBits
                }
                ++counter;
            }

            if (normalizedWidth && (chars[c].ascii >= 48 && chars[c].ascii <= 57) && charDesiredWidth - chars[c].width > 0) {
                counter += (charDesiredWidth - chars[c].width) / 2;
            }

            if (c != textLenght - 1) {
                counter += spacing;
            }
        }

        // Text this!
        counter += (SCREEN_WIDTH - textWidth);
    }

    return true;
}

bool loadingBar(uint16_t centerX, uint16_t centerY, intptr_t variable)
{
    //DEBUG("Loading bar called");

    // Hardcoded to black bar for now
    uint16_t colour = reverseBytes(BLACK);
    uint16_t light = reverseBytes(BLACK | 0x8410);

    // Hardcoded size for now
    uint8_t height = 10;
    uint8_t lenght = 160;
    uint8_t XPadding = 5;
    uint8_t YPadding = 4;

    uint8_t angle = height / 3;
    uint8_t midsection = height - (angle * 2);

    // Determine progress based on linked variable
    float progress;

    if (variable == (intptr_t)&brewElapsedTime) {
        progress = (brewElapsedTime / (brewOrder.brewTime * 1000.0));
        //DEBUG("Progress: %g Elapsed: %i Total: %i", progress, brewElapsedTime, (brewOrder.brewTime * 1000));
    } else {
        ERROR("Invalid variable link in loadingBar");
        return false;
    }
    uint8_t pixelProgress = ((lenght * progress) - (angle * 2)) <= 1 ? 2 : (lenght * progress) - (angle * 2);

    uint16_t topLeftX = centerX - (uint16_t)(loadingBarBackground.width / 2);
    uint16_t topLeftY = centerY - (uint16_t)(loadingBarBackground.height / 2);

    // Fill buffer with bar background
    fillBufferAreaWithImage(topLeftX, topLeftY, &loadingBarBackground);

    // Plot bar on buffer
    // -------------------

    // Top section
    for (uint8_t h = 0; h < angle; ++h)
    {
        if (h == angle / 2) {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle - h - 1] = colour;
        } else {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle - h - 1] = light;
        }

        for (uint8_t l = angle - h; l < angle + pixelProgress + h; ++l)
        {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + l] = colour;            
        }                             

        if (h == angle / 2) {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle + pixelProgress + h] = colour;       // Bytes flipped on | 0xXXXX number
        } else {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle + pixelProgress + h] = light;       // Bytes flipped on | 0xXXXX number
        }
    }

    // DEBUG("Done with top section");

    // Middle section
    for (uint8_t h = angle; h < midsection + angle; ++h)
    {
        for (uint8_t l = 0; l < pixelProgress + (angle * 2); ++l)
        {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + l] = colour;
        }
    }

    //DEBUG("Done with Midddle section");

    // Bottom section
    for (uint8_t h = midsection + angle; h < (angle * 2) + midsection; ++h)
    {
        if (h == angle + midsection + (angle / 2)) {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle - (height - h)] = colour;   // Bytes flipped on | 0xXXXX number
        } else {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle - (height - h)] = light;   // Bytes flipped on | 0xXXXX number
        }

        for (uint8_t l = angle - (height- 1 - h); l < angle + pixelProgress + (height - 1 - h); ++l)
        {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + l] = colour;
        }

        if (h == angle + midsection + (angle / 2)) {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle + pixelProgress + (height - h - 1)] = colour;    // Bytes flipped on | 0xXXXX number
        } else {
            screenBuffer[((topLeftY + YPadding + h) * SCREEN_WIDTH) + topLeftX + XPadding + angle + pixelProgress + (height - h - 1)] = light;  // Bytes flipped on | 0xXXXX number
        }
    }

    // DEBUG("Done with Bottom section");

    // Send buffer area
    if (!sendBufferArea(topLeftX, topLeftY, topLeftX + loadingBarBackground.width, topLeftY + loadingBarBackground.height)) {
        ERROR("Could not send buffer to screen");
        return false;
    }
    return true;
}

bool brewingAnimation(uint16_t centerX, uint16_t centerY, uint8_t stage)
{
    // uint8_t y_drop_limit;

    // How many stages? (Frame)

    // Calculate where to place drop based on stage

    // Load static machine and cup image

    return true;
}

bool processLoadingCircle(uint16_t centerX, uint16_t centerY, intptr_t variable)
{
    ERROR("processLoadingCircle() not implemented!");

    // Diameter 120

    /*

            •  •              
         •        •        
        •          •      
        •          •      
         •        •        
            •  •  

    */

    return false;
}

bool barAdjuster(uint16_t centerX, uint16_t centerY, intptr_t variable)
{
    uint8_t numberOfBars;
    uint8_t activeBars;
    uint8_t barWidth;
    uint8_t barHeight;
    uint8_t barSpacing;
    uint8_t topY;
    uint8_t leftX;

    if (variable == getCalibrationValue(BUTTONS)) {
        numberOfBars = 8;
        activeBars = getCalibrationValue(BUTTONS) + 1;
        barWidth = 100;
        barHeight = 8;
        barSpacing = 12;
    } else {
        ERROR("Invalid or unimplemented variable in barAdjuster");
        return false;
    }

    leftX = centerX - (barWidth / 2);
    topY = centerY - (((numberOfBars * barHeight) + (barSpacing * (numberOfBars - 1))) / 2);

    for (uint8_t b = 0; b < numberOfBars; ++b)
    {
        if (numberOfBars - b <= activeBars) {
            fillBufferAreaWithColour(leftX, topY + (b * (barHeight + barSpacing)), leftX + barWidth, topY + (b * (barHeight + barSpacing)) + barHeight, WHITE);
        } else {
            fillBufferAreaWithColour(leftX, topY + (b * (barHeight + barSpacing)), leftX + barWidth, topY + (b * (barHeight + barSpacing)) + barHeight, BLACK);
        }
    }

    // DEBUG("Variables:\n\t* leftX: %i\n\t* topY: %i\n\t* bottomY: %i\n\t* bottomY: %i", leftX, topY, leftX + barWidth, topY + ((numberOfBars * barHeight) + (barSpacing * (numberOfBars - 1))));

    return sendBufferArea(leftX, topY, leftX + barWidth, topY + ((numberOfBars * barHeight) + (barSpacing * (numberOfBars - 1))));
}


// ----------------------------------------------
//                    OLD CODE
// ----------------------------------------------

/*bool sendBytes(TFT_t* dev, DataOrCommand doc, uint8_t* buffer, uint16_t size)
{
    // DC pin should by default in DATA mode, and changed to COMMAND only while sending, and then reset!

    if (doc == COMMAND) { 
        gpio_set_level(dev->_dc, COMMAND)
        if (spi_master_write_byte( dev->_SPIHandle, buffer, size)) {
            gpio_set_level(dev->_dc, DATA)
            return true;
        } else {
            ERROR("Could not write 8 bit data to ILI9341 screen!");
            gpio_set_level(dev->_dc, DATA);
            return false;
        }
    } else {
        if (spi_master_write_byte( dev->_SPIHandle, buffer, size)) {
            return true;
        } else {
            ERROR("Could not write 8 bit data to ILI9341 screen!");
            return false
        }
    }
}*/


/*bool send16bits(TFT_t* dev, DataOrCommand doc, uint16_t* buffer, uint16_t size)
{
    // DC pin should by default in DATA mode, and changed to COMMAND only while sending, and then reset!

    if (doc == COMMAND) { 
        gpio_set_level(dev->_dc, COMMAND)
            // UNREVERSED BUFFER !!!
        if (spi_master_write_byte( dev->_SPIHandle, buffer, size)) {
            gpio_set_level(dev->_dc, DATA)
            return true;
        } else {
            ERROR("Could not write 8 bit data to ILI9341 screen!");
            gpio_set_level(dev->_dc, DATA);
            return false;
        }
    } else {
            // UNREVERSED BUFFER !!!
        if (spi_master_write_byte( dev->_SPIHandle, buffer, size)) {
            return true;
        } else {
            ERROR("Could not write 8 bit data to ILI9341 screen!");
            return false
        }
    }
}*/