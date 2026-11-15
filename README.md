# Sudoku Solver in C

A command-line Sudoku solver written in C, with no dependencies beyond the standard library.

This is a clean rewrite of the Sudoku solver I built for my first-semester computer project. The original was written to get the answer; this version is written so that someone else can read it, and so that a machine can test it on every push.

## Build and run

```bash
gcc -std=c11 -O2 -o sudoku solver.c    # or: make

./sudoku puzzle.txt                    # solve a file
echo "530070000600195000098000060800060003400803001700020006060000280000419005000080079" | ./sudoku
./sudoku --check                       # self-check, the same one CI runs
```

A puzzle is 81 cells: `1`–`9` are clues, and `.`, `0` or `_` are empty. Spaces, `|` and `+` are ignored, so you can paste a grid out of a puzzle book as-is.

```
5 3 4 | 6 7 8 | 9 1 2
6 7 2 | 1 9 5 | 3 4 8
1 9 8 | 3 4 2 | 5 6 7
+-------+-------+-------+
8 5 9 | 7 6 1 | 4 2 3
4 2 6 | 8 5 3 | 7 9 1
7 1 3 | 9 2 4 | 8 5 6
+-------+-------+-------+
9 6 1 | 5 3 7 | 2 8 4
2 8 7 | 4 1 9 | 6 3 5
3 4 5 | 2 8 6 | 1 7 9
```

Exit codes: `0` solved, `1` no solution, `2` the input wasn't a puzzle.

## How it works

1. **Parse** — the text becomes 81 bytes, one per cell, `0` meaning empty.
2. **Check the clues** — a digit repeated in a row, column or box is caught before any searching happens.
3. **Backtrack** — pick the empty cell with the fewest legal digits left, try each digit, and undo the choice when it leads nowhere.

Each row, column and box is tracked as a 9-bit mask, so "which digits are still legal here?" is three ORs and a NOT. Choosing the most constrained cell first is the real trick: a solver that simply fills the next empty cell walks a much larger search tree, while picking the forced cell first usually means almost no backtracking at all.

`--check` solves three samples and verifies that each answer is a legal Sudoku, that no given clue moved, and that a contradictory puzzle is correctly rejected. GitHub Actions compiles with `-Wall -Wextra -Werror` and runs it on every push, so a warning is a failed build.

## Files

| File | What it is |
| --- | --- |
| `solver.c` | The solver, the CLI and the self-check |
| `Makefile` | `make`, `make check`, `make clean` |
| `.github/workflows/ci.yml` | Builds and runs the self-check on every push |

## What I'd add next

- Check whether a puzzle has exactly one solution, not just one solution
- Generate puzzles of a chosen difficulty instead of solving supplied ones
- Count the backtracks and compare the heuristic against naive backtracking

## Why C

C is what my course uses, and this project is small enough that the whole thing fits in one file you can read top to bottom. My hardware projects (an earthquake detector and a night lamp built on an Arduino with a gyroscope) belong in their own repository with wiring notes and photos.
