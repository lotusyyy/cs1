// cs_caverun.c
// Written by <INSERT YOUR FULL NAME> <INSERT YOUR ZID> on <INSERT DATE>
//
// Description: <INSERT DESCRIPTION OF PROGRAM>

// Provided Libraries
#include <stdio.h>

// Add your own #include statements below this line

// Provided constants
#define COLS 10
#define ROWS 10
#define INVALID_ROW -1
#define INVALID_COL -1
#define INITIAL_LIVES 3

// Add your own #defines constants below this line

// Provided Enums
// Enum for features on the game board
enum entity {
    EMPTY, DIRT, WALL, BOULDER, GEM, EXIT_LOCKED, EXIT_UNLOCKED, HIDDEN, LAVA
};

// Add your own enums below this line

// Represents a tile/cell on the game board
struct tile {
    enum entity entity;
};

// Add your own structs below this line

// Provided Function Prototypes
void initialise_board(struct tile board[ROWS][COLS]);
void print_board(struct tile board[ROWS][COLS], int player_row, int player_col, int lives_remaining);
void print_board_line(void);
void print_board_header(int lives);
void print_map_statistics(int number_of_dirt_tiles, int number_of_gem_tiles, int number_of_boulder_tiles,
        double completion_percentage, int maximum_points_remaining);

// Add your function prototypes below this line

// Provided sample main() function (you will need to modify this)
int main(void) {
    printf("Welcome to CS Caverun!\n\n");

    struct tile board[ROWS][COLS];
    initialise_board(board);

    print_board(board, INVALID_ROW, INVALID_COL, INITIAL_LIVES);

    return 0;
}

// Add your function definitions below this line

// =============================================================================
// Definitions of Provided Functions
// =============================================================================

// Given a 2D board array, initialise all tile entities to DIRT.
void initialise_board(struct tile board[ROWS][COLS]) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            board[row][col].entity = DIRT;
        }
    }
}

// Prints the game board, showing the player's position and lives remaining
void print_board(struct tile board[ROWS][COLS], int player_row, int player_col, int lives_remaining) {
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
