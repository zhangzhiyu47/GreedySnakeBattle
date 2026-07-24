#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/// Game configuration of application reading and writing configuration files.
typedef struct GameConfig {
    uint64_t wallNum;           /**< The number of walls */
    uint64_t foodNum;           /**< The number of snake's food */

    uint64_t speed;             /**< The speed of snake */
    uint64_t histryHighestScr;  /**< The highest score of history */

    uint64_t scrnHigh;          /**< The height of the game area */
    uint64_t scrnWide;          /**< The weight of the game area */

    bool isEnableObs;           /**< Is enable the obstacle snake */
    bool isEnableEatSlfGmOver;  /**< Is enable die when snake eat body */

    uint64_t obsIQ;             //< The IQ of the obstacle snake
} GameConfig;

/**
 * Get game config, fallback to outline mode
 *
 * @retval 0 success, store game config to parameter config
 *         1 fail to read, but write default config successfully
 */
int getGameConfig(GameConfig *config);

/// Write game config file
int setGameConfig(GameConfig *config);

#endif  // GAME_CONFIG_H
