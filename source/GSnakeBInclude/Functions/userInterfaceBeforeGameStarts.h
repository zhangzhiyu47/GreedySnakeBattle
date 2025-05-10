/**
 * @file userInterfaceBeforeGameStarts.h
 * @brief This header declares the functions about user
 *        interface before the game starts.
 */
#ifndef USER_INTERFACE_BEFORE_GAME_STARTS_H
#define USER_INTERFACE_BEFORE_GAME_STARTS_H

#include "../Struct/GameAllRunningData.h"
#include <unistd.h>

/**
 * @brief Introduce and explain the game in detail.
 *
 * Introduce and explain the game in detail (basic gameplay,
 * game interface introduction, game rules and offline mode).
 */
void gameIntroduction();

/**
 * @brief Configure the game.
 *
 * Read the game settings adjusted by the end user and save
 * them to the configuration file for use when loading the
 * game interface. Due to file read and write operations,
 * offline mode may be enabled. In offline mode, the game can
 * still be configured (see @ref OffLineMode for details).
 */
void configureGame();

/**
 * @brief Show game menu.
 *
 * Let users choose to start the game, introduce the game,
 * set the game and exit the game. And perform the
 * corresponding behavior. In addition to starting the game
 * and exiting the game, the other two options will print the
 * menu again after execution.
 * | code | action |
 * | :--: | :----: |
 * | 1 | Start the game |
 * | 2 | Introduce the game |
 * | 3 | set the game |
 * | 4 | Exit the game |
 *
 * @param[in,out] data All the game's data when the game is running.
 * @param[in] childProcess ID of the child process.
 *
 * @return Does user want to quit the game?
 * @retval true  Yes.Then quit the game.
 * @retval false No.Then start the game.
 */
bool showGameMenu(GameAllRunningData *data,const pid_t childProcess);

/**
 * @brief First login game loading.
 * @note It's just for decoration. It's dispensable.
 */
void firstLoginToGameLoading();

/**
 * @brief The game starts to load animation.
 * @note It's just for decoration. It's dispensable.
 */
void gameStartupLoading();

#endif
