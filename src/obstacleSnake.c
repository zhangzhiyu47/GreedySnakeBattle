#include "include/Struct/GameAllRunningData.h"
#include "include/GlobalVariable/globalVariable.h"
#include "include/Functions/food.h"

#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define MAX_DIM 2048

#define DX_ARR { 0,  0, -1, 1}
#define DY_ARR {-1,  1,  0, 0}

static const int DX[4] = DX_ARR;
static const int DY[4] = DY_ARR;

typedef struct {
    uint64_t x;
    uint64_t y;
} Node;

/**
 * Check whether the coordinate is blocked.
 *
 * @param[in] ignoreObsTail true = treat obstacle snake's tail
 *            as free space(it will move away next step).
 */
static bool isBlocked(GameAllRunningData *data, uint64_t x, uint64_t y,
        bool ignoreObsTail) {

    if (x <= 1 || x >= WIDE || y <= 1 || y >= HIGH) {
        return true;
    }

    for (uint64_t i = 0; i < data->wallNum; i++) {
        if (data->wall[i].x == x && data->wall[i].y == y) {
            return true;
        }
    }

    for (uint64_t i = 0; i < data->usrSnkLeng; i++) {
        if (data->usrSnkBody[i].x == x && data->usrSnkBody[i].y == y) {
            return true;
        }
    }

    for (uint64_t i = 1; i < data->obsSnkLeng; i++) {
        if (ignoreObsTail && i == data->obsSnkLeng - 1) {
            continue;
        }
        if (data->obsSnkBody[i].x == x && data->obsSnkBody[i].y == y) {
            return true;
        }
    }

    return false;
}

/**
 * BFS to find the shortest path to target.
 *
 * @param[out] outDir First step direction index(0~3).
 * @return 1 if path found, 0 otherwise.
 */
static int bfs(GameAllRunningData *data, uint64_t tx, uint64_t ty,
        int *outDir, bool ignoreTail) {

    static bool visited[MAX_DIM][MAX_DIM];
    static Node parent[MAX_DIM][MAX_DIM];
    static int firstDir[MAX_DIM][MAX_DIM];
    static Node queue[MAX_DIM * MAX_DIM];

    for (uint64_t i = 0; i <= HIGH; i++) {
        for (uint64_t j = 0; j <= WIDE; j++) {
            visited[i][j] = false;
        }
    }

    uint64_t front = 0, rear = 0;
    uint64_t sx = data->obsSnkBody[0].x;
    uint64_t sy = data->obsSnkBody[0].y;

    queue[rear++] = (Node){sx, sy};
    visited[sy][sx] = true;
    firstDir[sy][sx] = -1;

    while (front < rear) {
        Node cur = queue[front++];

        if (cur.x == tx && cur.y == ty) {
            Node p = cur;
            while (firstDir[p.y][p.x] != -1) {
                *outDir = firstDir[p.y][p.x];
                p = parent[p.y][p.x];
            }
            return 1;
        }

        for (int d = 0; d < 4; d++) {
            uint64_t nx = (uint64_t)((int64_t)cur.x + DX[d]);
            uint64_t ny = (uint64_t)((int64_t)cur.y + DY[d]);

            if (nx < 1 || nx > WIDE || ny < 1 || ny > HIGH) {
                continue;
            }
            if (visited[ny][nx]) {
                continue;
            }
            if (isBlocked(data, nx, ny, ignoreTail)) {
                continue;
            }

            visited[ny][nx] = true;
            parent[ny][nx] = cur;
            firstDir[ny][nx] = (firstDir[cur.y][cur.x] == -1)
                    ? d : firstDir[cur.y][cur.x];
            queue[rear++] = (Node){nx, ny};
        }
    }

    return 0;
}

/**
 * Flood Fill to count reachable empty cells.
 */
