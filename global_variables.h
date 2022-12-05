#ifndef __GLOBAL_VARIABLES_H__
#define __GLOBAL_VARIABLES_H__

#include <pinmap.h>

// Utility
// --------
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

#define UNUSED(p) ((void)(p))
#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))
#define milliseconds(x) (x / portTICK_PERIOD_MS)

#define GLOBAL_ERROR_MESSAGES
#define GLOBAL_DEBUG_MESSAGES
#define GLOBAL_STATUS_MESSAGES

#ifdef GLOBAL_ERROR_MESSAGES
#define ERROR(x, ...) printf(ANSI_RED x ANSI_RESET "\n", ##__VA_ARGS__)
#else
#define ERROR(x, ...)
#endif

#ifdef GLOBAL_DEBUG_MESSAGES
#define DEBUG(x, ...) printf(ANSI_YELLOW x ANSI_RESET "\n", ##__VA_ARGS__)
#else 
#define DEBUG(x, ...)
#endif

#ifdef GLOBAL_ERROR_MESSAGES
#define STATUS(x, ...) printf(ANSI_CYAN x ANSI_RESET "\n", ##__VA_ARGS__)
#else
#define STATUS(x, ...)
#endif

// No-components implementation >>> For early testing
// #define NO_BUTTON_DRIVER
// #define NO_WATER_LEVEL_SENSOR
#define NO_WEIGHT_SENSOR
#define NO_THERMOMETER
#define NO_TOF
// #define NO_SCREEN

// Whether or not to create a log describing the brew, to be printed when the brew is finished
#define LOG_BREW
#define LOG_RESOLUTION 	250		// ms
#define LOG_MAX_TIME	5 * 60 // min in seconds

typedef struct Brew {
	uint8_t version;
	uint8_t brewType;
	uint16_t coffee;
	uint16_t coffeeToWater;
	uint16_t grind;
	uint16_t temperature;
	uint16_t bloomTime;
	uint16_t bloomWater;
	uint16_t brewTime;
} Brew;

typedef struct Preset {
	struct Brew brew;
	char name[24];
	bool empty;
} Preset;

typedef enum BrewStage {
	NOT_STARTED,
	STARTING,
	INSERT_WATER,				
	ADJUSTING_TEMPERATURE,
	BLOOMING,				
	POURING,				
	DRAINING,				
	FINISHED,				

	// Not for use
	BREW_STAGE_LAST
} BrewStage;


// Definitions
// ------------

// Brew

#define MIN_COFFEE_VALUE 10
#define MAX_COFFEE_VALUE 55

#define MIN_COFFEE_TO_WATER_RATIO 55
#define MAX_COFFEE_TO_WATER_RATIO 75

#define MIN_BLOOM_TIME 20
#define MAX_BLOOM_TIME 45

#define MIN_BLOOM_WATER 25
#define MAX_BLOOM_WATER 120

#define MIN_BREW_TEMPERATURE 85
#define MAX_BREW_TEMPERATURE 98

#define MIN_BREW_TIME 90
#define MAX_BREW_TIME 240

// Settings

#define BUTTON_SENSITIVITY_MIN 0
#define BUTTON_SENSITIVITY_MAX 7


// Variables
// ----------
volatile bool brewOrderRecieved;
volatile Brew brewOrder;

volatile Preset brewPresetI;
volatile Preset brewPresetII;
volatile Preset brewPresetIII;
volatile Preset brewPresetIV;
volatile Preset brewPresetV;

volatile uint8_t brewStage;
volatile uint32_t brewElapsedTime;

volatile float containerTemperature;
volatile int32_t funnelWaterLevel;

volatile bool leftButtonPressed;
volatile bool middleButtonPressed;
volatile bool rightButtonPressed;

// Fake variables
// ---------------

volatile float fakeTemperature;
volatile float fakeWeight;
volatile uint8_t fakeLoadingValue;

#endif  /* __GLOBAL_VARIABLES__ */