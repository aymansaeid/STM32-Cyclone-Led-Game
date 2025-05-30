#include "game.h"
#include "ws2812.h" // ws2812 LED Functions
#include <stdlib.h> // abs()

#define NUM_LEDS noOfLEDs
#define CENTER_LED 29
#define BRIGHTNESS 100

#define LIFE_LED_1_PIN GPIO_PIN_0 // 1st life
#define LIFE_LED_2_PIN GPIO_PIN_1 // 2nd life
#define LIFE_LED_3_PIN GPIO_PIN_7 // 3rd life

// Levels
#define EASY 1
#define MEDIUM 2
#define HARD 3
#define ON_SPEED 4
#define SONIC_SPEED 5
#define ROCKET_SPEED 6
#define LIGHT_SPEED 7
#define MISSION_IMPOSSIBLE 8

#define BUTTON_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_0
#define STABLE_RELEASE_DURATION_MS 100

// RGB Struct
typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RGBColor;

static volatile bool Playing = true; // playing cycle (green led rotates around the circle)
static volatile bool CycleEnded = true; // handle win - loss situations

static int difficulty = EASY;
static bool wonThisRound = false;
static int LEDaddress = 0; // current led
static int player_lives = 3; // lives

// Variables for button interrupt
static uint32_t button_seen_released_at_tick = 0;
static bool waiting_for_stable_release = false;

// Variables for game animations
static RGBColor led_strip[NUM_LEDS];
static uint8_t global_brightness = BRIGHTNESS;

// Functions for only game.c
static void update_led_strip_to_physical_leds(void);
static void play_cylon_animation(void);
static void play_flash_animation(void);
static void update_life_leds(void);
static void increase_game_difficulty(void);
static int getTime(int diff_level);
static void fill_strip_solid(uint8_t r, uint8_t g, uint8_t b);
static void set_led_color_in_strip(int index, uint8_t r, uint8_t g, uint8_t b);
static void play_won_animation(void);

// Public functions
void Game_Setup(void) {
	fill_strip_solid(0, 0, 0);
	update_led_strip_to_physical_leds();
	update_life_leds();
	LEDaddress = 0;
}

// Change status of the game
void Game_ButtonPressCallback(void) {
	if (Playing) {
		Playing = false;
		CycleEnded = true;
	}
}

void Game_Loop(void) {
	if (!Playing) {
		if (CycleEnded) {
			// Turn off all LEDs except the target LED and the selected LED
			for (int i = 0; i < NUM_LEDS; i++) {
				set_led_color_in_strip(i, 0, 0, 0); // Black
			}
			set_led_color_in_strip(CENTER_LED, 255, 0, 0); // Red Target LED
			set_led_color_in_strip(LEDaddress, 0, 255, 0); // Green Selected LED
			update_led_strip_to_physical_leds();

			int diff = abs(CENTER_LED - LEDaddress);
			if (diff == 0) { // WIN
				wonThisRound = true;
				if (difficulty == MISSION_IMPOSSIBLE) {
					play_won_animation();
					for (int i = 0; i < 2; i++) {
						play_cylon_animation();
					}
					difficulty = EASY;
					player_lives = 3;
					update_life_leds();
				} else {
					for (int i = 0; i < 2; i++) {
						play_cylon_animation();
					}
				}
				increase_game_difficulty();
				wonThisRound = false;
			} else { // LOSE
				player_lives--;
				update_life_leds();
				if (player_lives > 0) {
					HAL_Delay(1000);
					for (int i = 0; i < 2; i++) {
						play_flash_animation();
					}
				} else {
					HAL_Delay(1000);
					for (int i = 0; i < 4; i++) {
						play_flash_animation();
					}
					player_lives = 3;
					update_life_leds();
					difficulty = EASY;
				}
			}
			CycleEnded = false;
			waiting_for_stable_release = false;
		}

		GPIO_PinState current_button_state = HAL_GPIO_ReadPin(BUTTON_GPIO_Port,
				BUTTON_Pin);
		if (current_button_state == GPIO_PIN_RESET) { // Button released
			if (!waiting_for_stable_release) {
				waiting_for_stable_release = true;
				button_seen_released_at_tick = HAL_GetTick();
			} else {
				if (HAL_GetTick()
						- button_seen_released_at_tick> STABLE_RELEASE_DURATION_MS) {
					Playing = true;
					LEDaddress = 0;
					HAL_Delay(10);
					fill_strip_solid(0, 0, 0);
					set_led_color_in_strip(CENTER_LED, 255, 0, 0);
					update_led_strip_to_physical_leds();
					HAL_Delay(10);
					waiting_for_stable_release = false;
				}
			}
		} else { // Button pressed
			if (waiting_for_stable_release) {
				waiting_for_stable_release = false;
			}
		}
	}

	if (Playing) {
		for (int i = 0; i < NUM_LEDS; i++) {
			set_led_color_in_strip(i, 0, 0, 0);
		}
		set_led_color_in_strip(CENTER_LED, 255, 0, 0);
		set_led_color_in_strip(LEDaddress, 0, 255, 0);
		update_led_strip_to_physical_leds();

		LEDaddress++;
		if (LEDaddress == NUM_LEDS) {
			LEDaddress = 0;
		}
		HAL_Delay(getTime(difficulty));
	}
}

