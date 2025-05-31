/**
 * @file userSnake.h
 * @brief This header declares the functions about snake's snake.
 */
#ifndef USER_SNAKE_H
#define USER_SNAKE_H

#include "../Struct/GameAllRunningData.h"

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
void userSnakeMove(GameAllRunningData *data);

/**
 * @brief Control the direction of user's snake movement.
 *
 * | input | action |
 * | :---: | :----: |
 * | ←/A/7 | ← |
 * | ↑/W/5 | ↑ |
 * | ↓/S/8 | ↓ |
 * | →/D/9 | → |
 * | j/J | Jump |
 * | f/F/Tab | Fly |
 * | p/P | Pause |
 * | r/R | Repaint |
 * | Esc | Block game |
 * | o/O | Game over |
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @return The state of the game when the game is running.
 * @retval 0 Game will be last running.
 * @retval 1 Game will be over.
 */
int userSnakeMoveDirecControl(GameAllRunningData *data);

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
 *       (Maybe is gameInterfacePainting(GameAllRunningData const
 *       *const data) function).
 */
void userSnakeEatFood(GameAllRunningData *data);

/**
 * @brief Is user's snake eating its own body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating its own body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatSelf(const GameAllRunningData *data);

/**
 * @brief Is user's snake eating wall.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating wall.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatWall(const GameAllRunningData *data);

/**
 * @brief Is user's snake eating obstacle snake body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating obstacle snake body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatObsSnake(const GameAllRunningData *data);

#endif
