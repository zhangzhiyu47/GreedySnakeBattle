#ifndef GAME_ALL_RUNNING_DATA_H
#define GAME_ALL_RUNNING_DATA_H

#include "Point.h"
#include <stdbool.h>
#include <stdint.h>

/// All the game-running data.
typedef struct GameAllRunningData {
    uint64_t speed;             /**< The speed of snake */
    uint64_t histryHighestScr;  /**< The highest usr score(The amount of food actually eaten by user's snakes) of history */

    bool isEnableEatSlfGmOver;  /**< Is enable the setting of dying when the snakes eat themself */
    bool isEnableObs;           /**< Is enable the obstacle Snake */

    Point usrSnkBody[1143];     /**< Record the position of each node of the user's snake bodies ([0] is the snake's head) */

    uint64_t usrSnkLeng;        /**< The length of the user's snake */

    int usrSnkNxtXDrc;          /**< ←→ The user's snake next-step crosswise direction */
    int usrSnkNxtYDrc;          /**< ↓↑ The user's snake next-step vertical direction  */

    uint64_t usrSrc;            //< User's score

    bool usrSnkGameEndState;    /**< The state of user's snake when the game is over(1 for winning,0 for losing) */

    bool usrSnkIsEatingObsSnk;  /**< Does the user's snake eat the obstacle snake */
    bool usrSnkIsJumping;       /**< Is user's snake jumping */

    Point food[1143];           /**< Record the position of each one of foods */
    uint64_t foodNum;           /**< The number of snake's food */

    Point wall[15];             /**< Record the position of each one of walls */
    uint64_t wallNum;           /**< The number of walls */

    Point obsSnkBody[1143];     /**< Record the position of each node of the obstacle snake bodies ([0] is the snake's head) */
    
    uint64_t obsSnkLeng;        /**< The length of the obstacle snake */
    
    uint64_t obsState;          /**< The state of obstacle snake(The details are as follows)
                                     | Number | Represent |
                                     | :----: | :-------: |
                                     | 0 | living(if it is enable) |
                                     | 1 | die by eating the walls |
                                     | 2 | die by eating the user snake |
                                */
    
    int obsSnkNxtXDrc;          /**< ←→ The obstacle snake next-step crosswise direction */
    int obsSnkNxtYDrc;          /**< ↓↑ The obstacle snake next-step vertical direction  */

    int obsClosestFood;         /**< The array subscript of the food closest to the obstacle snake */
    uint64_t obsIQ;             //< The IQ of the obstacle snake

    uint64_t refreshTimes;      //< The refresh times of the game interface
} GameAllRunningData;

#endif
