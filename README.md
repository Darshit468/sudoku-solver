# Sudoku Solver

A command-line Sudoku solver written in plain Python — no dependencies, no install step.

This is a clean rewrite of the Sudoku solver I built for my first-semester computer project. The original was written to get the answer; this version is written so someone else can read it.

## Run it

```bash
python solver.py puzzle.txt      # solve a file
python solver.py < puzzle.txt    # or pipe one in
echo "530070000600195000098000060800060003400803001700020006060000280000419005000080079" | python solver.py
python solver.py --check         # self-check, the same one CI runs
```

A puzzle is 81 cells: `1`–`9` are clues, and `.`, `0` or `_` are empty. Spaces, `|` and `+` are ignored, so you can paste a grid out of a puzzle book as-is.

```
+-------+-------+-------+
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

## How it works

1. **Parse** — the text becomes a 9×9 grid of integers, `0` meaning empty.
2. **Check the clues** — if a digit already repeats in a row, column or box, the puzzle is unsolvable before any search begins.
3. **Backtrack** — repeatedly pick the empty cell with the fewest legal digits left (a *minimum-remaining-values* heuristic), try each digit, and undo the choice when it leads nowhere.

Choosing the most constrained cell first is the whole trick. A naive solver that always fills the next empty cell has to explore far more of the search tree; picking the forced cell first usually means almost no backtracking at all.

`--check` solves three samples and verifies each answer is a legal Sudoku, that no given clue moved, and that a contradictory puzzle is correctly rejected. It's wired into GitHub Actions, so every push runs it on Python 3.12.

## Files

| File | What it is |
| --- | --- |
| `solver.py` | The solver, the CLI and the self-check |
| `.github/workflows/ci.yml` | Runs `python solver.py --check` on every push |

## What I'd add next

- Check whether a puzzle has exactly one solution, not just one solution
- Generate puzzles of a chosen difficulty instead of solving supplied ones
- Compare the MRV heuristic against naive backtracking with a counter

## Why Python

My other hardware projects (an earthquake detector and a night lamp built on an Arduino with a gyroscope) belong in their own repository with wiring notes and photos. This one is deliberately software-only: it runs anywhere Python runs, which means it can be tested automatically on every commit.
