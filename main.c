#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

int bestDepth;
int taskThreshold;
int *bestMoves;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

void copyIntArray(const int *source, int *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}

void copyArray(const char *source, char *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}

void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubbleSort(int *scores, int *successors, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (scores[j] <= scores[j + 1]) {
                swap(&successors[j], &successors[j + 1]);
                swap(&scores[j], &scores[j + 1]);
            }
        }
    }
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

bool isPositionValid(int rowIndex, int colIndex, int chessboardSize) {
    return rowIndex < chessboardSize
           && rowIndex >= 0
           && colIndex < chessboardSize
           && colIndex >= 0;
}

int calculateScore(char *chessboard, int chessboardSize, int successor, char movingFigure) {
    if (chessboard[successor] == 'P')
        return 2;

    if (movingFigure == 'J')
        return 0;

    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        POSITION position = mapIndexInverse(successor, chessboardSize);
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (chessboard[index] == 'P') return 1;
        }
    }

    return 0;
}

void val(char *chessboard, int chessboardSize, int *successors, int successorsSize, char nextMove) {
    int *score = (int *) malloc(sizeof(int) * successorsSize);
    for (int i = 0; i < successorsSize; i++) {
        score[i] = calculateScore(chessboard, chessboardSize, successors[i], nextMove);
    }

    bubbleSort(score, successors, successorsSize);
    free(score);
}

int move(char *chessboard, int chessboardSize, int startMoveIndex, int endMoveIndex) {
    int taken = 0;
    if (chessboard[endMoveIndex] == 'P') taken++;

    char figureStart = chessboard[startMoveIndex];

    chessboard[startMoveIndex] = '-';
    chessboard[endMoveIndex] = figureStart;

    return taken;
}

void printFigureCapturedMove(int moveOrder, char figure, POSITION startPosition, POSITION endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]*\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printFigureMove(int moveOrder, char figure, POSITION startPosition, POSITION endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printMoves(int *moves, int movesCount, int chessboardSize, int knightIndex, int bishopIndex) {
    char figure;
    POSITION knightPos = mapIndexInverse(knightIndex, chessboardSize);
    POSITION bishopPos = mapIndexInverse(bishopIndex, chessboardSize);

    for (int i = 0; i < movesCount; i++) {
        figure = i % 2 == 0 ? 'S' : 'J';
        POSITION nextMovePos;
        if (moves[i] > 0) {
            nextMovePos = mapIndexInverse(moves[i], chessboardSize);
        } else {
            nextMovePos = mapIndexInverse(moves[i] * -1, chessboardSize);
        }

        if (i == 0 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, bishopPos, nextMovePos);
        } else if (i == 0 && moves[i] <= 0) {
            printFigureMove(i, figure, bishopPos, nextMovePos);
        }

        if (i == 1 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, knightPos, nextMovePos);
        } else if (i == 1 && moves[i] <= 0) {
            printFigureMove(i, figure, knightPos, nextMovePos);
        }

        if (i > 1) {
            POSITION startPosition;
            if (moves[i - 2] > 0) {
                //nextMovePos = mapIndexInverse(moves[i - 2], chessboardSize);
                startPosition = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                //nextMovePos = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
                startPosition = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
            }

            if (moves[i] > 0) {
                printFigureCapturedMove(i, figure, startPosition, nextMovePos);
            } else if (moves[i] <= 0) {
                printFigureMove(i, figure, startPosition, nextMovePos);
            }
        }
    }
}

bool canMove(char figurine) {
    return figurine != 'J' && figurine != 'S';
}

// get next moves for a bishop
int nextBishop(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    POSITION position = mapIndexInverse(startIndex, chessboardSize);
    int nextMoveCount = 0;

    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }
    }

    return nextMoveCount;
}

// get next moves for a knight
int nextKnight(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    int moves[4] = {-1, -2, 1, 2};
    int movesSize = sizeof(moves) / sizeof(moves[0]);
    POSITION position = mapIndexInverse(startIndex, chessboardSize);

    int nextMoveCount = 0;
    for (int i = 0; i < movesSize; i++) {
        for (int j = 0; j < movesSize; j++) {
            if (moves[i] + moves[j] == 0 || moves[i] == moves[j])
                continue;

            if (!isPositionValid(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize))
                continue;

            int index = mapIndex(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize);
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }
    }

    return nextMoveCount;
}

// get next moves for a knight or a bishop depends on position
int next(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    //int index = mapIndex(position.rowIndex, position.colIndex, chessboardSize);
    if (chessboard[startIndex] == 'J') {
        return nextKnight(chessboard, chessboardSize, successors, startIndex);
    } else if (chessboard[startIndex] == 'S') {
        return nextBishop(chessboard, chessboardSize, successors, startIndex);
    } else {
        printf("Invalid position '%c' for J/S.\n", chessboard[startIndex]);
        return -1;
    }
}


