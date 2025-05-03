/**
 * @file wall.c
 * @brief This source realizes the functions about obstacle walls.
 */

#include "Include/Struct/GameAllRunningData.h"
#include "Include/GlobalVariable/globalVariable.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Initialize all the obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void wallInit(GameAllRunningData *data) {
    srand((unsigned)time(NULL));
    for ( int i=0; i<data->wallNum; i++ ) {
        for ( bool isRandWrongPos=true; isRandWrongPos; ) {
            data->wall[i].x=rand()%(WIDE-3)+2;
            data->wall[i].y=rand()%(HIGH-3)+2;
            isRandWrongPos=false;
            for ( int j=0; j<i; j++ ) {
                if ( data->wall[j].x==data->wall[i].x && \
                        data->wall[j].y==data->wall[i].y ) {
                    isRandWrongPos=true;
                    break;
                }
            }
            if ( data->wall[i].y>27 && data->wall[i].y<32 && \
                    data->wall[i].x>7 && data->wall[i].x<13 ) {
                isRandWrongPos=true;
            }
        }
    }
}
