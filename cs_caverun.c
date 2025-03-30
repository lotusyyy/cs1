// cs_caverun.c
// Written by <INSERT YOUR FULL NAME> <INSERT YOUR ZID> on <INSERT DATE>
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

// Represents a tile/cell on the game board
struct tile_t {
    enum entity entity;
};

// Add your own structs below this line
struct world_t {
    struct tile_t board[ROWS][COLS];
    int lives;
    int num_collectible;
    int num_collected;

    char gravity;

    int win;
    int lost;

    int illumination;
    double radius;

    int score;
    int player_row;
    int player_col;
    int player_row_start;
    int player_col_start;

    int last_dash;
};

// Provided Function Prototypes
void initialise_board(struct tile_t board[ROWS][COLS]);
void print_board(struct tile_t board[ROWS][COLS], int player_row, int player_col, int lives_remaining);
void print_board_line(void);
void print_board_header(int lives);
void print_map_statistics(int number_of_dirt_tiles, int number_of_gem_tiles, int number_of_boulder_tiles,
        double completion_percentage, int maximum_points_remaining);

// Add your function prototypes below this line
void setup(struct world_t *world);
void game_loop(struct world_t *world);
int is_valid_position(int row, int col);
int add_walls(struct world_t *world, int row1, int col1, int row2, int col2);

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
    if (count_entities(world, GEM) == 0) {
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                if (world->board[row][col].entity == EXIT_LOCKED) {
                    world->board[row][col].entity = EXIT_UNLOCKED;
                }
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
    } else if (world->board[nrow][ncol].entity == BOULDER || world->board[nrow][ncol].entity == WALL
            || world->board[nrow][ncol].entity == EXIT_LOCKED) {

        return TRUE;
    } else if (world->board[nrow][ncol].entity == DIRT) {
        world->board[nrow][ncol].entity = EMPTY;
        world->player_row = nrow;
        world->player_col = ncol;
        world->score += SCORE_DIRT;
        world->num_collected++;
    } else if (world->board[nrow][ncol].entity == GEM) {
        world->board[nrow][ncol].entity = EMPTY;
        world->player_row = nrow;
        world->player_col = ncol;
        world->score += SCORE_GEM;
        world->num_collected++;

        try_unlock(world);
    } else {
        world->player_row = nrow;
        world->player_col = ncol;
    }

    if (world->board[world->player_row][world->player_col].entity == EXIT_UNLOCKED) {
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

    completion_percentage = 100.0 * world->num_collected / world->num_collectible;
    print_map_statistics(number_of_dirt_tiles, number_of_gem_tiles, number_of_boulder_tiles, completion_percentage,
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

void boulder_move_to(struct world_t *world, int row1, int col1, int row2, int col2) {
    if (is_valid_position(row1, col1) && is_valid_position(row2, col2) && world->board[row1][col1].entity == BOULDER
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
            if (world->board[row][col].entity == BOULDER && row == world->player_row && col == world->player_col) {
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

    if (world->board[row][col].entity == EMPTY) {
        printf("Respawning!\n");
        world->player_row = world->player_row_start;
        world->player_col = world->player_col_start;
        blocked = FALSE;
    } else {
        printf("Respawn blocked! Game over. Final score: %d points.\n", world->score);
    }

    return blocked;
}

void print_game_board(struct world_t *world) {
    if (world->illumination) {
        struct tile_t board[ROWS][COLS];
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                double distance = sqrt(pow(row - world->player_row, 2) + pow(col - world->player_col, 2));
                if (distance <= world->radius) {
                    board[row][col].entity = world->board[row][col].entity;
                } else {
                    board[row][col].entity = HIDDEN;
                }
            }
        }

        print_board(board, world->player_row, world->player_col, world->lives);
    } else {
        print_board(world->board, world->player_row, world->player_col, world->lives);
    }
}

void handler_bounder(struct world_t *world){
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

    if (command == 'w' || command == 'a' || command == 's' || command == 'd') {
        world->last_dash = FALSE;
        move_player(world, command);
    } else if (command == 'r') {
        world->last_dash = FALSE;
    } else if (strlen(input) == 2 && is_in(input[0], "WASD") && is_in(input[1], "WASD")) {
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

void game_loop(struct world_t *world) {
    char input[20];

    printf("--- Gameplay Phase ---\n");

    while (!world->win && !world->lost && scanf("%s", input) == 1 && input[0] != 'q') {
        char command = input[0];
        if (command == 'p') {
            printf("You have %d point(s)!\n", world->score);
        } else if (command == 'm') {
            print_statistics(world);
        } else if (command == 'i') {
            scanf("%lf", &world->radius);
            if (world->radius > 0) {
                world->illumination = TRUE;
                printf("Illumination Mode: Activated\n");
            } else {
                world->illumination = FALSE;
                printf("Illumination Mode: Deactivated\n");
            }
            print_game_board(world);
        } else if (command == 'g') {
            scanf(" %c", &world->gravity);
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
        } else {
            step(world, input);
        }
    }

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

            if (world->board[row][col].entity != DIRT || (row == world->player_row && col == world->player_col)) {
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
    //enter map feature
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
        } else if (world->board[row][col].entity != DIRT || (row == world->player_row && col == world->player_col)) {
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
            }
        }
    }
    try_unlock(world);

    print_board(world->board, world->player_row, world->player_col, world->lives);
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

    initialise_board(world->board);

    printf("--- Game Setup Phase ---\n");
    printf("Enter the player's starting position: ");
    scanf("%d%d", &world->player_row, &world->player_col);
    while (!is_valid_position(world->player_row, world->player_col)) {
        printf("Position %d %d is invalid!\n", world->player_row, world->player_col);

        printf("Enter the player's starting position: ");
        scanf("%d%d", &world->player_row, &world->player_col);
    }
    world->player_row_start = world->player_row;
    world->player_col_start = world->player_col;

    print_board(world->board, world->player_row, world->player_col, world->lives);

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
void print_board(struct tile_t board[ROWS][COLS], int player_row, int player_col, int lives_remaining) {
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
void print_map_statistics(int number_of_dirt_tiles, int number_of_gem_tiles, int number_of_boulder_tiles,
        double completion_percentage, int maximum_points_remaining) {
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
