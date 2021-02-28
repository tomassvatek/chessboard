#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>

extern int errno;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

typedef struct {
    int *successors;
    int successorCount;
} SUCCESSORS;

int mapIndex(int i, int j, int chessboardSize) {
    return i * chessboardSize + j;
}

bool canMove(char figurine) {
    return figurine != 'J' && figurine != 'S';
}

bool isPositionValid(int rowIndex, int colIndex, int length) {
    return rowIndex < length
           && rowIndex >= 0
           && colIndex < length
           && colIndex >= 0;
}

// Množina následovníků střelce
// TODO: střelec nemuze preskakovat
SUCCESSORS nextKnight(char *chessboard, int length, POSITION position) {
    int *next = (int *) malloc(sizeof(int) * 2 * length - 2);

    int successorsCount = 0;
    for (int i = 0; i < length; i++) {
        if (position.rowIndex + i < length && position.colIndex + i < length) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, length);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, length);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex + i < length && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, length);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex + i < length) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, length);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }
    }

    SUCCESSORS successors;
    successors.successors = next;
    successors.successorCount = successorsCount;

    return successors;
}

// Množina následovníků jezdce
SUCCESSORS nextBishop(char *chessboard, int length, POSITION position) {
    int *next = (int *) malloc(sizeof(int) * 8);
    int moves[4] = {-1, -2, 1, 2};
    int movesSize = sizeof(moves) / sizeof(moves[0]);

    int successorsCount = 0;
    for (int i = 0; i < movesSize; i++) {
        for (int j = 0; j < movesSize; j++) {
            if (moves[i] + moves[j] == 0 || moves[i] == moves[j])
                continue;

            if (!isPositionValid(position.rowIndex + moves[i], position.colIndex + moves[j], length))
                continue;

            int index = mapIndex(position.rowIndex + moves[i], position.colIndex + moves[j], length);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }
    }

    SUCCESSORS successors;
    successors.successors = next;
    successors.successorCount = successorsCount;

    return successors;
}

// Vrátí množinu následovníků na které se může střelec/kůň posuntou jedním tahem
//TODO: Ošetřit nevalidní nextMove
SUCCESSORS next(char chessboard[], int length, POSITION position) {
    int index = mapIndex(position.rowIndex, position.colIndex, strlen(chessboard));
    if (chessboard[index] == 'J') {
        return nextKnight(chessboard, length, position);
    } else {
        return nextBishop(chessboard, length, position);
    }
}

// Pokud tah vede na stejnou diagonalu
int calculateScore(char chessboard[], int successorIndex, char nextMove) {
    if (chessboard[successorIndex] == 'P')
        return 2;

    if (nextMove == 'J')
        return 0;


}

SUCCESSORS val(char chessboard[], SUCCESSORS successors, char nextMove) {
    int score[successors.successorCount];
    for (int i = 0; i < successors.successorCount; i++) {
        score[i] = calculateScore(chessboard, successors.successors[i], nextMove);
    }

    successors.successors = sortedSuccessors;
    return successors;
}

// Začíná střelec
void
chessboardSearch(char chessboard[], POSITION knightPos, POSITION bishopPos, int maxDepth, int depth, char nextMove) {
    if(depth >= maxDepth)


    int size = sizeof(chessboard) / 2;
    SUCCESSORS successors = next(chessboard, size, nextMove == 'S' ? knightPos : bishopPos);
    successors = val(chessboard, successors, nextMove == 'S' ? 'J' : 'S');

    chessboard = move(chessboard, successors.successors[0]);
    // UPDATE POSITION AFTER MOVE
    chessboardSearch(chessboard, knightPos, bishopPos, maxDepth, depth + 1, nextMove);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);

    int chessboardStartArg = 3;

    char *p_chessboardSize;
    p_chessboardSize = *(argv + 1);

    char *p_maxDepth;
    p_maxDepth = *(argv + 2);

    int chessboard_size = atoi(p_chessboardSize);
    int max_depth = atoi(p_maxDepth);

    printf("Chessboard size is: %d\n", chessboard_size);
    printf("MaxDepth size is: %d\n", max_depth);

    char *chessBoard;
    int oneDimensionSize = chessboard_size * chessboard_size;
    chessBoard = (char *) malloc(sizeof(char) * oneDimensionSize);
    printf("Chessboard size is: %d\n", strlen(chessBoard));

    for (int i = chessboardStartArg; i < argc; i++) {
        int size = strlen(argv[i]);
        for (int j = 0; j < size; j++) {
            int index = mapIndex(i - chessboardStartArg, j, chessboard_size);
            chessBoard[index] = argv[i][j];
        }
    }

    // Test knight successors
    POSITION position;
    position.rowIndex = 0;
    position.colIndex = 0;

    SUCCESSORS next = nextBishop(chessBoard, chessboard_size, position);
    printf("%d successors:\n", next.successorCount);
    for (int i = 0; i < next.successorCount; i++) {
        printf("%d\n", next.successors[i]);
    }

    return 0;
}