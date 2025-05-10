/**
 * @file GameConfig.h
 * @brief This header declares struct GameConfig.
 */
#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include "../TypeDefine/typedef.h"
#include <stdbool.h>

/**
 * @brief Game configuration of program reading and writing configuration files.
 *
 * > **Tip**: The following *VR* is a symbol of *value range*.
 */
typedef struct GameConfig {
    /**
     * @name NUMBER.
     */
    /** @{ */
    muint_t wallNum;            /**< The number of walls(VR:0-15) */
    muint_t foodNum;            /**< The number of snake's food(VR:1-20) */
    /** @} */

    /**
     * @name CONFIG.
     */
    /** @{ */
    muint_t speed;              /**< The speed of snake(VR:0.2*1000*1000-2*1000*1000 microsecond) */
    muint_t minScrOpnVctryPnt;  /**< The minimum usr score for opening the victory point(VR:30-500) */
    muint_t histryHighestScr;   /**< The highest usr score(The amount of food actually eaten by user's snakes) of history */
    /** @} */

    /**
     * @name GAME SCREEN.
     */
    /** @{ */
    muint_t scrnHigh;           /**< The height of the game-face sreen(VR:5-40) */
    muint_t scrnWide;           /**< The weight of the game-face sreen(VR:10-80) */
    /** @} */

    /**
     * @name ENABLE.
     */
    /** @{ */
    bool isEnableObs;           /**< Is enable the obstacle snake */
    bool isEnableEatSlfGmOver;  /**< Is enable the setting of dying when the snakes eat themself */
    /** @} */

    /**
     * @name COLOUR.
     */
    /** @{ */
    int wordColr;               /**< The color of the game-face words(VR:0-255) */
    int scrnColr;               /**< The color of the game-face sreen(VR:0-255) */
    /** @} */
} GameConfig;

#endif
