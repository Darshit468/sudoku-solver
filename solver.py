#!/usr/bin/env python3
"""A Sudoku solver in plain Python.

Reads a 9x9 puzzle, fills it in with backtracking, and prints the answer.

    python solver.py puzzle.txt     # solve a file
    python solver.py < puzzle.txt   # or pipe one in
    python solver.py --check        # self-check, also run by CI

A puzzle is 81 cells: digits 1-9 are clues, and '.', '0' or '_' are empty.
Layout characters (spaces, pipes, plus signs) are ignored, so a grid copied
straight out of a puzzle book usually works as-is.
"""

from __future__ import annotations

import sys
from typing import List, Optional, Set, Tuple

SIZE = 9
BOX = 3
EMPTY = 0

Grid = List[List[int]]


def parse(text: str) -> Grid:
    """Turn puzzle text into a 9x9 grid of ints."""
    cells: List[int] = []
    for char in text:
        if char in ".0_":
            cells.append(EMPTY)
        elif char.isdigit():
            cells.append(int(char))
        elif char.isspace() or char in "|+-=":
            continue
        else:
            raise ValueError(f"unexpected character {char!r}")
    if len(cells) != SIZE * SIZE:
        raise ValueError(f"a puzzle needs {SIZE * SIZE} cells, got {len(cells)}")
    return [cells[start:start + SIZE] for start in range(0, SIZE * SIZE, SIZE)]


def to_text(grid: Grid) -> str:
    """Flatten a grid back into an 81-character string, using '.' for empties."""
    return "".join(str(value) if value else "." for row in grid for value in row)


def count_empty(grid: Grid) -> int:
    return sum(1 for row in grid for value in row if value == EMPTY)


def has_conflict(grid: Grid) -> bool:
    """True if the same digit appears twice in a row, column or box."""
    for row in range(SIZE):
        values = [v for v in grid[row] if v != EMPTY]
        if len(values) != len(set(values)):
            return True
    for col in range(SIZE):
        values = [grid[r][col] for r in range(SIZE) if grid[r][col] != EMPTY]
        if len(values) != len(set(values)):
            return True
    for box_row in range(0, SIZE, BOX):
        for box_col in range(0, SIZE, BOX):
            values = [
                grid[r][c]
                for r in range(box_row, box_row + BOX)
                for c in range(box_col, box_col + BOX)
                if grid[r][c] != EMPTY
            ]
            if len(values) != len(set(values)):
                return True
    return False


def candidates(grid: Grid, row: int, col: int) -> Set[int]:
    """Digits that could legally go in this cell right now."""
    used = set(grid[row])
    used.update(grid[r][col] for r in range(SIZE))
    box_row, box_col = row - row % BOX, col - col % BOX
    for r in range(box_row, box_row + BOX):
        for c in range(box_col, box_col + BOX):
            used.add(grid[r][c])
    return set(range(1, 10)) - used


def _search(grid: Grid) -> bool:
    """Recursive backtracking. Chooses the most constrained empty cell first."""
    best: Optional[Tuple[int, int]] = None
    best_choices: Set[int] = set()

    for row in range(SIZE):
        for col in range(SIZE):
            if grid[row][col] != EMPTY:
                continue
            choices = candidates(grid, row, col)
            if not choices:
                return False  # this cell can never be filled: dead end
            if best is None or len(choices) < len(best_choices):
                best, best_choices = (row, col), choices

    if best is None:
        return True  # nothing empty left, so the grid is full

    row, col = best
    for value in sorted(best_choices):
        grid[row][col] = value
        if _search(grid):
            return True
        grid[row][col] = EMPTY
    return False


def solve(grid: Grid) -> bool:
    """Fill the grid in place. Returns False if it has no solution."""
    if has_conflict(grid):
        return False
    return _search(grid)


def is_valid(grid: Grid) -> bool:
    """True if the grid is complete and every row, column and box is 1-9."""
    expected = list(range(1, 10))
    for row in range(SIZE):
        if sorted(grid[row]) != expected:
            return False
    for col in range(SIZE):
        if sorted(grid[r][col] for r in range(SIZE)) != expected:
            return False
    for box_row in range(0, SIZE, BOX):
        for box_col in range(0, SIZE, BOX):
            values = [
                grid[r][c]
                for r in range(box_row, box_row + BOX)
                for c in range(box_col, box_col + BOX)
            ]
            if sorted(values) != expected:
                return False
    return True


def respects(grid: Grid, puzzle: Grid) -> bool:
    """True if every clue from the original puzzle is unchanged."""
    return all(
        puzzle[r][c] == EMPTY or puzzle[r][c] == grid[r][c]
        for r in range(SIZE)
        for c in range(SIZE)
    )


def format_grid(grid: Grid) -> str:
    """Draw the grid with box separators."""
    lines: List[str] = []
    for row in range(SIZE):
        if row and row % BOX == 0:
            lines.append("+-------+-------+-------+")
        cells: List[str] = []
        for col in range(SIZE):
            if col and col % BOX == 0:
                cells.append("|")
            cells.append(str(grid[row][col]) if grid[row][col] else ".")
        lines.append(" ".join(cells))
    return "\n".join(lines)


SAMPLES: List[str] = [
    # The classic puzzle from Wikipedia's Sudoku article.
    "530070000600195000098000060800060003400803001700020006060000280000419005000080079",
    # An empty grid: the solver still has to complete it.
    "." * 81,
    # Two 5s in the same row, so there is no solution and solve() must say so.
    "550000000600195000098000060800060003400803001700020006060000280000419005000080079",
]


def self_check() -> int:
    """Solve the samples and verify the answers. Returns a process exit code."""
    failures = 0

    for index, text in enumerate(SAMPLES, start=1):
        puzzle = parse(text)
        original = [row[:] for row in puzzle]
        solved = solve(puzzle)

        if index == len(SAMPLES):
            # The last sample is unsolvable on purpose.
            if solved or puzzle != original:
                print(f"sample {index}: expected no solution")
                failures += 1
            else:
                print(f"sample {index}: correctly reported as unsolvable")
            continue

        if not solved:
            print(f"sample {index}: no solution found")
            failures += 1
        elif not is_valid(puzzle):
            print(f"sample {index}: answer is not a legal Sudoku")
            failures += 1
        elif not respects(puzzle, original):
            print(f"sample {index}: answer changed one of the clues")
            failures += 1
        else:
            print(f"sample {index}: solved, filled {count_empty(original)} empty cells")

    if failures:
        print(f"\n{failures} check(s) failed")
        return 1
    print("\nAll checks passed")
    return 0


def main(argv: List[str]) -> int:
    if "--check" in argv:
        return self_check()

    paths = [arg for arg in argv if not arg.startswith("-")]
    try:
        if paths:
            with open(paths[0], encoding="utf-8") as handle:
                text = handle.read()
        else:
            text = sys.stdin.read()
        puzzle = parse(text)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    if has_conflict(puzzle):
        print("This puzzle breaks the Sudoku rules: a digit is repeated.", file=sys.stderr)
        return 1

    empties = count_empty(puzzle)
    if not solve(puzzle):
        print("No solution: the clues contradict each other.", file=sys.stderr)
        return 1

    print(format_grid(puzzle))
    print(f"\nSolved: filled {empties} empty cells.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
