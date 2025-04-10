// cs_caverun.c
// Written by <INSERT YOUR FULL NAME> z5621509 on <INSERT DATE>
//
// Description: This file implements a simplification of game mechanics
//              explored by the 1980s 8-bit video game Boulder Dash.
//              In this turn-based re-imagination: the player tunnels
//              through dirt to collect gems, all while avoiding falling
//              boulders in an attempt to reach an unlocked exit with the
//              most points. But beware! For those of you brave enough
//              to venture into the deep: shadow and fire awaits!

// Provided Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Add your own #include statements below this line

// Provided constants
#define COLS 10
#define ROWS 10
#define INVALID_ROW -1
#define INVALID_COL -1
#define INITIAL_LIVES 3
#define SCORE_DIRT 1
#define SCORE_GEM 20

// Add your own #defines constants below this line
#define TRUE 1
#define FALSE 0

// Provided Enums
// Enum for features on the game board
enum entity {
    EMPTY, DIRT, WALL, BOULDER, GEM, EXIT_LOCKED, EXIT_UNLOCKED, HIDDEN, LAVA
};

// Add your own enums below this line
enum lava_mode {
    NONE, GAME, SEED
};

// Represents a tile/cell on the game board
struct tile_t {
    enum entity entity;
};

// Add your own structs below this line
struct world_t {
    struct tile_t board[ROWS][COLS];
    int lavas[ROWS][COLS];

    int lives;
    int num_collectible;
    int num_collected;

    char gravity;

    int win;
    int lost;

    int illumination;
    int shadow;
    double radius;

    int score;
    int player_row;
    int player_col;
    int player_row_start;
    int player_col_start;

    int last_dash;

    enum lava_mode mode;
};

struct point_t {
    double x;
    double y;
};

struct stat_t {
    int rows[ROWS * COLS];
    int cols[ROWS * COLS];
    int types[ROWS * COLS];
    struct point_t start[ROWS * COLS];
    struct point_t end[ROWS * COLS];
    int num;
};

// Provided Function Prototypes
void initialise_board(struct tile_t board[ROWS][COLS]);
void print_board(struct tile_t board[ROWS][COLS], int player_row,
        int player_col, int lives_remaining);
void print_board_line(void);
void print_board_header(int lives);
void print_map_statistics(int number_of_dirt_tiles, int number_of_gem_tiles,
        int number_of_boulder_tiles, double completion_percentage,
        int maximum_points_remaining);

// Add your function prototypes below this line
int count_max_points_remain(struct world_t *world);
int count_entities(struct world_t *world, enum entity type);
void try_unlock(struct world_t *world);
void get_offset(char command, int *drow, int *dcol);
int move_player(struct world_t *world, char command);
void print_statistics(struct world_t *world);
int is_in(char ch, const char *chars);
void dash_move_player(struct world_t *world, const char *input);
void boulder_move_to(struct world_t *world, int row1, int col1, int row2,
        int col2);
int boulder_move(struct world_t *world);
int spawn_player(struct world_t *world);
int ccw(struct point_t a, struct point_t b, struct point_t c);
int intersect(struct point_t a, struct point_t b, struct point_t c,
        struct point_t d);
int side_on(struct point_t a, struct point_t b, struct point_t c);
int point_online(struct point_t a, struct point_t b, struct point_t c);
int same_side(struct point_t a, struct point_t b, struct point_t p1,
        struct point_t p2);
int is_wall(struct world_t *world, int row1, int col1);
int is_neighbor(int x1, int y1, int x2, int y2);
void get_blocks(struct world_t *world, int x1, int y1, int x2, int y2,
        struct stat_t *stat);
void set_start_end(int i, struct stat_t *stat, int sub_value1, int sub_value2,
        int sub_value3, int sub_value4, struct point_t point1,
        struct point_t point2, struct point_t point3, struct point_t point4);
void get_block_type(struct world_t *world, int x1, int y1, int x2, int y2,
        struct stat_t *stat);
