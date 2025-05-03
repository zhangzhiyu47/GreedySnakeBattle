/**
 * @file gameApplicationStartupRelated.h
 * @brief This header declares the functions about launching game app.
 */
#ifndef GAME_APPLICATION_STARTUP_RELATED_H
#define GAME_APPLICATION_STARTUP_RELATED_H

/*
 * @brief Configuration file initialization and game interface
 *        rendering.
 *
 * Read the configuration file, and if it exists, render the
 * game interface color and assign GameConfig.scrnHigh and
 * GameConfig.scrnWide to @ref HIGH and @ref WIDE. If the opening fails,
 * open it again by writing, then render the interface. If the
 * opening is successful, write the default game configuration
 * to the configuration file; If unsuccessful, do not read or
 * write files and exit the function.(The configuration file is
 * "./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data".)
 *
 * @return Returns the status of file reading and writing.
 * @retval  0 For successful reading of the configuration file
 *                and rendering.
 * @retval  1 For no configuration file but successfully
 *                initialized and rendered configuration file.
 * @retval -1 For that both reading and writing of the
 *                configuration file have failed, and offline mode
 *                needs to be enabled.
 *
 * @attention This function is complex, but necessary!
 */
int confgFileInitAndGameIntrfcRndrng();

#endif
