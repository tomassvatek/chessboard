#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>

extern int errno;

int recursion_call;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

typedef struct {
    int *successors;
    int successorCount;
} SUCCESSORS;


void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubbleSort(int scores[], int successors[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (scores[j] <= scores[j + 1]) {
                swap(&successors[j], &successors[j + 1]);
                swap(&scores[j], &scores[j + 1]);
            }
}

void printChessboard(char *chessboard, int chessboardSize, int oneDimensionLength) {
    printf("\n");
    for (int i = 0; i < oneDimensionLength; i++) {
        printf("%c", chessboard[i]);
        if ((i + 1) % chessboardSize == 0)
            printf("\n");
    }
    printf("\n");
}

void printSuccessors(SUCCESSORS successors, char figureMove) {
    printf("%d successors for '%c'\n", successors.successorCount, figureMove);
    printf("[");
    for (int i = 0; i < successors.successorCount; i++) {
        printf("%d,", successors.successors[i]);
    }
    printf("]");
    printf("\n");
}


int mapIndex(int i, int j, int chessboardSize) {
    return i * chessboardSize + j;
}

POSITION mapIndexInverse(int index, int chessboardSize) {
    POSITION position;
    position.rowIndex = index / chessboardSize;
    position.colIndex = index % chessboardSize;

    return position;
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
SUCCESSORS nextBishop(char *chessboard, int chessboardSize, POSITION position) {
    int *next = (int *) malloc(sizeof(int) * 2 * chessboardSize - 2);

    int successorsCount = 0;
    for (int i = 0; i < chessboardSize; i++) {
        if (position.rowIndex + i < chessboardSize && position.colIndex + i < chessboardSize) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex + i < chessboardSize && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex + i < chessboardSize) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
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
SUCCESSORS nextKnight(char *chessboard, int chessboardSize, POSITION position) {
    int *next = (int *) malloc(sizeof(int) * 8);
    int moves[4] = {-1, -2, 1, 2};
    int movesSize = sizeof(moves) / sizeof(moves[0]);

    int successorsCount = 0;
    for (int i = 0; i < movesSize; i++) {
        for (int j = 0; j < movesSize; j++) {
            if (moves[i] + moves[j] == 0 || moves[i] == moves[j])
                continue;

            if (!isPositionValid(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize))
                continue;

            int index = mapIndex(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize);
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
SUCCESSORS next(char chessboard[], int chessboardSize, POSITION position) {
    int index = mapIndex(position.rowIndex, position.colIndex, chessboardSize);
    if (chessboard[index] == 'J') {
        return nextKnight(chessboard, chessboardSize, position);
    } else if (chessboard[index] == 'S') {
        return nextBishop(chessboard, chessboardSize, position);
    } else {
        printf("ERROR");
    }
}

//TODO: Refactor indicitating diagonal chess piece
int calculateScore(char chessboard[], int chessboardSize, int successorIndex, char nextPiece) {
    if (chessboard[successorIndex] == 'P')
        return 2;

    if (nextPiece == 'J')
        return 0;

    for (int i = 0; i < chessboardSize; i++) {
        POSITION position = mapIndexInverse(successorIndex, chessboardSize);
        if (position.rowIndex + i < chessboardSize && position.colIndex + i < chessboardSize) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P') return 1;
        }

        if (position.rowIndex - i >= 0 && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P') return 1;
        }

        if (position.rowIndex + i < chessboardSize && position.colIndex - i >= 0) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P') return 1;
        }

        if (position.rowIndex - i >= 0 && position.colIndex + i < chessboardSize) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P') return 1;
        }
    }

    return 0;
}

// Otestovat zda funguje jak má.
SUCCESSORS val(char chessboard[], int chessboardSize, SUCCESSORS successors, char nextMove) {
    int score[successors.successorCount];
    for (int i = 0; i < successors.successorCount; i++) {
        score[i] = calculateScore(chessboard, chessboardSize, successors.successors[i], nextMove);
    }

    bubbleSort(score, successors.successors, successors.successorCount);

    return successors;
}

// Pohyb figurky po šachovnici. Vytvoří novou šachovnici a vrátí počet aktuální počet figurek na šachovnici
int move(char *chessboard, int chessboardSize, POSITION *startPosition, int moveIndex) {
    int taken = 0;
    if (chessboard[moveIndex] == 'P')
        taken++;

    int startMoveIndex = mapIndex(startPosition->rowIndex, startPosition->colIndex, chessboardSize);
    char figureStart = chessboard[startMoveIndex];
    chessboard[startMoveIndex] = '-';
    chessboard[moveIndex] = figureStart;
    *startPosition = mapIndexInverse(moveIndex, chessboardSize);

    return taken;
}

// Začíná střelec
// TODO: Implement Branch&Bound
void chessboardSearch(char chessboard[], POSITION knightPos, POSITION bishopPos, int pieceCount,
                      int discardedPieceCount, int maxDepth, int depth, int bestDepth, char pieceMove) {

    recursion_call++;
    printf("Recursion call number: %d\n", recursion_call);

    if (pieceCount == discardedPieceCount && depth < maxDepth) {
        maxDepth = depth;
    }

    // lower bound
    if (depth == pieceCount && pieceCount == discardedPieceCount)
        return;

    // upper bound
    if (depth > maxDepth)
        return;

//    if (depth + (pieceCount - discardedPieceCount) < bestDepth)
//        return;

    printf("%d pieces discarded in %d turns\n", discardedPieceCount, depth);

    POSITION *movePosition = pieceMove == 'S' ? &bishopPos : &knightPos;
    SUCCESSORS successors = next(chessboard, 5, *movePosition);
    //TADY CHYBA! Nejspíš uniká paměť
    if (successors.successorCount > 150) {
        printf('err');
    }

    char nextMove = pieceMove == 'S' ? 'J' : 'S';
    successors = val(chessboard, 5, successors, pieceMove);
    printSuccessors(successors, pieceMove);

    for (int i = 0; i < successors.successorCount; i++) {
        char *chessboardCopy = (char *) malloc(sizeof(char) * 25);
        strcpy(chessboardCopy, chessboard);

        discardedPieceCount += move(chessboardCopy, 5, movePosition, successors.successors[i]);
        printChessboard(chessboardCopy, 5, 25);

        depth++;
        chessboardSearch(chessboardCopy, knightPos, bishopPos, pieceCount, discardedPieceCount, maxDepth, depth,
                         bestDepth, nextMove);

        free(chessboardCopy);
        //free(successors.successors);
    }
    //free(successors.successors);

    // UPDATE POSITION AFTER MOVE
    //chessboardSearch(chessboard, knightPos, bishopPos, maxDepth, depth + 1, pieceMove);
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
    POSITION knightPosition;
    knightPosition.rowIndex = 0;
    knightPosition.colIndex = 0;

    POSITION bishopPosition;
    bishopPosition.rowIndex = 1;
    bishopPosition.colIndex = 1;

//    printChessboard(chessBoard, chessboard_size, oneDimensionSize);
//
//    SUCCESSORS next = nextBishop(chessBoard, chessboard_size, bishopPosition);
//    printSuccessors(next);
//
//    next = val(chessBoard, next, 'S');

    chessboardSearch(chessBoard, knightPosition, bishopPosition, 3, 0, max_depth, 0, 0, 'S');
    free(chessBoard);

    return 0;
}