int isblocked(struct world_t *world, int x1, int y1, int x2, int y2);
void print_game_board(struct world_t *world);
int count_neighbors(struct world_t *world, int row, int col);
void move_lavas(struct world_t *world);
void handler_bounder(struct world_t *world);
void step(struct world_t *world, const char *input);
void shift(char *buffer);
void trim(char *input);
void illumination(struct world_t *world);
void set_gravity(struct world_t *world);
void set_lava(struct world_t *world, const char *buffer);
void game_loop(struct world_t *world);
int is_valid_position(int row, int col);
int add_walls(struct world_t *world, int row1, int col1, int row2, int col2);
void setup_feature(struct world_t *world);
void setup(struct world_t *world);

// Provided sample main() function (you will need to modify this)
int main(void) {
    printf("Welcome to CS Caverun!\n\n");

    struct world_t world;
    setup(&world);
    game_loop(&world);

    return 0;
}

// Add your function definitions below this line
int count_max_points_remain(struct world_t *world) {
    int sum = 0;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (world->board[row][col].entity == DIRT) {
                sum += SCORE_DIRT;
            }
            if (world->board[row][col].entity == GEM) {
                sum += SCORE_GEM;
            }
        }
    }

    return sum;
}

int count_entities(struct world_t *world, enum entity type) {
    int sum = 0;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (world->board[row][col].entity == type) {
                sum++;
            }
        }
    }

    return sum;
}

void try_unlock(struct world_t *world) {
    if (count_entities(world, GEM) == 0)
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (world->board[row][col].entity == EXIT_LOCKED) {
                world->board[row][col].entity = EXIT_UNLOCKED;
            }
        }
    }
}

void get_offset(char command, int *drow, int *dcol) {
    *drow = 0;
    *dcol = 0;
    if (command == 'w') {
        *drow = -1;
    }
    if (command == 's') {
        *drow = 1;
    }
    if (command == 'a') {
        *dcol = -1;
    }
    if (command == 'd') {
        *dcol = 1;
    }
}

int move_player(struct world_t *world, char command) {
    int drow, dcol;
    int nrow, ncol;

    get_offset(command, &drow, &dcol);
    nrow = world->player_row + drow;
    ncol = world->player_col + dcol;

    if (!is_valid_position(nrow, ncol)) {

        return TRUE;
    } else if (world->board[nrow][ncol].entity == BOULDER
            || world->board[nrow][ncol].entity == WALL
            || world->board[nrow][ncol].entity == EXIT_LOCKED) {

        return TRUE;
    } else if (world->board[nrow][ncol].entity == DIRT) {
        world->board[nrow][ncol].entity = EMPTY;
        world->player_row = nrow;
        world->player_col = ncol;
        world->score += SCORE_DIRT;
        if (world->mode != NONE) {
            world->score += SCORE_DIRT * 9;
        }
        world->num_collected++;
    } else if (world->board[nrow][ncol].entity == GEM) {
        world->board[nrow][ncol].entity = EMPTY;
        world->player_row = nrow;
        world->player_col = ncol;
        world->score += SCORE_GEM;
        if (world->mode != NONE) {
            world->score += SCORE_GEM * 9;
        }
        world->num_collected++;

        try_unlock(world);
    } else {
        world->player_row = nrow;
        world->player_col = ncol;
    }

    if (world->board[world->player_row][world->player_col].entity
            == EXIT_UNLOCKED) {
        world->win = TRUE;
    }
    return FALSE;
}

void print_statistics(struct world_t *world) {
    int number_of_dirt_tiles = 0;
    int number_of_gem_tiles = 0;
    int number_of_boulder_tiles = 0;
    double completion_percentage = 0;
    int maximum_points_remaining = 0;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (world->board[row][col].entity == DIRT) {
                number_of_dirt_tiles++;
                maximum_points_remaining += SCORE_DIRT;
            }
            if (world->board[row][col].entity == GEM) {
                maximum_points_remaining += SCORE_GEM;
                number_of_gem_tiles++;
            }
            if (world->board[row][col].entity == BOULDER) {
                number_of_boulder_tiles++;
            }
        }
    }

    if (world->mode != NONE) {
        maximum_points_remaining *= 10;
    }

    completion_percentage = 100.0 * world->num_collected
            / world->num_collectible;
    print_map_statistics(number_of_dirt_tiles, number_of_gem_tiles,
            number_of_boulder_tiles, completion_percentage,
            maximum_points_remaining);
}

