# STM32 Cyclone LED Game (WS2812B) 🌀🕹️

Welcome to the STM32 Cyclone LED Game! This project brings the classic "Cyclone" arcade game concept to life using an STM32F031K6 microcontroller and a vibrant WS2812B addressable LED strip. Test your reflexes by stopping the rotating green light precisely on the central red target!

## 🌟 Features

* **Engaging Gameplay:** Classic Cyclone game – stop the moving light on the target.
* **Progressive Difficulty:** Speed increases as you advance through levels.
* **Visual Feedback:** Uses a WS2812B LED strip for the game display and separate LEDs for life indication.
* **Winning Animation:** A special animation plays upon completing the hardest level.
* **STM32 Powered:** Runs on the efficient STM32F031K6 microcontroller.
* **Hardware Interrupts:** Utilizes button press interrupts for precise input.
* **PWM & DMA:** Leverages PWM and DMA for smooth WS2812B LED control.

---

## 🎯 Game Objective

The core goal is simple yet challenging:
1.  A **green** light will be seen rotating circularly on the LED strip.
2.  Your objective is to press the button at the exact moment the green light aligns with the fixed **red** target LED (defined as `CENTER_LED 29` in `Core/Src/game.c`).

* **Successful Stop:** 🎉 You advance to the next level! The game's speed (difficulty) will increase. Conquer the ultimate `MISSION_IMPOSSIBLE` level to witness a special winning animation, after which the game will reset.
* **Failed Stop:** 💔 You lose a life. Your remaining lives are visually indicated by 3 separate LEDs connected to the `GPIOB` port. When all lives are depleted, the game resets.

---

## 🛠️ Hardware and Configuration

### Components Needed:

* **Microcontroller:** STM32F031K6
* **LED Strip:** WS2812B Addressable RGB LED Strip
    * **Number of LEDs Used:** `59` (Configurable in `Core/Inc/ws2812.h` via `noOfLEDs`)
    * **Target LED Index:** `29` (Configurable in `Core/Src/game.c` via `CENTER_LED`). Ensure `CENTER_LED` is less than `noOfLEDs`.
* **Button:** 1 x Momentary Push-button
* **Life Indicator LEDs:** 3 x Standard LEDs (e.g., red, yellow, green)
* **Resistors:** Appropriate current-limiting resistors for the life indicator LEDs.
* **Power Supply:**
    * 5V for the WS2812B LED strip.
    * 3.3V for the STM32F031K6 (often supplied by the programmer or a dedicated regulator).
* **Connecting Wires**
* **ST-Link Programmer** (or similar for flashing the STM32)

### Pin Configuration:

* **Button:** Connected to `PA0`.
    * Configured in `main.c` (`MX_GPIO_Init()`) for external interrupt (`EXTI0_1_IRQn`), **Rising Edge** triggered, with an internal **Pulldown** resistor.
    * **Connection:** One terminal of the button to `PA0` and the other terminal to `3.3V`.
* **WS2812 Data Pin (DIN):** Driven via `TIM3_CH1` PWM output.
    * Reference: `Core/Inc/ws2812.h` -> `extern TIM_HandleTypeDef htim3`.
    * PWM and DMA settings are configured in `MX_TIM3_Init()` within `main.c`.
    * **Possible STM32F031K6 Pins for TIM3_CH1:** `PA6`, `PB4`, or `PC6`.
    * The specific GPIO pin configuration for `TIM3_CH1` is done in `HAL_TIM_MspPostInit(&htim3)` (typically found in `Core/Src/stm32f0xx_hal_msp.c`). **Verify this in your CubeMX configuration or code.**
* **Life Indicator LEDs:** Connected to `GPIOB` (defined in `Core/Src/game.c`).
    * 1st Life (e.g., Green LED): `PB0` (`LIFE_LED_1_PIN`)
    * 2nd Life (e.g., Yellow LED): `PB1` (`LIFE_LED_2_PIN`)
    * 3rd Life (e.g., Red LED): `PB7` (`LIFE_LED_3_PIN`)
    * **Connection:** Connect the anode of each LED to its respective GPIO pin and the cathode through a current-limiting resistor to `GND`.
* **Brightness:** LED brightness is set to `BRIGHTNESS 100` (on a scale of 0-255) in `Core/Src/game.c`.

**Important:** Ensure all `GND` lines (STM32, LED strip, life LEDs) are connected to a common ground.

---

## 🧩 Core Libraries and Functions

### `Core/Inc/ws2812.h` & `Core/Src/ws2812.c`
Handles the low-level driving of the WS2812B LED strip.
* `noOfLEDs`: Defines the total number of LEDs on the strip being used (currently `59`).
* `pwmData[]`: Buffer that holds the PWM duty cycle values representing LED colors.
* `WS2812_SetLED(uint16_t LEDnum, uint8_t RED, uint8_t GREEN, uint8_t BLUE)`: Sets the color of an individual LED in the buffer.
* `WS2812_Send()`: Transmits the color data from the `pwmData` buffer to the LED strip via PWM and DMA.
* `WS2812_ResetAllLED()`: Turns off all LEDs.
* `WS2812_SetAllLED(uint8_t RED, uint8_t GREEN, uint8_t BLUE)`: Sets all LEDs to the same specified color.

