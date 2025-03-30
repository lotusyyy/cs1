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

    int win;
    int score;
    int player_row;
    int player_col;
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

void move_player(struct world_t *world, char command) {
    int drow, dcol;
    int nrow, ncol;

    drow = 0;
    dcol = 0;
    if (command == 'w') {
        drow = -1;
    }
    if (command == 's') {
        drow = 1;
    }
    if (command == 'a') {
        dcol = -1;
    }
    if (command == 'd') {
        dcol = 1;
    }

    nrow = world->player_row + drow;
    ncol = world->player_col + dcol;

    if (!is_valid_position(nrow, ncol)) {

    } else if (world->board[nrow][ncol].entity == BOULDER || world->board[nrow][ncol].entity == WALL
            || world->board[nrow][ncol].entity == EXIT_LOCKED) {

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

    print_board(world->board, world->player_row, world->player_col, world->lives);
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

void game_loop(struct world_t *world) {
    char command = 0;

    printf("--- Gameplay Phase ---\n");

    while (scanf(" %c", &command) == 1 && command != 'q' && !world->win) {
        if (command == 'w' || command == 'a' || command == 's' || command == 'd') {
            move_player(world, command);

            if (world->board[world->player_row][world->player_col].entity == EXIT_UNLOCKED) {
                printf("You Win! Final Score: %d point(s)!\n", world->score);
                world->win = TRUE;
            }
        } else if (command == 'r') {
            print_board(world->board, world->player_row, world->player_col, world->lives);
        } else if (command == 'p') {
            printf("You have %d point(s)!\n", world->score);
        } else if (command == 'm') {
            print_statistics(world);
        }
    }

    if (command == 'q') {
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
    print_board(world->board, world->player_row, world->player_col, world->lives);
    world->board[world->player_row][world->player_col].entity = EMPTY;

    world->num_collectible += count_entities(world, DIRT);
    world->num_collectible += count_entities(world, GEM);
}

void setup(struct world_t *world) {
    world->win = FALSE;
    world->score = 0;
    world->lives = INITIAL_LIVES;
    world->num_collectible = 0;
    world->num_collected = 0;

    initialise_board(world->board);

    printf("--- Game Setup Phase ---\n");
    printf("Enter the player's starting position: ");
    scanf("%d%d", &world->player_row, &world->player_col);
    while (!is_valid_position(world->player_row, world->player_col)) {
        printf("Position %d %d is invalid!\n", world->player_row, world->player_col);

        printf("Enter the player's starting position: ");
        scanf("%d%d", &world->player_row, &world->player_col);
    }
    print_board(world->board, world->player_row, world->player_col, world->lives);

    setup_feature(world);
    try_unlock(world);
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