int is_in(char ch, const char *chars) {
    int len = strlen(chars);
    for (int i = 0; i < len; i++) {
        if (chars[i] == ch) {
            return TRUE;
        }
    }
    return FALSE;
}

void dash_move_player(struct world_t *world, const char *input) {
    char command = input[0];
    char command2 = input[1];
    command += 'a' - 'A';
    command2 += 'a' - 'A';

    if (!move_player(world, command)) {
        if (!world->win) {
            move_player(world, command2);
        }
    }
}

void boulder_move_to(struct world_t *world, int row1, int col1, int row2,
        int col2) {
    if (is_valid_position(row1, col1) && is_valid_position(row2, col2)
            && world->board[row1][col1].entity == BOULDER
            && world->board[row2][col2].entity == EMPTY) {
        world->board[row1][col1].entity = EMPTY;
        world->board[row2][col2].entity = BOULDER;
    }
}

int boulder_move(struct world_t *world) {
    int drow, dcol;
    get_offset(world->gravity, &drow, &dcol);

    if (world->gravity == 's') {
        for (int row = ROWS - 1; row >= 0; row--) {
            for (int col = 0; col < COLS; col++) {
                boulder_move_to(world, row, col, row + drow, col + dcol);
            }
        }
    }

    if (world->gravity == 'w') {
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                boulder_move_to(world, row, col, row + drow, col + dcol);
            }
        }
    }

    if (world->gravity == 'a') {
        for (int col = 0; col < COLS; col++) {
            for (int row = 0; row < ROWS; row++) {
                boulder_move_to(world, row, col, row + drow, col + dcol);
            }
        }
    }

    if (world->gravity == 'd') {
        for (int col = COLS - 1; col >= 0; col--) {
            for (int row = 0; row < ROWS; row++) {
                boulder_move_to(world, row, col, row + drow, col + dcol);
            }
        }
    }

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (world->board[row][col].entity == BOULDER
                    && row == world->player_row && col == world->player_col) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int spawn_player(struct world_t *world) {
    int row = world->player_row_start;
    int col = world->player_col_start;
    int blocked = TRUE;

    if (world->lavas[row][col]) {
        printf("Respawn blocked! You're toast! Final score: %d points.\n",
                world->score);
    } else if (world->board[row][col].entity == EMPTY) {
        printf("Respawning!\n");
        world->player_row = world->player_row_start;
        world->player_col = world->player_col_start;
        blocked = FALSE;
    } else {
        printf("Respawn blocked! Game over. Final score: %d points.\n",
                world->score);
    }

    return blocked;
}

int ccw(struct point_t a, struct point_t b, struct point_t c) {
    return (c.y - a.y) * (b.x - a.x) > (b.y - a.y) * (c.x - a.x);
}

int intersect(struct point_t a, struct point_t b, struct point_t c,
        struct point_t d) {
    return ccw(a, c, d) != ccw(b, c, d) && ccw(a, b, c) != ccw(a, b, d);
}

int side_on(struct point_t a, struct point_t b, struct point_t c) {
    return ccw(a, c, c) != ccw(b, c, c) && ccw(a, b, c) != ccw(a, b, c);
}

int point_online(struct point_t a, struct point_t b, struct point_t c) {
    double var_m = (b.y - a.y) / (b.x - a.x);
    double var_n = b.y - var_m * b.x;
    double var_v = var_m * c.x + var_n;

    return fabs(c.y - var_v) < 0.0001;
}

int same_side(struct point_t a, struct point_t b, struct point_t p1,
        struct point_t p2) {
    double var_f1 = (a.y - b.y) * (p1.x - a.x) + (b.x - a.x) * (p1.y - a.y);
    double var_f2 = (a.y - b.y) * (p2.x - a.x) + (b.x - a.x) * (p2.y - a.y);
    return var_f1 * var_f2 > 0;
}