### `Core/Inc/game.h` & `Core/Src/game.c`
Contains the main game logic, state management, and animations.
* `Game_Setup()`: Initializes game variables, difficulty, and player lives.
* `Game_Loop()`: The main polling loop that updates the LED positions and checks game state. Called repeatedly from `main()`.
* `Game_ButtonPressCallback()`: Interrupt handler executed when the player presses the button. It checks if the light was stopped on the target.
* **Difficulty Levels:** Defines various game speeds, from `EASY` to `MISSION_IMPOSSIBLE`.
* **Animations:** Includes functions for various visual effects:
    * `play_cylon_animation()`
    * `play_flash_animation()`
    * `play_won_animation()`

### `Core/Inc/main.h` & `Core/Src/main.c`
The entry point of the application. Handles system initialization and dispatches interrupts.
* `SystemClock_Config()`: Configures the system clock, typically to 48MHz for this MCU.
* `MX_GPIO_Init()`, `MX_DMA_Init()`, `MX_TIM3_Init()`: Initializes the GPIO pins, DMA controller, and Timer 3 for PWM output.
* `HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)`: This HAL library callback is triggered by GPIO interrupts. It identifies the pin (`BUTTON_Pin`) and calls `Game_ButtonPressCallback()`.

---

## 🚀 Build and Run

### Prerequisites:

* **STM32CubeIDE:** Or any other compatible IDE for STM32 development (Keil, IAR).
* **STM32CubeProgrammer:** For flashing the compiled firmware to the microcontroller.
* **ST-Link Debugger/Programmer:** Or compatible programmer.

### Steps:

1.  **Get the Project:**
    * Clone this repository: `git clone https://github.com/aymansaeid/STM32-Cyclone-Led-Game.git`
    * Or download the project files as a ZIP and extract them.
2.  **Open in IDE:**
    * Open the project in STM32CubeIDE (File -> Open Projects from File System...).
3.  **Verify Configuration:**
    * **LED Count:** Double-check that `noOfLEDs` (currently `59`) in `Core/Inc/ws2812.h` matches the number of LEDs on your physical strip.
    * **Target LED:** Ensure `CENTER_LED` (currently `29`) in `Core/Src/game.c` is a valid index (i.e., less than `noOfLEDs`).
    * **TIM3_CH1 Pin:** Confirm the correct GPIO pin is configured for `TIM3_CH1` in `Core/Src/stm32f0xx_hal_msp.c` within the `HAL_TIM_MspPostInit(&htim3)` function. This pin will be connected to your WS2812B DIN.
4.  **Hardware Connections (Double Check!):**
    * Connect the **DIN** (Data Input) pin of the WS2812B LED strip to the STM32's `TIM3_CH1` PWM output pin (e.g., `PA6`, `PB4`, or `PC6`).
    * Connect the **button** between `PA0` and `3.3V`.
    * Connect the **life indicator LEDs** (e.g., `PB0`, `PB1`, `PB7`) to their respective pins. Remember to use current-limiting resistors and connect their cathodes to `GND`.
    * Provide a suitable **power supply**:
        * ~5V to the WS2812B strip's VCC.
        * 3.3V to the STM32F031K6.
    * Ensure all **GND** lines (STM32, LED strip, life LEDs) are connected to a **common ground**. This is crucial!
5.  **Build the Project:**
    * In STM32CubeIDE, compile the project (e.g., "Project" -> "Build All").
6.  **Flash the Microcontroller:**
    * Connect your ST-Link programmer to the STM32.
    * Upload the compiled `.hex` or `.bin` file (usually found in the `Debug` or `Release` folder within your project) to your STM32F031K6 using STM32CubeProgrammer or the IDE's built-in flashing utility.
7.  **Run & Play:**
    * Power up the hardware. The game should start automatically.
    * Enjoy the game and test your reflexes!

---

## 💡 Customization

Feel free to tweak the game to your liking:

* **Number of LEDs:** Modify `noOfLEDs` in `Core/Inc/ws2812.h`.
* **Target LED Position:** Change `CENTER_LED` in `Core/Src/game.c`.
* **Brightness:** Adjust `BRIGHTNESS` in `Core/Src/game.c` (0-255).
* **Difficulty Levels:** Modify the speed/delay values for each level in `Core/Src/game.c`.
* **Colors:** Change the LED colors used for the player, target, and animations.
* **Life LED Pins:** If needed, redefine `LIFE_LED_1_PIN`, `LIFE_LED_2_PIN`, `LIFE_LED_3_PIN` in `Core/Src/game.c` and update `MX_GPIO_Init()`.

---

## 📸 Game in Action! (Placeholder)



---

## 🤝 Contributing

Contributions are welcome! If you have ideas for improvements, new features, or bug fixes, feel free to:

1.  Fork the repository.
2.  Create a new branch (`git checkout -b feature/AmazingFeature`).
3.  Make your changes.
4.  Commit your changes (`git commit -m 'Add some AmazingFeature'`).
5.  Push to the branch (`git push origin feature/AmazingFeature`).
6.  Open a Pull Request.

---