// Set color of the led for given index
static void set_led_color_in_strip(int index, uint8_t r, uint8_t g, uint8_t b) {
	if (index >= 0 && index < NUM_LEDS) {
		led_strip[index].r = r;
		led_strip[index].g = g;
		led_strip[index].b = b;
	}
}

// Set all LEDs to given RGB color
static void fill_strip_solid(uint8_t r, uint8_t g, uint8_t b) {
	for (int i = 0; i < NUM_LEDS; i++) {
		set_led_color_in_strip(i, r, g, b);
	}
}

// Set physical led strip to given led_strip array with brightness
static void update_led_strip_to_physical_leds(void) {
	for (int i = 0; i < NUM_LEDS; i++) {
		uint8_t r_adj = ((uint16_t) led_strip[i].r * global_brightness) / 255;
		uint8_t g_adj = ((uint16_t) led_strip[i].g * global_brightness) / 255;
		uint8_t b_adj = ((uint16_t) led_strip[i].b * global_brightness) / 255;
		WS2812_SetLED(i, r_adj, g_adj, b_adj);
	}
	WS2812_Send();
}

// Get levels of the game
static int getTime(int diff_level) {
	int timeValue = 0;
	switch (diff_level) {
	case EASY:
		timeValue = 200;
		break;
	case MEDIUM:
		timeValue = 150;
		break;
	case HARD:
		timeValue = 100;
		break;
	case ON_SPEED:
		timeValue = 80;
		break;
	case SONIC_SPEED:
		timeValue = 60;
		break;
	case ROCKET_SPEED:
		timeValue = 50;
		break;
	case LIGHT_SPEED:
		timeValue = 30;
		break;
	case MISSION_IMPOSSIBLE:
		timeValue = 20;
		break;
	default:
		timeValue = 100;
	}
	return timeValue;
}

// Increase game difficulty
static void increase_game_difficulty() {
	if (wonThisRound) {
		if (difficulty < MISSION_IMPOSSIBLE) {
			difficulty++;
		} else {
			difficulty = 1; // Restart the game
		}
	}
}

// Update life leds
static void update_life_leds() {
	HAL_GPIO_WritePin(GPIOB, LIFE_LED_3_PIN,
			(player_lives >= 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LIFE_LED_2_PIN,
			(player_lives >= 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LIFE_LED_1_PIN,
			(player_lives >= 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// -------- Animation Functions --------

// Fade animation
static void apply_fade_to_all_leds() {
	uint8_t fade_amount = 245;
	for (int i = 0; i < NUM_LEDS; i++) {
		led_strip[i].r = (uint32_t) led_strip[i].r * fade_amount / 256;
		led_strip[i].g = (uint32_t) led_strip[i].g * fade_amount / 256;
		led_strip[i].b = (uint32_t) led_strip[i].b * fade_amount / 256;
	}
}

// Helper function for Cylon animation to track current color (Rainbow animation)
static void rgb_color_wheel(uint8_t wheel_pos, RGBColor *color) {
	wheel_pos = 255 - wheel_pos;
	if (wheel_pos < 85) {
		color->r = 255 - wheel_pos * 3;
		color->g = 0;
		color->b = wheel_pos * 3;
	} else if (wheel_pos < 170) {
		wheel_pos -= 85;
		color->r = 0;
		color->g = wheel_pos * 3;
		color->b = 255 - wheel_pos * 3;
	} else {
		wheel_pos -= 170;
		color->r = wheel_pos * 3;
		color->g = 255 - wheel_pos * 3;
		color->b = 0;
	}
}

// Lose animation (blink red)
static void play_flash_animation() {
	fill_strip_solid(255, 0, 0);
	update_led_strip_to_physical_leds();
	HAL_Delay(300);
	fill_strip_solid(0, 0, 0);
	update_led_strip_to_physical_leds();
	HAL_Delay(300);
}

// Win animation
static void play_cylon_animation() {
	static uint8_t wheel_position = 0;
	uint16_t step_delay = 10;
	// Forward
	for (int i = 0; i < NUM_LEDS; i++) {
		RGBColor current_color;
		rgb_color_wheel(wheel_position++, &current_color);
		set_led_color_in_strip(i, current_color.r, current_color.g,
				current_color.b);
		update_led_strip_to_physical_leds();
		apply_fade_to_all_leds();
		HAL_Delay(step_delay);
	}
	// Backward
	for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
		RGBColor current_color;
		rgb_color_wheel(wheel_position++, &current_color);
		set_led_color_in_strip(i, current_color.r, current_color.g,
				current_color.b);
		update_led_strip_to_physical_leds();
		apply_fade_to_all_leds();
		HAL_Delay(step_delay);
	}
}

// Animation for beating the game
static void play_won_animation() {
	uint16_t blink_delay = 600;
	for (int i = 0; i < 2; i++) {
		fill_strip_solid(0, 255, 0);
		update_led_strip_to_physical_leds();
		HAL_Delay(blink_delay);
		fill_strip_solid(0, 0, 0);
		update_led_strip_to_physical_leds();
		HAL_Delay(blink_delay);
	}
}
