#include "include/GlobalVariable/globalVariable.h"
#include "include/Functions/terminal.h"
#include "include/painting.h"
#include "include/gameConfig.h"
#include "include/menu.h"
#include "include/browser.h"
#include "include/button.h"
#include "include/number.h"
#include "include/gameMenu.h"
#include "include/constants.h"
#include "include/logger.h"
#include "include/exitApp.h"
#include "include/res/gameIntro.h"
#include "include/res/EULA.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/stat.h>

/// Introduce and explain the game in detail.
void gameIntroduction(const char *button) {
    Point termSize = terminalSize();
    fillBackground(termSize.x, termSize.y, NULL);

    TextBrowser *tb = browserCreate();
    browserHint(tb, "介绍 & 教程");
    browserText(tb, (const char*)gameIntro);
    browserButtonRight(tb, button);
    browserStyle(tb, 1, 1);
    browserUseAltBuffer(tb, 0);
    browserRun(tb);

    browserFree(tb);
}

/// Show EULA
static void showEULA(const char *button, const char *hint) {
    Point termSize = terminalSize();
    fillBackground(termSize.x, termSize.y, NULL);

    TextBrowser *tb = browserCreate();
    browserHint(tb, hint);
    browserText(tb, (const char*)EULA);
    browserButtonRight(tb, button);
    browserPrompt(tb, "《最终用户许可协议（EULA）》");
    browserStyle(tb, 1, 1);
    browserUseAltBuffer(tb, 0);
    browserRun(tb);

    browserFree(tb);
}

static void showDeveloper() {
    char developer[2048] = {0};
    snprintf(developer, 2048,
            "开发者：张志宇工作室\n"
            "联系方式：电话：187 3603 3903（微信同号）\n"
            "          邮箱：2585689367@qq.com\n"
            "\n开发者比较懒，zZZ zZZ...");

    Point termSize = terminalSize();
    fillBackground(termSize.x, termSize.y, NULL);

    TextBrowser *tb = browserCreate();
    browserHint(tb, "联系开发者");
    browserText(tb, developer);
    browserButtonRight(tb, "返回");
    browserStyle(tb, 1, 1);
    browserUseAltBuffer(tb, 0);
    browserRun(tb);

    browserFree(tb);
}

static void gameVersion() {
    char gameVersion[2048] = {0};
    snprintf(gameVersion, 2048,
            "游戏版本：%s\n配置文件版本：%s\n",
            APP_VERSION, CONFIG_VERSION);

    Point termSize = terminalSize();
    fillBackground(termSize.x, termSize.y, NULL);

    TextBrowser *tb = browserCreate();
    browserHint(tb, "关于版本");
    browserText(tb, gameVersion);
    browserButtonRight(tb, "返回");
    browserStyle(tb, 1, 1);
    browserUseAltBuffer(tb, 0);
    browserRun(tb);

    browserFree(tb);
}

