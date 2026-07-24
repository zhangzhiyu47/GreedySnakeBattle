/**
 * @file food.c
 * @brief This source realizes the functions about snake's foods.
 */

#include "include/Struct/GameAllRunningData.h"
#include "include/GlobalVariable/globalVariable.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Initialize a food.
 *
 * Update a terminal position (or initialize) for the
 * GameAllRunningData.food[number].
 *
 * @param[in,out] data All the game's data when the game is running.
 * @param number The index of the data.food will be
 *               initialized or updated.
 */
void foodInit(GameAllRunningData *data, int number) {
    for ( bool isRandWrongPos=true; isRandWrongPos; ) {
        data->food[number].x=rand()%(WIDE-3)+2;
        data->food[number].y=rand()%(HIGH-3)+2;
        isRandWrongPos=false;
        for (uint64_t i=0; i<data->usrSnkLeng; i++ ) {
            if ( data->usrSnkBody[i].x==data->food[number].x &&
                    data->usrSnkBody[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->foodNum; i++ ) {
            if ( data->food[i].x==data->food[number].x &&
                    data->food[i].y==data->food[number].y
                    && i != (uint64_t)number) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->wallNum; i++ ) {
            if ( data->wall[i].x==data->food[number].x &&
                    data->wall[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->obsSnkLeng; i++ ) {
            if ( data->obsSnkBody[i].x==data->food[number].x &&
                    data->obsSnkBody[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
    }
}
