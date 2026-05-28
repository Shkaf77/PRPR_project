# PRPR Project — Sudoku Records Processing

## Author

Oleksandr Nitsenko

## Project Description

This project is a console application written in the C programming language.

The program works with text files containing information about Sudoku games, players and game results. It allows the user to load data from files, display records, create dynamic arrays, work with a linked list, add and delete records, validate input data and generate output files.

The project was created as part of the PRPR course.

## Used Language

- C

## Main Source File

```text
main/main.c
```

## Input Files

The program uses the following input files:

```text
Sudoku.txt
RegisterHracov.txt
RegisterRieseni.txt
```

### Sudoku.txt

Contains information about Sudoku games.

### RegisterHracov.txt

Contains information about players.

### RegisterRieseni.txt

Contains information about solutions and results.

## Output Files

The program can generate output files:

```text
Vystup_E.txt
Vystup_H.txt
```

## Compilation

To compile the project, open terminal in the `main` folder and run:

```bash
gcc main.c -o main
```

## Running the Program

After compilation, run the program with:

```bash
./main
```

On Windows, the program can be compiled and executed for example with:

```bash
gcc main.c -o main.exe
main.exe
```

## Program Commands

The program is controlled by commands entered from standard input.

### Command `v`

Displays data.

```text
v 1
v 2
v 3
```

#### `v 1`

Displays records directly from text files.

#### `v 2`

Displays records from dynamically created arrays.

Before using this command, arrays must be created using command `n`.

#### `v 3`

Displays records from the linked list.

Before using this command, the linked list must be created using command `m`.

---

### Command `n`

Creates dynamic arrays from input files.

```text
n
```

This command loads data from files into dynamically allocated arrays.

---

### Command `q`

Adds a new record into the dynamic array of solutions.

```text
q
```

The program asks the user to enter required data and then adds the new record.

---

### Command `w`

Deletes records from the dynamic array of solutions.

```text
w
```

The program removes selected records according to the entered condition.

---

### Command `e`

Generates Sudoku output into file `Vystup_E.txt`.

```text
e SIDxxxxx X
```

Example:

```text
e SIDA1234 3
```

Where:

- `SIDxxxxx` is the Sudoku identifier,
- `X` is a number from 1 to 5.

The command creates a modified Sudoku board according to the entered value.

---

### Command `m`

Creates a linked list from input files.

```text
m
```

The linked list connects player information with game result information.

---

### Command `a`

Adds a new record into the linked list.

```text
a
```

The program asks the user to enter all required values and inserts the new record into the selected position.

---

### Command `s`

Deletes records from the linked list.

```text
s
```

The program deletes selected records from the linked list and prints the number of deleted records.

---

### Command `d`

Deletes the whole linked list from memory.

```text
d
```

This command frees allocated memory used by the linked list.

---

### Command `h`

Creates a summary output file `Vystup_H.txt`.

```text
h SIDxxxxx
```

Example:

```text
h SIDA1234
```

The command creates a summary for the selected Sudoku identifier.

---

### Command `k`

Closes opened files and frees allocated memory.

```text
k
```

This command is used for cleaning resources before ending work with the program.

## Data Structures Used

The project uses:

- structures,
- dynamic arrays,
- singly linked list,
- pointers,
- file processing,
- dynamic memory allocation.

## Main Structures

### Player Structure

```c
typedef struct {
    char PID[32];
    char Identita[128];
    char Krajina[64];
    int RokNar;
} MPlayer;
```

This structure stores information about a player.

### Result Structure

```c
typedef struct {
    char SID[32];
    char NarHry;
    char GID[32];
    char NarSut;
    char DatHry[9];
    int Trvanie;
} MResult;
```

This structure stores information about a game result.

### Linked List Node

```c
typedef struct MNode {
    MPlayer player;
    MResult result;
    struct MNode *next;
} MNode;
```

This structure represents one node of the linked list.

## Validation

The program validates several identifiers and values:

- `PID`
- `SID`
- `GID`
- date format
- numeric input
- file availability
- memory allocation

Examples of valid formats:

```text
PIDa12345
SIDA1234
GIDa123
```

## Memory Management

The program uses dynamic memory allocation with:

```c
malloc
free
```

Allocated memory is released when arrays or linked lists are deleted.

The program also closes opened files when they are no longer needed.

## Example Usage

```text
n
v 2
m
v 3
h SIDA1234
e SIDA1234 3
k
```

## Conclusion

This project demonstrates working with files, structures, dynamic arrays and linked lists in the C programming language.

The main goal of the program is to process Sudoku-related records, validate data, display information in different forms and generate output files based on user commands.
