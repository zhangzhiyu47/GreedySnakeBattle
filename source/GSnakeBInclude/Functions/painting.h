/**
 * @file painting.h
 * @brief This header declares the functions about painting interface.
 */

#ifndef PAINTING_H
#define PAINTING_H

#include "../Struct/GameAllRunningData.h"
#include "../Struct/GameConfig.h"

#include <stddef.h>

/**
 * Formats number with leading zeros and fixed decimal places
 * 
 * @param value Number to format
 * @param decimalPlaces Number of decimal places
 * @param maxValue Maximum value (determines leading zeros)
 * @param buffer Output buffer
 * @param bufferSize Buffer size
 */
void formatNumber(double value, int decimalPlaces, double maxValue, 
                  char* buffer, size_t bufferSize);

/**
 * Adjusts number with full cursor control and validation
 * 
 * @param defaultValue Initial value
 * @param decimalPlaces Decimal places
 * @param maxValue Maximum value
 * @param minValue Minimum value
 * @param prefixText Prefix text
 * @param suffixText Suffix text
 * @param config Game config
 * @return Final adjusted value
 */
double adjustNumberPainting(double defaultValue, unsigned int decimalPlaces, 
                    double maxValue, double minValue,
                    const char* prefixText, const char* suffixText, const GameConfig config);

/**
 * @brief Displays a menu interface and allows user to
 *        select an option using arrow keys or w/s keys.
 * 
 * This function prints a list of menu entries and allows
 * the user to navigate through them using either arrow
 * keys (up/down) or w/s keys. The currently selected option
 * is indicated by a > marker. The selection is confirmed
 * by pressing Enter or Tab.
 *
 * @param size The number of entries in the menu
 * @param entry An array of strings representing the menu entries
 * @param defaultOption At the beginning, point to the option
 *                      pointed by the marker >
 * @return The index (1-based) of the selected menu option
 * 
 * @note The function uses ANSI escape codes for cursor positioning
 * @note Navigation can be done with arrow keys (converted
 *       to w/s) or direct w/s key presses
 * @note The selection is confirmed with Enter or Tab key
 */
int selectInterfacePainting(int size, char* entry[], unsigned int defaultOption);

/**
 * @brief Paint all the walls.
 * 
 * Use @ref HIGH,@ref WIDE,GameAllRunningData.wallNum
 * and GameAllRunningData.wall to paint the outer fences
 * and obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @attention The parameters are not checked, the wrong parameters
 *            will produce unpredictable results or crash.
 */
void wallPainting(GameAllRunningData *data);

/**
 * @brief Paint the game interface when the game is running.
 *
 * Paint foods,user's snake,obstacle snake(if it is enable)
 * and user's score.
 *
 * @param[in] data All the game's data when the game is running.
 */
void gameInterfacePainting(GameAllRunningData const *const data);

/**
 * @brief Paint the game-over interface and update the highest
 *        history score(If the configuration file open failed,
 *        it will start the Offline Mode).
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void gameOverPainting(GameAllRunningData *data);

#endif
