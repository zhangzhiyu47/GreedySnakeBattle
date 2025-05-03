/**
 * @file GreedySnakeBattleGameExternalInterface.h
 * @brief This header declares the function @ref
 *        GreedySnakeBattleGameExternalInterface.
 * @author Zhang Zhiyu
 */
#ifndef GREEDY_SNAKE_BATTLE_GAME_EXTERNAL_INTERFACE_H
#define GREEDY_SNAKE_BATTLE_GAME_EXTERNAL_INTERFACE_H

/**
 * @page GSnakeBGEI Greedy Snake Battle Game External Interface
 * @brief The Snake Battle Game opens the only suggested
 *        interface for game applications.
 *
 * # Function
 * @ref GreedySnakeBattleGameExternalInterface
 */

/**
 * @brief Block running game.
 *
 * This macro is used for the function parameter
 * of @ref GreedySnakeBattleGameExternalInterface,
 * which blocks the operation of the greedy snake
 * battle game.
 */
extern const int SNAKE_BLOCK;   /* value:1 */
/**
 * @brief Non-block running game.
 *
 * This macro is used for the function parameter
 * of @ref GreedySnakeBattleGameExternalInterface,
 * which non-blocks the operation of the greedy snake
 * battle game.
 */
extern const int SNAKE_UNBLOCK; /* value:0 */