void dfsChessboard(char *chessboard, int chessboardSize, int *moves, int knightStartIndex,
                   int bishopStartIndex, int boardFigures, int takeFiguresCount, int maxDepth, int depth,
                   char figureMove) {
    // lower bound
    if (depth == boardFigures && boardFigures == takeFiguresCount) {
#pragma omp critical
        {
            if (depth == boardFigures && boardFigures == takeFiguresCount) {
                bestDepth = depth;
                copyIntArray(moves, bestMoves, maxDepth);
            }
        }
        return;
    }

    // upper bound (init best depth is max depth)
    if (depth >= bestDepth) {
        return;
    }

    // update best depth
    if (boardFigures == takeFiguresCount && depth < bestDepth) {
#pragma omp critical
        {
            if (boardFigures == takeFiguresCount && depth < bestDepth) {
                bestDepth = depth;
                copyIntArray(moves, bestMoves, maxDepth);
            }
        }
        return;
    }

    // Branch and bound
    if (depth + (boardFigures - takeFiguresCount) >= bestDepth) {
        return;
    }

    int startMoveIndex;
    char nextMove;
    int *successors;
    if (figureMove == 'S') {
        nextMove = 'J';
        startMoveIndex = bishopStartIndex;
        successors = (int *) malloc(sizeof(int) * (2 * chessboardSize - 2));
    } else {
        nextMove = 'S';
        startMoveIndex = knightStartIndex;
        successors = (int *) malloc(sizeof(int) * 8);
    }

    // get all possible moves
    int successorSize = next(chessboard, chessboardSize, successors, startMoveIndex);
    val(chessboard, chessboardSize, successors, successorSize, figureMove);

    for (int i = 0; i < successorSize; i++) {
        // copy chessboard
        char *chessboardCopy = (char *) malloc(sizeof(char) * chessboardSize * chessboardSize);
        copyArray(chessboard, chessboardCopy, chessboardSize * chessboardSize);

        int startPosition = figureMove == 'S' ? bishopStartIndex : knightStartIndex;

        int capturedFigure = move(chessboardCopy, chessboardSize, startPosition, successors[i]);
        if (capturedFigure == 0) {
            moves[depth] = successors[i] * -1;
        } else if (capturedFigure == 1) {
            moves[depth] = successors[i];
        }

        // update index position after move
        int nextBishopIndex = bishopStartIndex;
        int nextKnightIndex = knightStartIndex;
        if (figureMove == 'S') {
            nextBishopIndex = successors[i];
        } else {
            nextKnightIndex = successors[i];
        }

        if (taskThreshold > depth) {
            int *copyMoves = (int *) malloc(sizeof(int) * maxDepth);
            copyIntArray(moves, copyMoves, bestDepth);
        } else {
            dfsChessboard(chessboardCopy, chessboardSize, moves, nextKnightIndex, nextBishopIndex, boardFigures,
                          takeFiguresCount + capturedFigure, maxDepth, depth + 1, nextMove);
            free(chessboardCopy);
        }
    }
    free(successors);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);

    int chessboardStartArg = 3;
    char *p_chessboardSize;
    p_chessboardSize = *(argv + 1);

    char *p_maxDepth;
    p_maxDepth = *(argv + 2);

    int chessboardSize = atoi(p_chessboardSize);
    int maxDepth = atoi(p_maxDepth);

    int oneDimensionSize = chessboardSize * chessboardSize;

    char *chessboard = (char *) malloc(sizeof(char) * oneDimensionSize);
    int *moves = (int *) malloc(sizeof(int) * maxDepth);

    int knightStartIndex;
    int bishopStartIndex;
    int boardFigures = 0;

// Parse instance from CMD
    for (int i = chessboardStartArg; i < argc; i++) {
        int size = strlen(argv[i]);
        for (int j = 0; j < size; j++) {
            int index = mapIndex(i - chessboardStartArg, j, chessboardSize);
            chessboard[index] = argv[i][j];
            if (chessboard[index] == 'J') {
                knightStartIndex = mapIndex(i - chessboardStartArg, j, chessboardSize);
            }
            if (chessboard[index] == 'S') {
                bishopStartIndex = mapIndex(i - chessboardStartArg, j, chessboardSize);
            }
            if (chessboard[index] == 'P')
                boardFigures++;
        }
    }

    bestDepth = maxDepth;
    bestMoves = (int *) malloc(sizeof(int) * maxDepth);
    taskThreshold = 0.3 * maxDepth;

    dfsChessboard(chessboard, chessboardSize, moves, knightStartIndex, bishopStartIndex, boardFigures,
                  0, maxDepth, 0, 'S');

    printMoves(bestMoves, bestDepth, chessboardSize, knightStartIndex, bishopStartIndex);

    free(chessboard);
    free(moves);
    free(bestMoves);

    return 0;
}