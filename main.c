#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

unsigned long recursionCall = 0;
int bestDepth;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

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

int move(char *chessboard, int chessboardSize, POSITION startPosition, int moveIndex) {
    int taken = 0;
    if (chessboard[moveIndex] == 'P') taken++;

    int startMoveIndex = mapIndex(startPosition.rowIndex, startPosition.colIndex, chessboardSize);
    char figureStart = chessboard[startMoveIndex];

    chessboard[startMoveIndex] = '-';
    chessboard[moveIndex] = figureStart;

    return taken;
}

void printFigureCapturedMove(int moveOrder, char figure, POSITION startPosition, POSITION endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]*\n", moveOrder + 1, figure, startPosition.colIndex, startPosition.rowIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printFigureMove(int moveOrder, char figure, POSITION startPosition, POSITION endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]\n", moveOrder + 1, figure, startPosition.colIndex, startPosition.rowIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printMoves(int *moves, int movesCount, int chessboardSize, POSITION knightPos, POSITION bishopPos) {
    char figure;
    for (int i = 0; i < movesCount; i++) {
        figure = i % 2 == 0 ? 'S' : 'J';
        POSITION startPosition;
        if (moves[i] > 0) {
            startPosition = mapIndexInverse(moves[i], chessboardSize);
        } else {
            startPosition = mapIndexInverse(moves[i] * -1, chessboardSize);
        }

        if (i == 0 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, bishopPos, startPosition);
        } else if (i == 0 && moves[i] <= 0) {
            printFigureMove(i, figure, bishopPos, startPosition);
        }

        if (i == 1 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, knightPos, startPosition);
        } else if (i == 1 && moves[i] <= 0) {
            printFigureMove(i, figure, knightPos, startPosition);
        }

        if (i > 1) {
            POSITION endPosition;
            if (moves[i - 2] > 0) {
                endPosition = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                endPosition = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
            }

            if (moves[i] > 0) {
                printFigureCapturedMove(i, figure, startPosition, endPosition);
            } else if (moves[i] <= 0) {
                printFigureMove(i, figure, startPosition, endPosition);
            }
        }
    }
}

bool canMove(char figurine) {
    return figurine != 'J' && figurine != 'S';
}

// get next moves for a bishop
int nextBishop(char *chessboard, int chessboardSize, int *successors, POSITION position) {
    int successorCount = 0;
    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (canMove(chessboard[index])) {
                successors[successorCount] = index;
                successorCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (canMove(chessboard[index])) {
                successors[successorCount] = index;
                successorCount++;
            }
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (canMove(chessboard[index])) {
                successors[successorCount] = index;
                successorCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (canMove(chessboard[index])) {
                successors[successorCount] = index;
                successorCount++;
            }
        }
    }
    return successorCount;
}

// get next moves for a knight
int nextKnight(char *chessboard, int chessboardSize, int *successors, POSITION position) {
    int moves[4] = {-1, -2, 1, 2};
    int movesSize = sizeof(moves) / sizeof(moves[0]);

    int successorCount = 0;
    for (int i = 0; i < movesSize; i++) {
        for (int j = 0; j < movesSize; j++) {
            if (moves[i] + moves[j] == 0 || moves[i] == moves[j])
                continue;

            if (!isPositionValid(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize))
                continue;

            int index = mapIndex(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize);
            if (canMove(chessboard[index])) {
                successors[successorCount] = index;
                successorCount++;
            }
        }
    }
    return successorCount;
}

// get next moves for a knight or a bishop depends on the moving figure position
int next(char *chessboard, int chessboardSize, int *successors, POSITION position) {
    int index = mapIndex(position.rowIndex, position.colIndex, chessboardSize);
    if (chessboard[index] == 'J') {
        return nextKnight(chessboard, chessboardSize, successors, position);
    } else if (chessboard[index] == 'S') {
        return nextBishop(chessboard, chessboardSize, successors, position);
    } else {
        printf("Invalid position for J/S.\n");
        return -1;
    }
}


void dfsChessboard(char *chessboard, int chessboardSize, int *movesHistory, int *bestMoves, POSITION knightPos,
                   POSITION bishopPos, int boardFigures, int takeFiguresCount, int depth, char figureMove) {
    recursionCall++;

    // lower bound
    if (depth == boardFigures && boardFigures == takeFiguresCount) {
        bestDepth = depth;
        copyIntArray(movesHistory, bestMoves, bestDepth);
        return;
    }

    // upper bound (init best depth is max depth)
    if (depth >= bestDepth) {
        return;
    }

    // update best depth
    if (boardFigures == takeFiguresCount && depth < bestDepth) {
        bestDepth = depth;
        copyIntArray(movesHistory, bestMoves, bestDepth);
        return;
    }

    // Branch and bound
    if (depth + (boardFigures - takeFiguresCount) >= bestDepth) {
        return;
    }

    POSITION moveStartPosition;
    char nextFigureMove;
    int *successors;
    if (figureMove == 'S') {
        nextFigureMove = 'J';
        moveStartPosition = bishopPos;
        successors = (int *) malloc(sizeof(int) * (2 * chessboardSize - 2));
    } else {
        nextFigureMove = 'S';
        moveStartPosition = knightPos;
        successors = (int *) malloc(sizeof(int) * 8);
    }

    // get all possible movesHistory
    int successorSize = next(chessboard, chessboardSize, successors, moveStartPosition);
    val(chessboard, chessboardSize, successors, successorSize, figureMove);

    for (int i = 0; i < successorSize; i++) {
        // copy chessboard
        char *chessboardCopy = (char *) malloc(sizeof(char) * chessboardSize * chessboardSize);
        copyArray(chessboard, chessboardCopy, chessboardSize * chessboardSize);

        POSITION startPosition = figureMove == 'S' ? bishopPos : knightPos;

        int capturedFigures = move(chessboardCopy, chessboardSize, startPosition, successors[i]);
        if (capturedFigures == 0) {
            movesHistory[depth] = successors[i] * -1;
        } else if (capturedFigures == 1) {
            movesHistory[depth] = successors[i];
        } else {
            printf("More than 1 figure has been thrown.");
            return;
        }

        // update position after move
        POSITION nextBishopPos = figureMove == 'S' ? mapIndexInverse(successors[i], chessboardSize) : bishopPos;
        POSITION nextKnightPos = figureMove == 'J' ? mapIndexInverse(successors[i], chessboardSize) : knightPos;;

        dfsChessboard(chessboardCopy, chessboardSize, movesHistory, bestMoves, nextKnightPos, nextBishopPos,
                      boardFigures,
                      takeFiguresCount + capturedFigures, depth + 1, nextFigureMove);
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

    int oneDimensionSize = chessboardSize * chessboardSize;
    char *chessBoard = (char *) malloc(sizeof(char) * oneDimensionSize);
    int *moves = (int *) malloc(sizeof(int) * maxDepth);
    int *bestMoves = (int *) malloc(sizeof(int) * maxDepth);

    POSITION knightPosition;
    POSITION bishopPosition;
    int boardFigures = 0;

    // parse instance from CMD
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

    bestDepth = maxDepth;

    clock_t start = clock();
    dfsChessboard(chessBoard, chessboardSize, moves, bestMoves, knightPosition, bishopPosition, boardFigures,
                  0, 0, 'S');
    printMoves(bestMoves, bestDepth, chessboardSize, knightPosition, bishopPosition);
    clock_t end = clock();

    free(chessBoard);
    free(moves);
    free(bestMoves);

    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Best score is %d. The program finished after %.6f seconds.\n", bestDepth, seconds);

    return 0;
}