static int floodFillCount(GameAllRunningData *data, uint64_t sx, uint64_t sy) {
    static bool visited[MAX_DIM][MAX_DIM];
    static Node queue[MAX_DIM * MAX_DIM];

    for (uint64_t i = 0; i <= HIGH; i++) {
        for (uint64_t j = 0; j <= WIDE; j++) {
            visited[i][j] = false;
        }
    }

    uint64_t front = 0, rear = 0;
    int count = 0;

    queue[rear++] = (Node){sx, sy};
    visited[sy][sx] = true;
    count++;

    while (front < rear) {
        Node cur = queue[front++];

        for (int d = 0; d < 4; d++) {
            uint64_t nx = (uint64_t)((int64_t)cur.x + DX[d]);
            uint64_t ny = (uint64_t)((int64_t)cur.y + DY[d]);

            if (nx < 1 || nx > WIDE || ny < 1 || ny > HIGH) {
                continue;
            }
            if (visited[ny][nx]) {
                continue;
            }
            if (isBlocked(data, nx, ny, false)) {
                continue;
            }

            visited[ny][nx] = true;
            queue[rear++] = (Node){nx, ny};
            count++;
        }
    }

    return count;
}

/**
 * Level 1: Greedy toward food, zero obstacle awareness.
 * Will happily run into walls or itself.
 */
static void obsMoveIQ1(GameAllRunningData *data) {
    int bestFood = -1;
    uint64_t minDist = WIDE + HIGH + 1;

    for (uint64_t i = 0; i < data->foodNum; i++) {
        uint64_t dx = data->food[i].x > data->obsSnkBody[0].x
                ? data->food[i].x - data->obsSnkBody[0].x
                : data->obsSnkBody[0].x - data->food[i].x;
        uint64_t dy = data->food[i].y > data->obsSnkBody[0].y
                ? data->food[i].y - data->obsSnkBody[0].y
                : data->obsSnkBody[0].y - data->food[i].y;
        uint64_t d = dx + dy;
        if (d < minDist) {
            minDist = d;
            bestFood = (int)i;
        }
    }

    if (bestFood == -1) {
        data->obsSnkNxtXDrc = data->obsSnkNxtYDrc = 0;
        return;
    }

    uint64_t tx = data->food[bestFood].x;
    uint64_t ty = data->food[bestFood].y;

    data->obsSnkNxtXDrc = 0;
    data->obsSnkNxtYDrc = 0;

    if (tx > data->obsSnkBody[0].x) {
        data->obsSnkNxtXDrc = 1;
    } else if (tx < data->obsSnkBody[0].x) {
        data->obsSnkNxtXDrc = -1;
    } else if (ty > data->obsSnkBody[0].y) {
        data->obsSnkNxtYDrc = 1;
    } else if (ty < data->obsSnkBody[0].y) {
        data->obsSnkNxtYDrc = -1;
    }
}

/**
 * Level 2: Greedy toward food, but avoids immediate death.
 * If the best direction is blocked, picks a random safe one.
 */
static void obsMoveIQ2(GameAllRunningData *data) {
    int bestFood = -1;
    uint64_t minDist = WIDE + HIGH + 1;

    for (uint64_t i = 0; i < data->foodNum; i++) {
        uint64_t dx = data->food[i].x > data->obsSnkBody[0].x
                ? data->food[i].x - data->obsSnkBody[0].x
                : data->obsSnkBody[0].x - data->food[i].x;
        uint64_t dy = data->food[i].y > data->obsSnkBody[0].y
                ? data->food[i].y - data->obsSnkBody[0].y
                : data->obsSnkBody[0].y - data->food[i].y;
        uint64_t d = dx + dy;
        if (d < minDist) {
            minDist = d;
            bestFood = (int)i;
        }
    }

    if (bestFood == -1) {
        data->obsSnkNxtXDrc = data->obsSnkNxtYDrc = 0;
        return;
    }

    uint64_t tx = data->food[bestFood].x;
    uint64_t ty = data->food[bestFood].y;
    int preferDir = -1;

    if (tx > data->obsSnkBody[0].x) {
        preferDir = 3;
    } else if (tx < data->obsSnkBody[0].x) {
        preferDir = 2;
    } else if (ty > data->obsSnkBody[0].y) {
        preferDir = 1;
    } else if (ty < data->obsSnkBody[0].y) {
        preferDir = 0;
    }

    if (preferDir != -1) {
        uint64_t nx = (uint64_t)((int64_t)data->obsSnkBody[0].x + DX[preferDir]);
        uint64_t ny = (uint64_t)((int64_t)data->obsSnkBody[0].y + DY[preferDir]);

        if (!isBlocked(data, nx, ny, true)) {
            data->obsSnkNxtXDrc = DX[preferDir];
            data->obsSnkNxtYDrc = DY[preferDir];
            return;
        }
    }

    int safe[4], n = 0;

    for (int d = 0; d < 4; d++) {
        uint64_t nx = (uint64_t)((int64_t)data->obsSnkBody[0].x + DX[d]);
        uint64_t ny = (uint64_t)((int64_t)data->obsSnkBody[0].y + DY[d]);

        if (!isBlocked(data, nx, ny, true)) {
            safe[n++] = d;
        }
    }

    if (n > 0) {
        int d = safe[rand() % n];
        data->obsSnkNxtXDrc = DX[d];
        data->obsSnkNxtYDrc = DY[d];
    } else {
        data->obsSnkNxtXDrc = data->obsSnkNxtYDrc = 0;
    }
}

