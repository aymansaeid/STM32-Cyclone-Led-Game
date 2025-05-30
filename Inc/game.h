#ifndef INC_GAME_H_
#define INC_GAME_H_

#include "main.h"
#include <stdbool.h>

// Functions
void Game_Setup(void);
void Game_Loop(void);

// Interrupt function to be called
void Game_ButtonPressCallback(void);

#endif /* INC_GAME_H_ */