void configureGame(GameConfig *config, int selected) {
    bool needWrite = false;

    if (selected == 0) {
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "小蛇的头碰到 ↓ 将会死亡（当前：%s）",
                config->isEnableEatSlfGmOver? "B": "A");

        Button *b = buttonCreate();
        buttonTitle(b, "设置 -> 死亡条件");
        buttonHint(b, buf);
        buttonAdd(b, "A: 墙 / 其他小蛇");
        buttonAdd(b, "B: 墙 / 其他小蛇 / 自己蛇身");
        buttonBottomRight(b, "确认");
        buttonBgDraw(b, fillBackground, NULL);
        buttonInitial(b, config->isEnableEatSlfGmOver);
        buttonUseAltBuffer(b, 0);

        usleep(40 * 1000);
        tcflush(STDIN_FILENO, TCIFLUSH);
        ButtonResult res = buttonRun(b);
        if (res.confirmed) {
            config->isEnableEatSlfGmOver = res.selectedTop;
            needWrite = true;
        }

        buttonFree(b);
    } else if (selected == 1) {
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "是否开启系统小蛇（当前：%s）",
                config->isEnableObs? "已开启": "未开启");

        Button *b = buttonCreate();
        buttonTitle(b, "设置 -> 系统小蛇");
        buttonHint(b, buf);
        buttonAdd(b, "开启");
        buttonAdd(b, "不开启");
        buttonBottomRight(b, "确认");
        buttonBgDraw(b, fillBackground, NULL);
        buttonInitial(b, !config->isEnableObs);
        buttonUseAltBuffer(b, 0);

        usleep(40 * 1000);
        tcflush(STDIN_FILENO, TCIFLUSH);
        ButtonResult res = buttonRun(b);
        if (res.confirmed) {
            config->isEnableObs = !res.selectedTop;
            needWrite = true;
        }

        buttonFree(b);
    } else if (selected == 2) {
        Point termSize = terminalSize();
        NumberDialog *nd = numberCreate();

        numberField(nd, "食物数量", 1, FOOD_NUMBER_MAX,
                config->foodNum, 0);
        numberStep(nd, 0, 1);

        numberField(nd, "围墙数量", 0, WALL_NUMBER_MAX,
                config->wallNum, 0);
        numberStep(nd, 1, 1);

        numberField(nd, "界面宽", MIN_TERMINAL_WIDE,
                termSize.x - ROCKER_BAR_WIDTH, config->scrnWide, 0);
        numberStep(nd, 2, 1);

        numberField(nd, "界面高", MIN_TERMINAL_HIGH,
                termSize.y, config->scrnHigh, 0);
        numberStep(nd, 3, 1);

        numberField(nd, "移动速度", 0.2, 2.0,
                config->speed / 1000.0 / 1000.0, 2);
        numberStep(nd, 4, 0.05);

        numberField(nd, "系统小蛇智商", 1, 5, config->obsIQ, 0);
        numberStep(nd, 5, 1);

        numberTitle(nd, "设置 -> 更多设置");
        numberBottomRight(nd, "完成");
        numberInitial(nd, 0);
        numberBgDraw(nd, fillBackground, NULL);
        numberStyle(nd, 1, 0, 1);
        numberUseAltBuffer(nd, 0);

        usleep(40 * 1000);
        tcflush(STDIN_FILENO, TCIFLUSH);
        NumberResult res = numberRun(nd);
        if (res.confirmed) {
            config->foodNum = res.values[0];
            config->wallNum = res.values[1];
            config->scrnWide = res.values[2];
            config->scrnHigh = res.values[3];
            config->speed = res.values[4] * 1000 * 1000;
            config->obsIQ = res.values[5];

            needWrite = true;
        }

        numberFree(nd);
        numberResultFree(&res);
    } else if (selected == 3) {
        Button *b = buttonCreate();
        buttonTitle(b, "设置 -> 一键初始化");
        buttonHint(b, "确定要初始化吗？这将不可恢复");
        buttonAdd(b, "确定");
        buttonAdd(b, "我再想想");
        buttonBottomLeft(b, "取消");
        buttonBottomRight(b, "确认");
        buttonBgDraw(b, fillBackground, NULL);
        buttonInitial(b, 1);
        buttonUseAltBuffer(b, 0);

        usleep(40 * 1000);
        tcflush(STDIN_FILENO, TCIFLUSH);
        ButtonResult res = buttonRun(b);
        if (res.confirmed
                && res.bottomButton == 1
                && res.selectedTop == 0) {

            unlink(configFile);
            getGameConfig(config);
        }

        buttonFree(b);
    }

    if (needWrite) {
        setGameConfig(config);
    }
}

int showGameMenu(GameConfig *config) {
    Point termSize = terminalSize();
    Menu *menu = menuCreate();
    MenuResult res;
    int retval = 0;

    menuTab(menu, "开始");
    menuItem(menu, 0, "经典模式", '\0');
    menuItem(menu, 0, "无限食物", '\0');
    menuItem(menu, 0, "退出游戏", '\0');

    menuTab(menu, "设置");
    menuItem(menu, 1, "死亡条件", '\0');
    menuItem(menu, 1, "系统小蛇", '\0');
    menuItem(menu, 1, "更多设置", '\0');
    menuItem(menu, 1, "一键初始化", '\0');

    menuTab(menu, "关于");
    menuItem(menu, 2, "介绍 & 教程", '\0');
    menuItem(menu, 2, "关于版本", '\0');
    menuItem(menu, 2, "联系开发者", '\0');
    menuItem(menu, 2, "最终用户许可协议（EULA）", '\0');

    menuInitial(menu, 0, 0);
    menuStyle(menu, 1, 0, 1);
    menuBgDraw(menu, fillBackground, NULL);
    menuUseAltBuffer(menu, 0);

    clearScreen();

    while (!retval) {
        res = menuRun(menu);

        if (res.selectedTab == 0) {
            switch (res.selectedItem) {
                case 0:
                    retval = GAME_MODE_CLASSIC;
                    break;
                case 1:
                    retval = GAME_MODE_UNLIMIT_FOOD;
                    break;
                case 2:
                    menuFree(menu);
                    return GAME_MODE_QUIT;
            }
        } else if (res.selectedTab == 1) {
            configureGame(config, res.selectedItem);
        } else if (res.selectedTab == 2) {
            switch (res.selectedItem) {
                case 0:
                    gameIntroduction("返回");
                    break;
                case 1:
                    gameVersion();
                    break;
                case 2:
                    showDeveloper();
                    break;
                case 3:
                    showEULA("返回", "最终用户许可协议（EULA）");
                    break;
            }
        } else {
            menuFree(menu);
            return GAME_MODE_QUIT;
        }

        menuInitial(menu, res.selectedTab, res.selectedItem);
    }

    menuFree(menu);
    fillBackground(termSize.x, termSize.y, NULL);
    clearScreen();
    return retval;
}

