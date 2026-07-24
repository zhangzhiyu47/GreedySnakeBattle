#ifndef GAME_ALL_RUNNING_DATA_H
#define GAME_ALL_RUNNING_DATA_H

#include "Point.h"
#include "../constants.h"

#include <stdbool.h>
#include <stdint.h>

/// All the game-running data.
typedef struct GameAllRunningData {
    uint64_t speed;             /**< Speed of snake */
    uint64_t histryHighestScr;  /**< The highest score of history */

    bool isEnableEatSlfGmOver;  /**< Is enable die when snake eat body */
    bool isEnableObs;           /**< Is enable the obstacle snake */

    /**< Each node of user's snake bodies ([0] for head) */
    Point usrSnkBody[1143];

    uint64_t usrSnkLeng;        /**< Length of the user's snake */

    int usrSnkNxtXDrc;          /**< ←→ The user's snake next-step crosswise direction */
    int usrSnkNxtYDrc;          /**< ↓↑ The user's snake next-step vertical direction  */

    uint64_t usrSrc;            //< User's score

    bool usrSnkIsJumping;       /**< Is user's snake jumping */

    Point food[FOOD_NUMBER_MAX];/**< Position of each foods */
    uint64_t foodNum;           /**< Number of snake's food */

    Point wall[WALL_NUMBER_MAX];/**< Position of each walls */
    uint64_t wallNum;           /**< Number of walls */

    /**< Each node of obstacle snake bodies ([0] for head) */
    Point obsSnkBody[1143];    
    uint64_t obsSnkLeng;        /**< Length of the obstacle snake */
    
    uint64_t obsState;          /**< The state of obstacle snake
                                     | 0 | living |
                                     | 1 | die by eating walls |
                                     | 2 | die by eating user snake | */
    
    int obsSnkNxtXDrc;          /**< ←→ The obstacle snake next-step crosswise direction */
    int obsSnkNxtYDrc;          /**< ↓↑ The obstacle snake next-step vertical direction  */

    int obsClosestFood;         /**< The array subscript of the food closest to the obstacle snake */
    uint64_t obsIQ;             //< The IQ of the obstacle snake

    uint64_t refreshTimes;      //< The refresh times of the game interface
} GameAllRunningData;

#endif