/**
 * ---
 * # About this function
 *  
 * @brief The Snake Battle Game opens the only suggested
 *        interface for game applications.
 * @author Zhang Zhiyu
 * @version 2.13.14
 *  
 * ## Header file and function prototype 
 * ```c
 * #include "GreedySnakeBattleGameExternalInterface.h"
 *
 *     int GreedySnakeBattleGameExternalInterface(int isBlockRunning);  
 * ```
 *
 * ## Library
 * Library files that need to be connected (both are acceptable,just
 * choose one of them).  
 * | Type | Library name | GCC connection instruction |
 * | :--: | :----------: | :------------------------: |
 * | Static connection | libgsnakebg.a | gcc ...... -lgsnakebg |
 * | Dynamic connection | libgsnakebg.so | gcc ...... -lgsnakebg |
 *
 * ## Standard
 * Code execution standard.
 * | Project | Standard |
 * | :-----: | :------: |
 * | Code language | the C programming language - C99(ISO/IEC 9899:1999) |
 * | Unix-like OS | POSIX.1-2008(Portable Operating System Interface of UNIX) |
 * | Windows OS | Win32 API(Windows Application Programming Interface) |
 *  
 * ## Introduction
 * **The Snake Battle Game opens the only suggested
 * interface for game applications**.  
 * This function call is divided into blocking and
 * non-blocking operation. Blocking is to block the
 * process of calling this function until the game is
 * over. Non-blocking means to exit the function
 * immediately after starting the game application, but
 * the game will continue. The game may fail to start
 * for various reasons, so **it is necessary to check the
 * return value of this function**.  
 *  
 * ## Parameter
 * @param isBlockRunning Parent process blocking when the
 *                       game is running.
 *                       | Macros(Real global constants) | Actions |
 *                       | :----: | :-----: |
 *                       | @ref SNAKE_BLOCK | Block running game |
 *                       | @ref SNAKE_UNBLOCK | Non-blocking running game |
 *
 * ## Return
 * @return Call status of this function.
 * @retval -2 Parameter error.
 * @retval -1 System call failed.
 * @retval 0  Blocking operation, game over.
 * @retval 1  Non-blocking operation, game startup completed,
 *            exit the function.
 *
 * ---
 * # About Greedy Snake Battle Game
 *
 * ## Introduction to the Snake Battle
 * Greedy Snake Battle is a classic game, and I have created a game
 * model using simple C language. You can use the "W, A, S, D" keys to
 * control the snake to move up, down, left, and right respectively
 * (triggering the mysterious key will bring surprises). Whenever you enter
 * "E", you will exit the game directly, and entering "O" in the game will
 * exit the current match. When selecting the interface, enter the serial
 * number before the function to perform the operation. If the input is
 * incorrect, the system will wait for you to enter the correct number.  
 *
 * "+"is the wall, "#"is the food(Obstacle snakes also become food after
 * death), "@"is the snake head,"*"is the snake body, and "$" is the victory
 * point. When the snake head touches the wall (or snake body), it will die
 * and turn into food. Eating food will increase the score and snake length.
 * When the score reaches a certain amount, a victory point will be displayed,
 * and the snake head will win when it reaches a certain amount, or it will
 * automatically win when it reaches a certain score.  
 *
 * If you are not satisfied with the game settings, please set them
 * yourself in "Game Settings".  
 *  
 * ## Game running process and frame  
 * - 1. Client resource loading (optional)
 * - 2. First login game loading (optional, only the first login
 *      game has this screen)
 * - 3. Menu interface
 *      | option | action |
 *      | :--: | :----: |
 *      | 1 | Start the game |
 *      | 2 | Introduce the game |
 *      | 3 | set the game |
 *      | 4 | Exit the game |
 *      (In addition to starting the game and exiting the
 *      game, the other two options will print the menu
 *      again after execution.)
 *  
 * ## Possible future functions
 *      | code | functions |
 *      | :----: | :----: |
 *      | 1 | Set the portal |
 *      | 2 | Unlimited food |
 *
 * ---
 * # BUGs and TODOs
 * 
 * ## BUGs
 * @bug
 *      When the game is running, when the snake's body is
 *      large enough, the game will get stuck there unless the
 *      signal is sent to stop(In @ref mainLoopOfGameProcess
 *      function).
 *
 * ## TODOs
 * @todo
 *       - 1.Obstacle snake unable to automatically avoid user snakes
 *           and walls, need to add judgment code(in @ref obsMoveDirecControl
 *           function).  
 *       - 2.When the user snake eats the body of the obstacle snake,
 *           this function simply reduces the length of the obstacle
 *           snake to reduce the length of the snake, but this method
 *           can not accurately show the snake that will be eaten when
 *           drawing the game interface. Need to modify part of the
 *           logic of the code and the logic of drawing the snake's body
 *           (In @ref gameInterfacePainting function and @ref userSnakeEatFood
 *           function).
 *
 * ---
 * # OffLine mode introduction
 *
 * ## Introduction
 * The mode is opened when **the configuration file cannot be
 * opened**. There are many places to read the configuration file,
 * and there are many reasons why it cannot be read. Once the
 * offline mode is opened, it cannot be closed before the game is
 * over. In this mode, the game no longer reads the configuration
 * file, but relies on its own configuration variables and default
 * configuration to play the game.  
 *
 * ## Attention
 * @attention In this mode, the game settings can still be adjusted,
 *            but in the configuration file that will not be saved,
 *            the settings can only last until the end of the game,
 *            and it is impossible to automatically restore the
 *            content set by the player when the game is restarted.
 *            Changing the game settings in this mode is not recommended.
 *
 * ---
 * # Example of this function  
 * ```c
 * #include <GreedySnakeBattleGameExternalInterface.h>
 *
 * int main(int argc, char* argv[]) {
 *     // Other code logic.
 *     // while(1) { ...... }
 * 
 *     int ret=GreedySnakeBattleGameExternalInterface(SNAKE_BLOCK);
 * 
 *     switch (ret) {
 *         case -2:
 *             printf("An incorrect parameter was passed in!\n");
 *             // Other error handling logic.
 *             break;
 *         case -1:
 *             printf("Greedy Snake Battle Game application failed to start!\n");
 *             // Other error handling logic.
 *             break;
 *         case 0:
 *             printf("Greedy Snake Battle Game is over.\n");
 *             // Other logic.
 *             break;
 *         case 1:
 *             printf("Greedy Snake Battle Game has been running in the background.\n");
 *             // Other logic.
 *             break;
 *     }
 * 
 *     // Other code logic.
 *     // while(1) { ...... }
 * }
 * ```
 */
int GreedySnakeBattleGameExternalInterface(int isBlockRunning);

#endif