/// Request the user to agree to the End User License Agreement
int requestUserAgreeEULA() {
    Button *b = buttonCreate();
    int retval = 0;

    buttonTitle(b, "最终用户许可协议");
    buttonHint(b, "同意协议内容后开始使用本应用");
    buttonAdd(b, "我同意");
    buttonAdd(b, "不同意");
    buttonAdd(b, "查看协议");
    buttonBgDraw(b, fillBackground, NULL);
    buttonInitial(b, 0);
    buttonUseAltBuffer(b, 0);

    showEULA("我已经了解协议内容",
             "在初次使用本应用前，您需要了解：");

    while (1) {
        ButtonResult res = buttonRun(b);

        if (res.confirmed) {
            if (res.selectedTop == 0) {
                break;
            } else if (res.selectedTop == 1) {
                retval = 1;
                break;
            } else if (res.selectedTop == 2) {
                showEULA("我已经了解协议内容",
                         "在初次使用本应用前，您需要了解：");
                buttonInitial(b, 2);
                continue;
            }
        } else {
            retval = 1;
            break;
        }
    }

    buttonFree(b);
    return retval;
}

/// Read the last n lines of the file and,
/// store them in dynamically allocated memory.
/// The caller must free the returned memory using free()
static char* readLastSeveralLines(const char *filename, int lines) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize <= 0) {
        fclose(fp);
        char *empty = malloc(1);
        if (empty) {
            empty[0] = '\0';
        }
        return empty;
    }

    int newlineCount = 0;
    long pos = fileSize - 1;
    long startPos = 0;

    while (pos >= 0 && newlineCount < lines) {
        fseek(fp, pos, SEEK_SET);
        char ch = fgetc(fp);

        if (ch == '\n') {
            newlineCount++;
            if (newlineCount == lines) {
                startPos = pos + 1;
                break;
            }
        }
        pos--;
    }

    long dataSize = fileSize - startPos;
    char *result = (char*)malloc(dataSize + 1);
    if (!result) {
        fclose(fp);
        return NULL;
    }

    if (dataSize > 0) {
        fseek(fp, startPos, SEEK_SET);
        size_t bytesRead = fread(result, 1, dataSize, fp);
        result[bytesRead] = '\0';
    } else {
        result[0] = '\0';
    }

    fclose(fp);
    return result;
}

/// Ask user whether to check app errors and display logs
void showErrorLog() {
    char time[64] = {0};
    char *logs = readLastSeveralLines(logFile, 10);
    if (logs == NULL) {
        return;
    }

    int fd = open(errSignFile, O_RDONLY);
    if (fd == -1) {
        char buf[64] = {0};

        struct stat st;
        if (stat(logFile, &st) == -1) {
            logger(LOG_ERROR, "stat: %s" HERE, strerror(errno));
            exitApp(EXIT_ERROR, "游戏出错！", NULL);
        }

        time_t sec = st.st_mtim.tv_sec;
        long usec = st.st_mtim.tv_nsec / 1000;

        struct tm tmInfo;
        localtime_r(&sec, &tmInfo);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmInfo);

        size_t len = strlen(buf);
        snprintf(buf + len, sizeof(buf) - len, ".%06ld", usec);
        snprintf(time, sizeof(time), "   When: %s", buf);
    } else {
        char buf[64] = {0};
        read(fd, buf, sizeof(buf));
        close(fd);

        snprintf(time, sizeof(time), "   %s", buf);
    }

    Button *b = buttonCreate();
    buttonTitle(b, "安全检测");
    buttonHint(b, "检测到程序上次异常退出");
    buttonAdd(b, "忽略，继续游戏");
    buttonAdd(b, "查看游戏日志");
    buttonBgDraw(b, fillBackground, NULL);
    buttonInitial(b, 0);
    buttonUseAltBuffer(b, 0);

    ButtonResult res = buttonRun(b);
    if (res.confirmed) {
        if (res.selectedTop) {
            Point termSize = terminalSize();
            fillBackground(termSize.x, termSize.y, NULL);

            TextBrowser *tb = browserCreate();
            browserHint(tb, "应用日志");
            browserPrompt(tb, time);
            browserText(tb, logs);
            browserButtonRight(tb, "返回游戏");
            browserStyle(tb, 1, 1);
            browserUseAltBuffer(tb, 0);
            browserRun(tb);

            browserFree(tb);
        }
    }

    free(logs);
    buttonFree(b);
}