int is_wall(struct world_t *world, int row1, int col1) {

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if ((world->board[row][col].entity == WALL
                    || world->board[row][col].entity == BOULDER
                    || world->board[row][col].entity == GEM) && row == row1
                    && col == col1) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int is_neighbor(int x1, int y1, int x2, int y2) {
    int distance = abs(x1 - x2) + abs(y1 - y2);
    return distance == 2 && x1 != x2 && y1 != y2;
}

void get_blocks(struct world_t *world, int x1, int y1, int x2, int y2,
        struct stat_t *stat) {
    struct point_t point_a = {
        x1, y1
    };
    struct point_t point_b = {
        x2, y2
    };

    for (int row = 0; row < ROWS; row++)
        for (int col = 0; col < COLS; col++) {
        if ((world->board[row][col].entity == WALL
                || world->board[row][col].entity == BOULDER
                || world->board[row][col].entity == GEM)
                && !(row == y2 && col == x2)) {
            double x = col;
            double y = row;

            struct point_t var_p1 = {
                x - 0.51, y - 0.51
            };
            struct point_t var_p2 = {
                x + 0.51, y - 0.51
            };
            struct point_t var_p3 = {
                x + 0.51, y + 0.51

            };
            struct point_t var_p4 = {
                x - 0.51, y + 0.51
            };

            if (intersect(point_a, point_b, var_p1, var_p2)
                    || intersect(point_a, point_b, var_p2, var_p3)
                    || intersect(point_a, point_b, var_p3, var_p4)
                    || intersect(point_a, point_b, var_p4, var_p1)) {
                stat->rows[stat->num] = row;
                stat->cols[stat->num] = col;
                stat->types[stat->num] = 0;
                stat->num++;
            }
        }
    }
}

void set_start_end(int i, struct stat_t *stat, int sub_value1, int sub_value2,
        int sub_value3, int sub_value4, struct point_t point1,
        struct point_t point2, struct point_t point3, struct point_t point4) {
    stat->types[i] = 1;
    if (sub_value1) {
        stat->start[i] = point1;
        stat->end[i] = point2;
    }
    if (sub_value2) {
        stat->start[i] = point2;
        stat->end[i] = point3;
    }
    if (sub_value3) {
        stat->start[i] = point3;
        stat->end[i] = point4;
    }
    if (sub_value4) {
        stat->start[i] = point4;
        stat->end[i] = point1;
    }
}

void get_block_type(struct world_t *world, int x1, int y1, int x2, int y2,
        struct stat_t *stat) {
    struct point_t point_a = {
        x1, y1
    };
    struct point_t point_b = {
        x2, y2
    };
    for (int i = 0; i < stat->num; i++) {
        double x = stat->cols[i];
        double y = stat->rows[i];
        struct point_t point1 = {
            x - 0.5, y - 0.5
        };
        struct point_t point2 = {
            x + 0.5, y - 0.5
        };
        struct point_t point3 = {
            x + 0.5, y + 0.5
        };
        struct point_t point4 = {
            x - 0.5, y + 0.5
        };
        int sub_value1 = point_online(point_a, point_b, point1);
        int sub_value2 = point_online(point_a, point_b, point2);
        int sub_value3 = point_online(point_a, point_b, point3);
        int sub_value4 = point_online(point_a, point_b, point4);
        int sum1 = sub_value1 + sub_value2 + sub_value3 + sub_value4;
        sub_value1 = same_side(point_a, point_b, point1, point2);
        sub_value2 = same_side(point_a, point_b, point2, point3);
        sub_value3 = same_side(point_a, point_b, point3, point4);
        sub_value4 = same_side(point_a, point_b, point4, point1);
        int sum2 = sub_value1 + sub_value2 + sub_value3 + sub_value4;
        if (sum1 == 1 && sum2 == 2) {
            set_start_end(i, stat, sub_value1, sub_value2, sub_value3,
                    sub_value4, point1, point2, point3, point4);
        }
    }
}

int isblocked(struct world_t *world, int x1, int y1, int x2, int y2) {
    struct point_t point_a = {
        x1, y1
    };
    struct point_t point_b = {
        x2, y2
    };

    struct stat_t stat;
    stat.num = 0;

    get_blocks(world, x1, y1, x2, y2, &stat);
    get_block_type(world, x1, y1, x2, y2, &stat);

    int count = 0;
    for (int i = 0; i < stat.num; i++) {
        if (stat.types[i] == 0) {
            count++;
        }
    }

    if (count > 0) {
        return TRUE;
    }

    count = 0;
    for (int i = 0; i < stat.num; i++) {
        for (int j = 0; j < stat.num; j++) {
            if (stat.types[i] == 1 && stat.types[j] == 1 && i != j) {
                if (!same_side(point_a, point_b, stat.start[i], stat.end[j]))
                    count++;
            }
        }
    }

    return count > 0;
}

void print_game_board(struct world_t *world) {
    struct tile_t board[ROWS][COLS];

    int var_y1 = world->player_row;
    int var_x1 = world->player_col;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            double distance = sqrt(
                    pow(row - world->player_row, 2)
                            + pow(col - world->player_col, 2));
            if (world->lavas[row][col]) {
                board[row][col].entity = LAVA;
                //printf("%d %d is lava\n", row, col);
            } else if (distance <= world->radius) {
                board[row][col].entity = world->board[row][col].entity;
            } else if (world->illumination) {
                board[row][col].entity = HIDDEN;
            } else {
                board[row][col].entity = world->board[row][col].entity;
            }
        }
    }

    //shadow
    if (world->shadow) {
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                if (board[row][col].entity != LAVA
                        && board[row][col].entity != HIDDEN
                        && isblocked(world, var_x1, var_y1, col, row))
                    board[row][col].entity = HIDDEN;
            }
        }
    }

    print_board(board, world->player_row, world->player_col, world->lives);
}

