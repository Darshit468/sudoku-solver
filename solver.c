/*
 * Sudoku solver in C.
 *
 * Build:  gcc -std=c11 -O2 -o sudoku solver.c     (or just: make)
 * Run:    ./sudoku puzzle.txt                     (or pipe a puzzle on stdin)
 *         ./sudoku --check                        (self-check, also run by CI)
 *
 * A puzzle is 81 cells: '1'-'9' are clues, and '.', '0' or '_' are empty.
 * Layout characters - spaces, pipes, plus signs, dashes - are ignored, so a
 * grid copied out of a puzzle book usually works as-is.
 */

#include <stdio.h>
#include <string.h>

#define SIZE 9
#define CELLS (SIZE * SIZE)
#define BOX 3
#define EMPTY 0
#define FULL_MASK 0x3FEu /* bits 1..9 set */

static unsigned bit(int digit)
{
    return 1u << (unsigned)digit;
}

static int popcount(unsigned mask)
{
    int count = 0;
    while (mask) {
        count += (int)(mask & 1u);
        mask >>= 1;
    }
    return count;
}

static int count_empty(const unsigned char *grid)
{
    int count = 0;
    for (int i = 0; i < CELLS; i++)
        if (grid[i] == EMPTY)
            count++;
    return count;
}

/* Is the same digit repeated in any row, column or box? */
static int has_conflict(const unsigned char *grid)
{
    for (int row = 0; row < SIZE; row++) {
        unsigned seen = 0;
        for (int col = 0; col < SIZE; col++) {
            unsigned char value = grid[row * SIZE + col];
            if (!value)
                continue;
            if (seen & bit(value))
                return 1;
            seen |= bit(value);
        }
    }

    for (int col = 0; col < SIZE; col++) {
        unsigned seen = 0;
        for (int row = 0; row < SIZE; row++) {
            unsigned char value = grid[row * SIZE + col];
            if (!value)
                continue;
            if (seen & bit(value))
                return 1;
            seen |= bit(value);
        }
    }

    for (int box_row = 0; box_row < SIZE; box_row += BOX) {
        for (int box_col = 0; box_col < SIZE; box_col += BOX) {
            unsigned seen = 0;
            for (int row = box_row; row < box_row + BOX; row++) {
                for (int col = box_col; col < box_col + BOX; col++) {
                    unsigned char value = grid[row * SIZE + col];
                    if (!value)
                        continue;
                    if (seen & bit(value))
                        return 1;
                    seen |= bit(value);
                }
            }
        }
    }

    return 0;
}

/* Which digits may legally go in this cell right now, as a bitmask. */
static unsigned candidates(const unsigned char *grid, int row, int col)
{
    unsigned used = 0;

    for (int c = 0; c < SIZE; c++)
        used |= bit(grid[row * SIZE + c]);
    for (int r = 0; r < SIZE; r++)
        used |= bit(grid[r * SIZE + col]);

    int box_row = row - row % BOX;
    int box_col = col - col % BOX;
    for (int r = box_row; r < box_row + BOX; r++)
        for (int c = box_col; c < box_col + BOX; c++)
            used |= bit(grid[r * SIZE + c]);

    return ~used & FULL_MASK;
}

/* Backtracking: always fill the most constrained cell first. */
static int search(unsigned char *grid)
{
    int best = -1;
    unsigned best_choices = 0;
    int best_count = 10;

    for (int index = 0; index < CELLS; index++) {
        if (grid[index] != EMPTY)
            continue;

        unsigned choices = candidates(grid, index / SIZE, index % SIZE);
        int count = popcount(choices);

        if (count == 0)
            return 0; /* a cell with no legal digit: this branch is dead */

        if (count < best_count) {
            best = index;
            best_choices = choices;
            best_count = count;
            if (count == 1)
                break; /* a forced cell: nothing can be better */
        }
    }

    if (best < 0)
        return 1; /* no empty cells left */

    for (int value = 1; value <= 9; value++) {
        if (!(best_choices & bit(value)))
            continue;
        grid[best] = (unsigned char)value;
        if (search(grid))
            return 1;
        grid[best] = EMPTY;
    }

    return 0;
}

static int solve(unsigned char *grid)
{
    if (has_conflict(grid))
        return 0;
    return search(grid);
}

/* Is this a complete, legal Sudoku? */
static int is_valid(const unsigned char *grid)
{
    for (int row = 0; row < SIZE; row++) {
        unsigned seen = 0;
        for (int col = 0; col < SIZE; col++) {
            unsigned char value = grid[row * SIZE + col];
            if (!value || (seen & bit(value)))
                return 0;
            seen |= bit(value);
        }
        if (seen != FULL_MASK)
            return 0;
    }

    for (int col = 0; col < SIZE; col++) {
        unsigned seen = 0;
        for (int row = 0; row < SIZE; row++) {
            unsigned char value = grid[row * SIZE + col];
            if (!value || (seen & bit(value)))
                return 0;
            seen |= bit(value);
        }
        if (seen != FULL_MASK)
            return 0;
    }

    for (int box_row = 0; box_row < SIZE; box_row += BOX) {
        for (int box_col = 0; box_col < SIZE; box_col += BOX) {
            unsigned seen = 0;
            for (int row = box_row; row < box_row + BOX; row++) {
                for (int col = box_col; col < box_col + BOX; col++) {
                    unsigned char value = grid[row * SIZE + col];
                    if (!value || (seen & bit(value)))
                        return 0;
                    seen |= bit(value);
                }
            }
            if (seen != FULL_MASK)
                return 0;
        }
    }

    return 1;
}

