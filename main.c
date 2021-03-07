#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int debug = false;

int recursionCall = 0;

int bestDepth;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubbleSort(int *scores, int *successors, int n) {
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

void printSuccessors(int *successors, int size, char figureMove) {
    printf("%d indexes for '%c'\n", size, figureMove);
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d,", successors[i]);
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

bool isPositionValid(int rowIndex, int colIndex, int chessboardSize) {
    return rowIndex < chessboardSize
           && rowIndex >= 0
           && colIndex < chessboardSize
           && colIndex >= 0;
}

// Množina následovníků střelce
int nextBishop(char *chessboard, int chessboardSize, int *successors, POSITION position) {
    int successorsCount = 0;

    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (canMove(chessboard[index])) {
                successors[successorsCount] = index;
                successorsCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (canMove(chessboard[index])) {
                successors[successorsCount] = index;
                successorsCount++;
            }
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (canMove(chessboard[index])) {
                successors[successorsCount] = index;
                successorsCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (canMove(chessboard[index])) {
                successors[successorsCount] = index;
                successorsCount++;
            }
        }
    }

    return successorsCount;
}

// Množina následovníků jezdce
int nextKnight(char *chessboard, int chessboardSize, int *successors, POSITION position) {
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
                successors[successorsCount] = index;
                successorsCount++;
            }
        }
    }

    return successorsCount;
}