int count_neighbors(struct world_t *world, int row, int col) {
    int sum = 0;

    for (int drow = -1; drow <= 1; drow++) {
        for (int dcol = -1; dcol <= 1; dcol++) {
            if (drow != 0 || dcol != 0) {
                int next_row = row + drow;
                int next_col = col + dcol;

                next_row = (next_row + ROWS) % ROWS;
                next_col = (next_col + COLS) % COLS;

                if (world->lavas[next_row][next_col])
                    sum++;
            }
        }
    }

    return sum;
}

void move_lavas(struct world_t *world) {
    int next_lavas[ROWS][COLS];

    for (int row = 0; row < ROWS; row++)
        for (int col = 0; col < COLS; col++) {
        next_lavas[row][col] = world->lavas[row][col];
        int neighbors = count_neighbors(world, row, col);
        if (world->mode == GAME) {
            if (!world->lavas[row][col]) {
                next_lavas[row][col] = neighbors == 3;
            } else if (neighbors < 2 || neighbors > 3) {
                next_lavas[row][col] = FALSE;
            } else {
                next_lavas[row][col] = TRUE;
            }
        } else if (world->mode == SEED) {
            if (!world->lavas[row][col]) {
                next_lavas[row][col] = neighbors == 2;
            } else {
                next_lavas[row][col] = FALSE;
            }
        }
    }

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            world->lavas[row][col] = next_lavas[row][col];
        }
    }
}

void handler_bounder(struct world_t *world) {
    int lose = boulder_move(world);
    int blocked = FALSE;

    if (lose) {
        world->lives--;
        if (world->lives == 0) {
            world->lost = TRUE;
        } else {
            blocked = spawn_player(world);
        }
    }

    if (!world->win && !world->lost && !blocked) {
        move_lavas(world);

        if (world->lavas[world->player_row][world->player_col]) {
            world->lives--;
            if (world->lives == 0) {
                world->lost = TRUE;
            } else {
                blocked = spawn_player(world);
            }
        }
    }

    if (world->lost) {
        printf("Game Lost! You scored %d points!\n", world->score);
    }

    print_game_board(world);
    if (world->win) {
        printf("You Win! Final Score: %d point(s)!\n", world->score);
    }

    if (blocked) {
        world->lost = TRUE;
    }
}

void step(struct world_t *world, const char *input) {
    char command = input[0];

    int tag = command == 'w' || command == 'a' || command == 's'
            || command == 'd';
    if (tag) {
        world->last_dash = FALSE;
        move_player(world, command);
    } else if (command == 'r') {
        world->last_dash = FALSE;
    } else if (strlen(input) == 2 && is_in(input[0], "WASD")
            && is_in(input[1], "WASD")) {
        if (world->last_dash) {
            printf("You're out of breath! Skipping dash move...\n");
            world->last_dash = FALSE;
        } else {
            dash_move_player(world, input);
            world->last_dash = TRUE;
        }
    }

    handler_bounder(world);
}