/**
 * Level 3: BFS pathfinding, no tail-ignore.
 */
static void obsMoveIQ3(GameAllRunningData *data) {
    if (WIDE >= MAX_DIM || HIGH >= MAX_DIM) {
        obsMoveIQ2(data);
        return;
    }

    int bestFood = -1;
    uint64_t minDist = WIDE + HIGH + 1;

    for (uint64_t i = 0; i < data->foodNum; i++) {
        uint64_t dx = data->food[i].x > data->obsSnkBody[0].x
                ? data->food[i].x - data->obsSnkBody[0].x
                : data->obsSnkBody[0].x - data->food[i].x;
        uint64_t dy = data->food[i].y > data->obsSnkBody[0].y
                ? data->food[i].y - data->obsSnkBody[0].y
                : data->obsSnkBody[0].y - data->food[i].y;
        uint64_t d = dx + dy;
        if (d < minDist) {
            minDist = d;
            bestFood = (int)i;
        }
    }

    if (bestFood == -1) {
        obsMoveIQ1(data);
        return;
    }

    uint64_t tx = data->food[bestFood].x;
    uint64_t ty = data->food[bestFood].y;
    int dir;

    if (bfs(data, tx, ty, &dir, false)) {
        data->obsSnkNxtXDrc = DX[dir];
        data->obsSnkNxtYDrc = DY[dir];
        return;
    }

    obsMoveIQ2(data);
}

/**
 * Level 4: BFS + Flood Fill, no tail-ignore.
 */
static void obsMoveIQ4(GameAllRunningData *data) {
    if (WIDE >= MAX_DIM || HIGH >= MAX_DIM) {
        obsMoveIQ2(data);
        return;
    }

    int bestFood = -1;
    uint64_t minDist = WIDE + HIGH + 1;

    for (uint64_t i = 0; i < data->foodNum; i++) {
        uint64_t dx = data->food[i].x > data->obsSnkBody[0].x
                ? data->food[i].x - data->obsSnkBody[0].x
                : data->obsSnkBody[0].x - data->food[i].x;
        uint64_t dy = data->food[i].y > data->obsSnkBody[0].y
                ? data->food[i].y - data->obsSnkBody[0].y
                : data->obsSnkBody[0].y - data->food[i].y;
        uint64_t d = dx + dy;
        if (d < minDist) {
            minDist = d;
            bestFood = (int)i;
        }
    }

    if (bestFood == -1) {
        obsMoveIQ1(data);
        return;
    }

    uint64_t tx = data->food[bestFood].x;
    uint64_t ty = data->food[bestFood].y;
    int dir;

    if (bfs(data, tx, ty, &dir, false)) {
        data->obsSnkNxtXDrc = DX[dir];
        data->obsSnkNxtYDrc = DY[dir];
        return;
    }

    int bestDir = -1, maxSpace = -1;

    for (int d = 0; d < 4; d++) {
        uint64_t nx = (uint64_t)((int64_t)data->obsSnkBody[0].x + DX[d]);
        uint64_t ny = (uint64_t)((int64_t)data->obsSnkBody[0].y + DY[d]);

        if (data->obsSnkLeng > 1
                && nx == data->obsSnkBody[1].x
                && ny == data->obsSnkBody[1].y) {
            continue;
        }

        if (isBlocked(data, nx, ny, true)) {
            continue;
        }

        int space = floodFillCount(data, nx, ny);
        if (space > maxSpace) {
            maxSpace = space;
            bestDir = d;
        }
    }

    if (bestDir != -1) {
        data->obsSnkNxtXDrc = DX[bestDir];
        data->obsSnkNxtYDrc = DY[bestDir];
    } else {
        obsMoveIQ1(data);
    }
}