// Vrátí množinu následovníků na které se může střelec/kůň posunout
//TODO: Ošetřit nevalidní nextMove
int next(char chessboard[], int chessboardSize, int *successors, POSITION position) {
    int index = mapIndex(position.rowIndex, position.colIndex, chessboardSize);
    if (chessboard[index] == 'J') {
        return nextKnight(chessboard, chessboardSize, successors, position);
    } else if (chessboard[index] == 'S') {
        return nextBishop(chessboard, chessboardSize, successors, position);
    } else {
        printf("Invalid position for J/S.\n");
    }
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

// Pohyb figurky po šachovnici. Updatuje šachovnici a vrátí počet aktuální počet figurek na šachovnici
int move(char *chessboard, int chessboardSize, POSITION startPosition, int moveIndex) {
    int taken = 0;
    if (chessboard[moveIndex] == 'P') taken++;

    int startMoveIndex = mapIndex(startPosition.rowIndex, startPosition.colIndex, chessboardSize);
    char figureStart = chessboard[startMoveIndex];

    chessboard[startMoveIndex] = '-';
    chessboard[moveIndex] = figureStart;

    return taken;
}

void printMoves(int *moves, int movesCount, int chessboardSize, POSITION knightPos, POSITION bishopPos) {
    char startFigure;
    for (int i = 0; i < movesCount; i++) {
        startFigure = i % 2 == 0 ? 'S' : 'J';
        if (i == 0 && moves[i] > 0) {
            POSITION position = mapIndexInverse(moves[i], chessboardSize);
            printf("%c: [%d, %d] -> [%d, %d]*\n", startFigure, bishopPos.colIndex, bishopPos.rowIndex,
                   position.rowIndex,
                   position.colIndex);
        } else if (i == 0 && moves[i] <= 0) {
            POSITION position = mapIndexInverse(moves[i] * -1, chessboardSize);
            printf("%c: [%d, %d] -> [%d, %d]\n", startFigure, bishopPos.colIndex, bishopPos.rowIndex, position.rowIndex,
                   position.colIndex);
        }

        if (i == 1 && moves[i] > 0) {
            POSITION position = mapIndexInverse(moves[i], chessboardSize);
            printf("%c: [%d, %d] -> [%d, %d]*\n", startFigure, knightPos.colIndex, knightPos.rowIndex,
                   position.rowIndex,
                   position.colIndex);
        } else if (i == 1 && moves[i] <= 0) {
            POSITION position = mapIndexInverse(moves[i] * -1, chessboardSize);
            printf("%c: [%d, %d] -> [%d, %d]\n", startFigure, knightPos.colIndex, knightPos.rowIndex, position.rowIndex,
                   position.colIndex);
        }

        if (i > 1 && moves[i] > 0) {
            POSITION position = mapIndexInverse(moves[i], chessboardSize);
            POSITION positionBefore;
            if (moves[i - 2] >= 0) {
                positionBefore = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                positionBefore = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
            }
            printf("%c: [%d, %d] -> [%d, %d]*\n", startFigure, positionBefore.colIndex, positionBefore.rowIndex,
                   position.rowIndex,
                   position.colIndex);
        } else if (i > 1 && moves[i] <= 0) {
            POSITION position = mapIndexInverse(moves[i] * -1, chessboardSize);
            POSITION positionBefore;
            if (moves[i - 2] >= 0) {
                positionBefore = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                positionBefore = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
            }
            printf("%c: [%d, %d] -> [%d, %d]\n", startFigure, positionBefore.colIndex, positionBefore.rowIndex,
                   position.rowIndex,
                   position.colIndex);
        }
    }
}

void copyIntArray(int *source, int *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}


void copyArray(char *source, char *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}

void
dfsChessboard(char *chessboard, int chessboardSize, int *moves, int *bestMoves, POSITION knightPos, POSITION bishopPos,
              int boardFigures, int takeFiguresCount, int depth, char figureMove) {
    recursionCall++;
    //printf("Recursion call %d\n", recursionCall);

    // lower bound
    if (bestDepth == boardFigures) {
        return;
    }

    // upper bound
    if (depth > bestDepth) {
        return;
    }

    if (boardFigures == takeFiguresCount && depth < bestDepth) {
        bestDepth = depth - 1;
        copyIntArray(moves, bestMoves, bestDepth);
        return;
    }

    if (boardFigures == takeFiguresCount) {
        return;
    }

    if (depth + (boardFigures - takeFiguresCount) >= bestDepth) {
        return;
    }

    POSITION nextFigureMove;
    char nextMove;
    int *successors;
    if (figureMove == 'S') {
        nextMove = 'J';
        nextFigureMove = bishopPos;
        successors = (int *) malloc(sizeof(int) * (2 * chessboardSize - 2));
    } else {
        nextMove = 'S';
        nextFigureMove = knightPos;
        successors = (int *) malloc(sizeof(int) * 8);
    }

    int successorSize = next(chessboard, chessboardSize, successors, nextFigureMove);
    //val(chessboard, chessboardSize, successors, successorSize, figureMove);

    if (debug) printSuccessors(successors, successorSize, figureMove);

    for (int i = 0; i < successorSize; i++) {
        char *chessboardCopy = (char *) malloc(sizeof(char) * chessboardSize * chessboardSize);
        copyArray(chessboard, chessboardCopy, chessboardSize * chessboardSize);

        POSITION startPosition = figureMove == 'S' ? bishopPos : knightPos;

//        if (debug) {
//            printf("Before %d move '%c' to successor %d. Discarded piece: %d\n", depth, figureMove,
//                   successors[i], takeFiguresCount);
//            printf("Position of '%c' [%d, %d]\n", figureMove, startPosition.rowIndex, startPosition.colIndex);
//            printChessboard(chessboardCopy, chessboardSize, chessboardSize * chessboardSize);
//        }

        int capturedFigure = move(chessboardCopy, chessboardSize, startPosition, successors[i]);
        if (capturedFigure == 0) {
            moves[depth - 1] = successors[i] * -1;
        } else if (capturedFigure == 1) {
            moves[depth - 1] = successors[i];
        } else {
            printf('More than 1 figure has been thrown.');
        }

        POSITION nextBishopPos = bishopPos;
        POSITION nextKnightPos = knightPos;
        if (figureMove == 'S') {
            nextBishopPos = mapIndexInverse(successors[i], chessboardSize);
        } else {
            nextKnightPos = mapIndexInverse(successors[i], chessboardSize);
        }

//        if (debug) {
//            printf("%d move '%c' to successor %d. Discarded piece: %d\n", depth, figureMove, successors[i],
//                   takeFiguresCount);
//            printf("Position of '%c' [%d, %d]\n", figureMove, startPosition.rowIndex, startPosition.colIndex);
//            printChessboard(chessboardCopy, chessboardSize, chessboardSize * chessboardSize);
//        }

        dfsChessboard(chessboardCopy, chessboardSize, moves, bestMoves, nextKnightPos, nextBishopPos, boardFigures,
                      takeFiguresCount + capturedFigure, depth + 1, nextMove);

        free(chessboardCopy);
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

    printf("Chessboard size is: %d\n", chessboardSize);
    printf("MaxDepth size is: %d\n", maxDepth);

    int oneDimensionSize = chessboardSize * chessboardSize;
    char *chessBoard = (char *) malloc(sizeof(char) * oneDimensionSize);
    int *moves = (int *) malloc(sizeof(int) * maxDepth);
    int *bestMoves = (int *) malloc(sizeof(int) * maxDepth);

    POSITION knightPosition;
    POSITION bishopPosition;
    int boardFigures = 0;

    // Parse instance
    for (int i = chessboardStartArg; i < argc; i++) {
        int size = strlen(argv[i]);
        for (int j = 0; j < size; j++) {
            int index = mapIndex(i - chessboardStartArg, j, chessboardSize);
            chessBoard[index] = argv[i][j];
            if (chessBoard[index] == 'J') {
                knightPosition.rowIndex = i - chessboardStartArg;
                knightPosition.colIndex = j;
            }
            if (chessBoard[index] == 'S') {
                bishopPosition.rowIndex = i - chessboardStartArg;
                bishopPosition.colIndex = j;
            }
            if (chessBoard[index] == 'P')
                boardFigures++;
        }
    }

    printf("Board figures are: %d\n", boardFigures);
    printChessboard(chessBoard, chessboardSize, chessboardSize * chessboardSize);
    bestDepth = maxDepth;

    clock_t start = clock();
    dfsChessboard(chessBoard, chessboardSize, moves, bestMoves, knightPosition, bishopPosition, boardFigures, 0, 1,
                  'S');
    printMoves(bestMoves, bestDepth, chessboardSize, knightPosition, bishopPosition);
    //printSuccessors(bestMoves, bestDepth, 'S');

    free(chessBoard);
    free(moves);
    free(bestMoves);
    clock_t end = clock();

    float seconds = (float) (end - start) / CLOCKS_PER_SEC;

    printf("Best score is %d. The program finished after %.6f seconds and recursion calls %d.\n", bestDepth, seconds,
           recursionCall);

    return 0;
}