void shift(char *buffer) {
    for (int i = 1; i < 4; i++) {
        buffer[i - 1] = buffer[i];
    }
}

void trim(char *input) {
    int len = strlen(input);
    while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
        input[len - 1] = 0;
        len = strlen(input);
    }
}

void illumination(struct world_t *world) {
    if (world->radius > 0) {
        world->illumination = TRUE;
        printf("Illumination Mode: Activated\n");
    } else {
        world->illumination = FALSE;
        printf("Illumination Mode: Deactivated\n");
    }
    print_game_board(world);
}

void set_gravity(struct world_t *world) {
    if (world->gravity == 'w') {
        printf("Gravity now pulls UP!\n");
    }
    if (world->gravity == 'a') {
        printf("Gravity now pulls LEFT!\n");
    }
    if (world->gravity == 's') {
        printf("Gravity now pulls DOWN!\n");
    }
    if (world->gravity == 'd') {
        printf("Gravity now pulls RIGHT!\n");
    }

    handler_bounder(world);
}

void set_lava(struct world_t *world, const char *buffer) {
    if (strcmp("wdsa", buffer) == 0) {
        world->mode = GAME;
        printf("Game Of Lava: Activated\n");
    }
    if (strcmp("wasd", buffer) == 0) {
        world->mode = SEED;
        printf("Lava Seeds: Activated\n");
    }
}

void game_loop(struct world_t *world) {
    char buffer[5] = { 0, 0, 0, 0, 0 };
    char input[50];

    printf("--- Gameplay Phase ---\n");

    fgets(input, 50, stdin);
    while (!world->win && !world->lost && fgets(input, 50, stdin)
            && input[0] != 'q') {
        trim(input);
        char command = input[0];

        if (command != 'L') {
            shift(buffer);
            buffer[3] = command;
        }

        if (command == 'p') {
            printf("You have %d point(s)!\n", world->score);
        } else if (command == 'm') {
            print_statistics(world);
        } else if (command == 'i') {
            sscanf(input + 1, "%lf", &world->radius);
            illumination(world);
        } else if (command == 'g') {
            sscanf(input + 1, " %c", &world->gravity);
            set_gravity(world);
        } else if (command == 'L') {
            set_lava(world, buffer);
            shift(buffer);
            buffer[3] = command;
        } else if (command == 'u') {
            if (world->shadow) {
                printf("Shadow Mode: Deactivated\n");
            } else {
                printf("Shadow Mode: Activated\n");
            }
            world->shadow = !world->shadow;
            print_game_board(world);
        } else {
            step(world, input);
        }
    }

    trim(input);
    if (strcmp("q", input) == 0) {
        printf("--- Quitting Game ---\n");
    }
}

