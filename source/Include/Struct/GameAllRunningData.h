/**
 * @file GameAllRunningData.h
 * @brief This header declares the struct @ref GameAllRunningData.
 */
#ifndef GAME_ALL_RUNNING_DATA_H
#define GAME_ALL_RUNNING_DATA_H

#include "../TypeDefine/typedef.h"
#include "Point.h"
#include <stdbool.h>

/**
 * @brief Include all the game-running data.
 */
typedef struct GameAllRunningData {
    /**
     * @name CONFIG.
     */
    /** @{ */
    muint_t speed;              /**< The speed of snake */
    muint_t minScrOpnVctryPnt;  /**< The minimum usr Score for opening the victory point */
    muint_t histryHighestScr;   /**< The highest usr score(The amount of food actually eaten by user's snakes) of history */
    /** @} */

    /**
     * @name ENABLE.
     */
    /** @{ */
    bool isEnableEatSlfGmOver;  /**< Is enable the setting of dying when the snakes eat themself */
    bool isEnableObs;           /**< Is enable the obstacle Snake */
    /** @} */

    /**
     * @name USER'S SNAKE.
     */
    /** @{ */
    Point usrSnkBody[1143];     /**< Record the position of each node of the user's snake bodies ([0] is the snake's head) */

    muint_t usrSnkLeng;         /**< The length of the user's snake */

    int usrSnkNxtXDrc;          /**< ←→ The user's snake next-step crosswise direction */
    int usrSnkNxtYDrc;          /**< ↓↑ The user's snake next-step vertical direction  */

    int usrSrc;                 /**< The usr score of user's snake(The amount of food actually eaten by the user's snake * 10, is deferent from 'histryHighestScr') */

    bool usrSnkGameEndState;    /**< The state of user's snake when the game is over(1 for winning,0 for losing) */

    bool usrSnkIsEatingObsSnk;  /**< Does the user's snake eat the obstacle snake */
    bool usrSnkIsJumping;       /**< Is user's snake jumping */
    /** @} */

    /**
     * @name FOOD.
     */
    /** @{ */
    Point food[1143];           /**< Record the position of each one of foods */
    muint_t foodNum;            /**< The number of snake's food */
    /** @} */

    /**
     * @name WALL.
     */
    /** @{ */
    Point wall[15];             /**< Record the position of each one of walls */
    muint_t wallNum;            /**< The number of walls */
    /** @} */

    /**
     * @name OBSTACLE SNAKE.
     */
    /** @{ */
    Point obsSnkBody[1143];     /**< Record the position of each node of the obstacle snake bodies ([0] is the snake's head) */
    
    muint_t obsSnkLeng;         /**< The length of the obstacle snake */
    
    muint_t obsState;           /**< The state of obstacle snake(The details are as follows)
                                     | Number | Represent |
                                     | :----: | :-------: |
                                     | 0 | living(if it is enable) |
                                     | 1 | die by eating the walls |
                                     | 2 | die by eating the user snake |
                                */
    
    int obsSnkNxtXDrc;          /**< ←→ The obstacle snake next-step crosswise direction */
    int obsSnkNxtYDrc;          /**< ↓↑ The obstacle snake next-step vertical direction  */

    int obsClosestFood;         /**< The array subscript of the food closest to the obstacle snake */
    /** @} */
} GameAllRunningData;

#endif