/**
 * Level 5: Full BFS (ignore tail) + Flood Fill, no flaws.
 */
static void obsMoveIQ5(GameAllRunningData *data) {
    if (WIDE >= MAX_DIM || HIGH >= MAX_DIM) {
        obsMoveIQ2(data);
        return;
    }

    int bestFood = -1;
    uint64_t minDist = WIDE + HIGH + 1;

    for (uint64_t i = 0; i < data->foodNum; i++) {
        uint64_t dx = data->food[i].x > data->obsSnkBody[0].x
                ? data->food[i].x - data->obsSnkBody[0].x
                : data->obsSnkBody[0].x - data->food[i].x;
        uint64_t dy = data->food[i].y > data->obsSnkBody[0].y
                ? data->food[i].y - data->obsSnkBody[0].y
                : data->obsSnkBody[0].y - data->food[i].y;
        uint64_t d = dx + dy;
        if (d < minDist) {
            minDist = d;
            bestFood = (int)i;
        }
    }

    if (bestFood == -1) {
        obsMoveIQ1(data);
        return;
    }

    uint64_t tx = data->food[bestFood].x;
    uint64_t ty = data->food[bestFood].y;
    int dir;

    if (bfs(data, tx, ty, &dir, true)) {
        data->obsSnkNxtXDrc = DX[dir];
        data->obsSnkNxtYDrc = DY[dir];
        return;
    }

    if (bfs(data, tx, ty, &dir, false)) {
        data->obsSnkNxtXDrc = DX[dir];
        data->obsSnkNxtYDrc = DY[dir];
        return;
    }

    int bestDir = -1, maxSpace = -1;

    for (int d = 0; d < 4; d++) {
        uint64_t nx = (uint64_t)((int64_t)data->obsSnkBody[0].x + DX[d]);
        uint64_t ny = (uint64_t)((int64_t)data->obsSnkBody[0].y + DY[d]);

        if (data->obsSnkLeng > 1
                && nx == data->obsSnkBody[1].x
                && ny == data->obsSnkBody[1].y) {
            continue;
        }

        if (isBlocked(data, nx, ny, true)) {
            continue;
        }

        int space = floodFillCount(data, nx, ny);
        if (space > maxSpace) {
            maxSpace = space;
            bestDir = d;
        }
    }

    if (bestDir != -1) {
        data->obsSnkNxtXDrc = DX[bestDir];
        data->obsSnkNxtYDrc = DY[bestDir];
        return;
    }

    int safe[4], n = 0;

    for (int d = 0; d < 4; d++) {
        uint64_t nx = (uint64_t)((int64_t)data->obsSnkBody[0].x + DX[d]);
        uint64_t ny = (uint64_t)((int64_t)data->obsSnkBody[0].y + DY[d]);

        if (!isBlocked(data, nx, ny, true)) {
            safe[n++] = d;
        }
    }

    if (n > 0) {
        int d = safe[rand() % n];
        data->obsSnkNxtXDrc = DX[d];
        data->obsSnkNxtYDrc = DY[d];
    } else {
        data->obsSnkNxtXDrc = data->obsSnkNxtYDrc = 0;
    }
}