int is_valid_position(int row, int col) {
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

int add_walls(struct world_t *world, int row1, int col1, int row2, int col2) {
    for (int row = row1; row <= row2; row++) {
        for (int col = col1; col <= col2; col++) {
            if (!is_valid_position(row, col)) {
                return FALSE;
            }

            if (world->board[row][col].entity != DIRT
                    || (row == world->player_row && col == world->player_col)) {
                return FALSE;
            }
        }
    }

    for (int row = row1; row <= row2; row++) {
        for (int col = col1; col <= col2; col++) {
            world->board[row][col].entity = WALL;
        }
    }

    return TRUE;
}

void setup_feature(struct world_t *world) {
    // enter map feature
    printf("Enter map features:\n");
    char type;
    int row, col, row2, col2;

    while (scanf(" %c", &type) == 1 && type != 's') {
        scanf("%d%d", &row, &col);

        if (type == 'W') {
            scanf("%d%d", &row2, &col2);
            if (!add_walls(world, row, col, row2, col2)) {
                printf("Invalid location: feature cannot be placed here!\n");
            }
        } else if (!is_valid_position(row, col)) {
            printf("Invalid location: position is not on map!\n");
        } else if (world->board[row][col].entity != DIRT
                || (row == world->player_row && col == world->player_col)) {
            printf("Invalid location: tile is occupied!\n");
        } else {
            if (type == 'w') {
                world->board[row][col].entity = WALL;
            } else if (type == 'b') {
                world->board[row][col].entity = BOULDER;
            } else if (type == 'g') {
                world->board[row][col].entity = GEM;
            } else if (type == 'e') {
                world->board[row][col].entity = EXIT_LOCKED;
            } else if (type == 'l') {
                world->lavas[row][col] = TRUE;
            }
        }
    }
    try_unlock(world);

    print_game_board(world);
    world->board[world->player_row][world->player_col].entity = EMPTY;

    world->num_collectible += count_entities(world, DIRT);
    world->num_collectible += count_entities(world, GEM);
}

void setup(struct world_t *world) {
    world->win = FALSE;
    world->lost = FALSE;
    world->illumination = FALSE;

    world->score = 0;
    world->lives = INITIAL_LIVES;
    world->num_collectible = 0;
    world->num_collected = 0;
    world->last_dash = FALSE;
    world->gravity = 's';
    world->mode = NONE;
    world->radius = 0;
    world->shadow = FALSE;

    initialise_board(world->board);

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            world->lavas[row][col] = FALSE;
        }
    }

    printf("--- Game Setup Phase ---\n");
    printf("Enter the player's starting position: ");
    scanf("%d%d", &world->player_row, &world->player_col);
    while (!is_valid_position(world->player_row, world->player_col)) {
        printf("Position %d %d is invalid!\n", world->player_row,
                world->player_col);

        printf("Enter the player's starting position: ");
        scanf("%d%d", &world->player_row, &world->player_col);
    }
    world->player_row_start = world->player_row;
    world->player_col_start = world->player_col;

    print_board(world->board, world->player_row, world->player_col,
            world->lives);

    setup_feature(world);

}

// =============================================================================
// Definitions of Provided Functions
// =============================================================================

// Given a 2D board array, initialise all tile entities to DIRT.
void initialise_board(struct tile_t board[ROWS][COLS]) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            board[row][col].entity = DIRT;
        }
    }
}

// Prints the game board, showing the player's position and lives remaining
void print_board(struct tile_t board[ROWS][COLS], int player_row,
        int player_col, int lives_remaining) {
    print_board_line();
    print_board_header(lives_remaining);
    print_board_line();
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            printf("|");
            if (row == player_row && col == player_col) {
                printf("^_^");
            } else if (board[row][col].entity == EMPTY) {
                printf("   ");
            } else if (board[row][col].entity == DIRT) {
                printf(" . ");
            } else if (board[row][col].entity == WALL) {
                printf("|||");
            } else if (board[row][col].entity == BOULDER) {
                printf("(O)");
            } else if (board[row][col].entity == GEM) {
                printf("*^*");
            } else if (board[row][col].entity == EXIT_LOCKED) {
                printf("[X]");
            } else if (board[row][col].entity == EXIT_UNLOCKED) {
                printf("[ ]");
            } else if (board[row][col].entity == HIDDEN) {
                printf(" X ");
            } else if (board[row][col].entity == LAVA) {
                printf("^^^");
            } else {
                printf("   ");
            }
        }
        printf("|\n");
        print_board_line();
    }
    printf("\n");
    return;
}

// Helper function for print_board(). You will not need to call this.
void print_board_header(int lives) {
    printf("| Lives: %d    C A V E R U N             |\n", lives);
}

// Helper function for print_board(). You will not need to call this.
void print_board_line(void) {
    printf("+");
    for (int col = 0; col < COLS; col++) {
        printf("---+");
    }
    printf("\n");
}

// Prints game statistics: tile types, completion %, and points remaining.
void print_map_statistics(int number_of_dirt_tiles, int number_of_gem_tiles,
        int number_of_boulder_tiles, double completion_percentage,
        int maximum_points_remaining) {
    printf("========= Map Statistics =========\n");
    printf("Tiles Remaining by Type:\n");
    printf("  - DIRT:      %d\n", number_of_dirt_tiles);
    printf("  - GEMS:      %d\n", number_of_gem_tiles);
    printf("  - BOULDERS:  %d\n", number_of_boulder_tiles);
    printf("Completion Status:\n");
    printf("  - Collectible Completion: %.1f%%\n", completion_percentage);
    printf("  - Maximum Points Remaining: %d\n", maximum_points_remaining);
    printf("==================================\n");
}
