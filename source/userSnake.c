/**
 * @file userSnake.c
 * @brief This source realizes the functions about snake's snake.
 */

#include "Include/Struct/GameAllRunningData.h"
#include "Include/Compat/snakeFullCompat.h"
#include "Include/Functions/food.h"

/**
 * @brief Move user's snake.
 *
 * The position of the snake's last body is copied from
 * the previous body, and the coordinates of the snake
 * head are added to GameAllRunningData.usrSnkNxtXDrc and
 * GameAllRunningData.usrSnkNxtYDrc
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void userSnakeMove(GameAllRunningData *data) {
    for ( int i=data->usrSnkLeng-1; i>0; i-- ) {
        data->usrSnkBody[i]=data->usrSnkBody[i-1];
    }
    data->usrSnkBody[0].x+=data->usrSnkNxtXDrc;
    data->usrSnkBody[0].y+=data->usrSnkNxtYDrc;
    return;
}

/**
 * @brief Control the direction of user's snake movement.
 *
 * | input | action |
 * | :---: | :----: |
 * | A/7 | ← |
 * | W/5 | ↑ |
 * | S/8 | ↓ |
 * | D/9 | → |
 * | j/J | Jump |
 * | f/F | Fly |
 * | o/O | Game over |
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @return The state of the game when the game is running.
 * @retval 0 Game will be last running.
 * @retval 1 Game will be over.
 */
int userSnakeMoveDirecControl(GameAllRunningData *data) {
    char key=0;
    if ( data->usrSnkIsJumping ) {
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc++;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc--;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc++;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc--;
        }
        data->usrSnkIsJumping=0;
    }
    while ( linux_kbhit() ) {
        key=linux_getch();
    }
    switch (key) {
    case 'a':
    case 'A':
    case '7':
        data->usrSnkNxtXDrc=-1;
        data->usrSnkNxtYDrc=0;
        break;
    case 'w':
    case 'W':
    case '5':
        data->usrSnkNxtXDrc=0;
        data->usrSnkNxtYDrc=-1;
        break;
    case 's':
    case 'S':
    case '8':
        data->usrSnkNxtXDrc=0;
        data->usrSnkNxtYDrc=1;
        break;
    case 'd':
    case 'D':
    case '9':
        data->usrSnkNxtXDrc=1;
        data->usrSnkNxtYDrc=0;
        break;
    case 'j':
    case 'J':
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc--;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc++;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc--;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc++;
        }
        data->usrSnkIsJumping=1;
        break;
    case 'f':
    case 'F':
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc--;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc++;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc--;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc++;
        }
        data->usrSnkIsJumping=0;
        break;
    case 'o':
    case 'O':
        return 1;
    }
    return 0;
}

/**
 * @brief User's snake eat foods or death the obstacle
 *        snake's body(If it is enable).
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @todo When the user snake eats the body of the obstacle snake,
 *       this function simply reduces the length of the obstacle
 *       snake to reduce the length of the snake, but this method
 *       can not accurately show the snake that will be eaten when
 *       drawing the game interface. Need to modify part of the
 *       logic of the code and the logic of drawing the snake's body
 *       (@ref gameInterfacePainting function also needs to improve).
 */
void userSnakeEatFood(GameAllRunningData *data) {
    for ( int i=0; i<data->foodNum; i++ ) {
        if ( data->food[i].x==data->usrSnkBody[0].x && \
                data->food[i].y==data->usrSnkBody[0].y ) {
            foodInit(data,i);
            data->usrSnkLeng++;
            data->usrSrc+=10;
        }
    }
    for ( int j=0; j<data->obsSnkLeng && \
            data->obsState!=0; j++ ) {
        if ( data->obsSnkBody[j].x==data->usrSnkBody[0].x && \
                data->obsSnkBody[j].y==data->usrSnkBody[0].y ) {
            data->usrSnkLeng++;
            data->usrSrc+=10;
            data->obsSnkLeng--; /// @todo Need improvement
        }
    }
}

/**
 * @brief Is user's snake eating wall.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating wall.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatWall(const GameAllRunningData *data) {
    for ( int i=0; i<data->wallNum; i++ ) {
        if ( data->usrSnkBody[0].x==data->wall[i].x && \
                data->usrSnkBody[0].y==data->wall[i].y) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Is user's snake eating obstacle snake body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating obstacle snake body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatObsSnake(const GameAllRunningData *data) {
    for ( int i=1; i<data->obsSnkLeng; i++ ) {
        if ( data->usrSnkBody[0].x==data->obsSnkBody[i].x && \
                data->usrSnkBody[0].y==data->obsSnkBody[i].y) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Is user's snake eating its own body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating its own body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatSelf(const GameAllRunningData *data) {
    for ( int i=1; i<data->usrSnkLeng; i++ ) {
        if ( data->usrSnkBody[0].x==data->usrSnkBody[i].x && \
                data->usrSnkBody[0].y==data->usrSnkBody[i].y ) {
            return true;
        }
    }
    return false;
}