/* Did every clue in the original puzzle survive? */
static int respects(const unsigned char *grid, const unsigned char *puzzle)
{
    for (int i = 0; i < CELLS; i++)
        if (puzzle[i] != EMPTY && puzzle[i] != grid[i])
            return 0;
    return 1;
}

/* Turn text into 81 cells. Returns 1 on success. */
static int parse(const char *text, unsigned char *grid)
{
    int count = 0;

    for (const char *cursor = text; *cursor; cursor++) {
        char ch = *cursor;

        if (ch == '.' || ch == '0' || ch == '_') {
            if (count == CELLS)
                return 0;
            grid[count++] = EMPTY;
        } else if (ch >= '1' && ch <= '9') {
            if (count == CELLS)
                return 0;
            grid[count++] = (unsigned char)(ch - '0');
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ||
                   ch == '|' || ch == '+' || ch == '-' || ch == '=') {
            continue;
        } else {
            return 0;
        }
    }

    return count == CELLS;
}

static void print_grid(const unsigned char *grid)
{
    for (int row = 0; row < SIZE; row++) {
        if (row && row % BOX == 0)
            printf("+-------+-------+-------+\n");
        for (int col = 0; col < SIZE; col++) {
            if (col && col % BOX == 0)
                printf("| ");
            unsigned char value = grid[row * SIZE + col];
            if (value)
                printf("%d ", value);
            else
                printf(". ");
        }
        printf("\n");
    }
}

/* One pass over one puzzle; returns the number of problems found. */
static int check_one(const char *name, const unsigned char *puzzle, int should_solve)
{
    unsigned char grid[CELLS];
    memcpy(grid, puzzle, sizeof grid);

    int solved = solve(grid);

    if (!should_solve) {
        if (solved) {
            printf("%s: expected no solution, but one was found\n", name);
            return 1;
        }
        if (memcmp(grid, puzzle, sizeof grid) != 0) {
            printf("%s: a failed search left the grid modified\n", name);
            return 1;
        }
        printf("%s: correctly reported as unsolvable\n", name);
        return 0;
    }

    if (!solved) {
        printf("%s: no solution found\n", name);
        return 1;
    }
    if (!is_valid(grid)) {
        printf("%s: the answer is not a legal Sudoku\n", name);
        return 1;
    }
    if (!respects(grid, puzzle)) {
        printf("%s: the answer changed one of the clues\n", name);
        return 1;
    }

    printf("%s: solved, filled %d empty cells\n", name, count_empty(puzzle));
    return 0;
}

static int self_check(void)
{
    unsigned char classic[CELLS];
    unsigned char empty[CELLS];
    unsigned char broken[CELLS];
    int failures = 0;

    if (!parse("530070000600195000098000060800060003400803001700020006060000280000419005000080079", classic)) {
        printf("could not read the classic puzzle\n");
        return 1;
    }
    memset(empty, EMPTY, sizeof empty);
    if (!parse("550000000600195000098000060800060003400803001700020006060000280000419005000080079", broken)) {
        printf("could not read the contradictory puzzle\n");
        return 1;
    }

    failures += check_one("classic puzzle", classic, 1);
    failures += check_one("empty grid", empty, 1);
    failures += check_one("contradictory puzzle", broken, 0);

    if (failures) {
        printf("\n%d check(s) failed\n", failures);
        return 1;
    }
    printf("\nAll checks passed\n");
    return 0;
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--check") == 0)
            return self_check();

    static char buffer[8192];
    size_t length;

    const char *path = NULL;
    for (int i = 1; i < argc; i++)
        if (argv[i][0] != '-')
            path = argv[i];

    if (path) {
        FILE *file = fopen(path, "rb");
        if (!file) {
            fprintf(stderr, "error: could not open %s\n", path);
            return 2;
        }
        length = fread(buffer, 1, sizeof buffer - 1, file);
        fclose(file);
    } else {
        length = fread(buffer, 1, sizeof buffer - 1, stdin);
    }
    buffer[length] = '\0';

    unsigned char grid[CELLS];
    if (!parse(buffer, grid)) {
        fprintf(stderr, "error: a puzzle needs 81 cells (digits 1-9, or . for empty)\n");
        return 2;
    }

    if (has_conflict(grid)) {
        fprintf(stderr, "This puzzle breaks the Sudoku rules: a digit is repeated.\n");
        return 1;
    }

    int empties = count_empty(grid);
    if (!solve(grid)) {
        fprintf(stderr, "No solution: the clues contradict each other.\n");
        return 1;
    }

    print_grid(grid);
    printf("\nSolved: filled %d empty cells.\n", empties);
    return 0;
}
