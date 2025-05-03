/**
 * @file gameSetConfiguration.h
 * @brief This header declares the functions
 *       about game settings and configurations.
 */
#ifndef GAME_SETTINGS_AND_CONFIGURATIONS_H
#define GAME_SETTINGS_AND_CONFIGURATIONS_H

/**
 * @defgroup SetConfig Greedy Snake Battle Game Settings
 * @brief These functions are used for game settings.
 *
 * These functions are used for game settings. These functions
 * all start with 'setConfig', connected by '_' in the middle,
 * and followed directly by the variable name in the GameConfig
 * to be set (for example, to set GameConfig.isEnableObs, you
 * need to call the @ref setConfig.isEnableObs function). Except
 * for the @ref setConfigAllToDefault function, which is used
 * to restore all settings to default. The parameters of these
 * functions are all GameConfig (i.e. all game configurations).
 */

#include "../Struct/GameConfig.h"

/**
 * @brief Set GameConfig.isEnableEatSlfGmOver.
 * @ingroup SetConfig
 * 
 * | number | setting |
 * | :----: | :-----: |
 * | 1 | Enable |
 * | others | disable |
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_isEnableEatSlfGmOver(GameConfig *config);

/**
 * @brief Set GameConfig.speed.
 * @ingroup SetConfig
 *
 * Read from the user in seconds and store in microseconds.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_speed(GameConfig *config);

/**
 * @brief Set GameConfig.minScrOpnVctryPnt.
 * @ingroup SetConfig
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_minScrOpnVctryPnt(GameConfig *config);

/**
 * @brief Set GameConfig.foodNum.
 * @ingroup SetConfig
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_foodNum(GameConfig *config);

/**
 * @brief Set GameConfig.wallNum.
 * @ingroup SetConfig
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_wallNum(GameConfig *config);

/**
 * @brief Set GameConfig.isEnableObs.
 * @ingroup SetConfig
 * 
 * | number | setting |
 * | :----: | :-----: |
 * | 1 | Enable |
 * | others | disable |
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_isEnableObs(GameConfig *config);

/**
 * @brief Set GameConfig.scrnHigh and GameConfig.scrnWide.
 * @ingroup SetConfig
 *
 * Set GameConfig.scrnHigh and GameConfig.scrnWide.By the
 * way,set global variables @ref HIGH and @ref WIDE.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_scrnHigh_scrnWide(GameConfig *config);

/**
 * @brief Set GameConfig.wordColr and GameConfig.scrnColr.
 * @ingroup SetConfig
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_wordColr_scrnColr(GameConfig *config);

/**
 * @brief Restore all settings to default.
 * @ingroup SetConfig
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfigAllToDefault(GameConfig* config);

#endif