/**
 * @brief Initialize the obstacle snake if it is enable.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsInit(GameAllRunningData *data) {
    static bool seeded = false;

    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }

    for (bool isRandWrongPos = true; isRandWrongPos;) {
        data->obsSnkBody[0].x = rand() % (WIDE - 3) + 2;
        data->obsSnkBody[0].y = rand() % (HIGH - 3) + 2;
        isRandWrongPos = false;

        for (uint64_t i = 0; i < data->wallNum; i++) {
            if (data->obsSnkBody[0].x == data->wall[i].x
                    && data->obsSnkBody[0].y == data->wall[i].y) {
                isRandWrongPos = true;
                break;
            }
        }

        if (!isRandWrongPos) {
            for (uint64_t i = 0; i < data->usrSnkLeng; i++) {
                uint64_t dx = data->obsSnkBody[0].x > data->usrSnkBody[i].x
                        ? data->obsSnkBody[0].x - data->usrSnkBody[i].x
                        : data->usrSnkBody[i].x - data->obsSnkBody[0].x;
                uint64_t dy = data->obsSnkBody[0].y > data->usrSnkBody[i].y
                        ? data->obsSnkBody[0].y - data->usrSnkBody[i].y
                        : data->usrSnkBody[i].y - data->obsSnkBody[0].y;
                if (dx < 5 && dy < 5) {
                    isRandWrongPos = true;
                    break;
                }
            }
        }
    }
}

/**
 * @brief Control the moving direction of obstacle snake.
 *
 * Dispatch to different IQ level handlers based on data->obsIQ.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsMoveDirecControl(GameAllRunningData *data) {
    switch (data->obsIQ) {
        case 1:
            obsMoveIQ1(data);
            break;
        case 2:
            obsMoveIQ2(data);
            break;
        case 3:
            obsMoveIQ3(data);
            break;
        case 4:
            obsMoveIQ4(data);
            break;
        case 5:
            obsMoveIQ5(data);
            break;
        default:
            obsMoveIQ1(data);
            break;
    }
}

/**
 * @brief The position of the moving obstacle snake.
 *
 * The position of the obstacle's last body is copied from
 * the previous body, and the coordinates of the obstacle
 * head are added to GameAllRunningData.obsSnkNxtXDrc and
 * GameAllRunningData.obsSnkNxtYDrc.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsMove(GameAllRunningData *data) {
    for (uint64_t i = data->obsSnkLeng - 1; i > 0; i--) {
        data->obsSnkBody[i].x = data->obsSnkBody[i - 1].x;
        data->obsSnkBody[i].y = data->obsSnkBody[i - 1].y;
    }
    data->obsSnkBody[0].x += data->obsSnkNxtXDrc;
    data->obsSnkBody[0].y += data->obsSnkNxtYDrc;
}

/**
 * @brief Obstacle snake eat foods.
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatFood(GameAllRunningData *data) {
    uint64_t i;

    for (i = 0; i < data->foodNum; i++) {
        if (data->obsSnkBody[0].x == data->food[i].x
                && data->obsSnkBody[0].y == data->food[i].y) {
            break;
        }
    }

    if (i == data->foodNum) {
        return;
    }

    foodInit(data, i);
    data->obsSnkLeng++;
}

/**
 * @brief Judge whether the obstacle snake eats the wall
 *        or the user snake.
 *
 * Judge whether the obstacle snake eats the wall or the
 * user snake. If obstacle snake eat them,it will change
 * the status(GameAllRunningData.obsState) of the obstacle
 * snake(to 1 or 2, see GameAllRunningData.obsState for
 * specific meaning) to describe the situation of obstacle.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatWallsOrUserSnake(GameAllRunningData *data) {
    for (uint64_t i = 0; i < data->wallNum; i++) {
        if (data->obsSnkBody[0].x == data->wall[i].x
                && data->obsSnkBody[0].y == data->wall[i].y) {
            data->obsState = 1;
            return;
        }
    }

    for (uint64_t i = 0; i < data->usrSnkLeng; i++) {
        if (data->obsSnkBody[0].x == data->usrSnkBody[i].x
                && data->obsSnkBody[0].y == data->usrSnkBody[i].y) {
            data->obsState = 2;
            data->usrSnkIsEatingObsSnk = 1;
            return;
        }
    }
}
