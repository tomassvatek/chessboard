#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

extern int errno;

int debug = true;

int recursion_call;

typedef struct {
    int rowIndex;
    int colIndex;
} POSITION;

typedef struct {
    int *successors;
    int successorCount;
} SUCCESSORS;

typedef struct {
    char *result;
    int score;
} RESULT;

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
    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        if (position.rowIndex + i < chessboardSize && position.colIndex + i < chessboardSize && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex - i >= 0 && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex + i < chessboardSize && position.colIndex - i >= 0 && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (canMove(chessboard[index])) {
                next[successorsCount] = index;
                successorsCount++;
            }
        }

        if (position.rowIndex - i >= 0 && position.colIndex + i < chessboardSize && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
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

// Začíná střelec, CHYBA VE STŘÍDÁNÍ JEZDCE A STŘELCE
// TODO: Implement Branch&Bound
int chessboardSearch(char chessboard[], int chessboardSize, POSITION knightPos, POSITION bishopPos, int pieceCount,
                     int discardedPieceCount, int maxDepth, int depth, int bestDepth, char pieceMove) {

    recursion_call++;
    if (debug) printf("Recursion call number: %d\n", recursion_call);

    // lower bound
    if (depth == pieceCount && pieceCount == discardedPieceCount)
        return bestDepth;

    // upper bound
    if (depth > maxDepth)
        return depth;

    if (pieceCount == discardedPieceCount && depth < maxDepth)
        maxDepth = depth;

    if (depth < bestDepth && pieceCount == discardedPieceCount)
        bestDepth = depth;

    if (depth + (pieceCount - discardedPieceCount) >= bestDepth)
        return bestDepth;

//    if (debug) printf("%d pieces discarded in %d turns\n", discardedPieceCount, depth);

    POSITION *movePosition = pieceMove == 'S' ? &bishopPos : &knightPos;
    SUCCESSORS successors = next(chessboard, chessboardSize, *movePosition);

    char nextMove = pieceMove == 'S' ? 'J' : 'S';
    successors = val(chessboard, chessboardSize, successors, pieceMove);

    if (debug) printSuccessors(successors, pieceMove);

    char *chessboardCopy = (char *) malloc(sizeof(char) * chessboardSize * chessboardSize);
    strcpy(chessboardCopy, chessboard);

    for (int i = 0; i < successors.successorCount; i++) {
        //char *chessboardCopy = (char *) malloc(sizeof(char) * 25);
        //strcpy(chessboardCopy, chessboard);

        if(debug) {
            printf("Before %d move '%c' to successor %d. Discarded piece: %d\n", depth + 1, pieceMove, successors.successors[i], discardedPieceCount);
            printf("Position of '%c' [%d, %d]\n", pieceMove, movePosition->rowIndex, movePosition->colIndex);
            printChessboard(chessboardCopy, chessboardSize, chessboardSize * chessboardSize);
        }

        discardedPieceCount += move(chessboardCopy, chessboardSize, movePosition, successors.successors[i]);
        depth++;

        if (debug) {
            printf("%d move '%c' to successor %d. Discarded piece: %d\n", depth, pieceMove, successors.successors[i], discardedPieceCount);
            printf("Position of '%c' [%d, %d]\n", pieceMove, movePosition->rowIndex, movePosition->colIndex);
            printChessboard(chessboardCopy, chessboardSize, chessboardSize * chessboardSize);
        }

        int depthRec = chessboardSearch(chessboardCopy, chessboardSize, knightPos, bishopPos, pieceCount,
                                        discardedPieceCount, maxDepth,
                                        depth, bestDepth, nextMove);
        if (bestDepth > depthRec)
            bestDepth = depthRec;

        //free(chessboardCopy);
        //free(successors.successors);
    }
    free(chessboardCopy);

    if (debug)
        printf("Best score is %d\n", bestDepth);

    return bestDepth;

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

    POSITION knightPosition;
    POSITION bishopPosition;
    int chessPiece = 0;

    for (int i = chessboardStartArg; i < argc; i++) {
        int size = strlen(argv[i]);
        for (int j = 0; j < size; j++) {
            int index = mapIndex(i - chessboardStartArg, j, chessboard_size);
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
                chessPiece++;
        }
    }

    // Test knight successors
//    POSITION knightPosition;
//    knightPosition.rowIndex = 0;
//    knightPosition.colIndex = 0;

    //POSITION bishopPosition;
//    bishopPosition.rowIndex = 1;
//    bishopPosition.colIndex = 1;

//    printChessboard(chessBoard, chessboard_size, oneDimensionSize);
//
//    SUCCESSORS next = nextBishop(chessBoard, chessboard_size, bishopPosition);
//    printSuccessors(next);
//
//    next = val(chessBoard, next, 'S');

    printChessboard(chessBoard, chessboard_size, chessboard_size * chessboard_size);
    int result = chessboardSearch(chessBoard, chessboard_size, knightPosition, bishopPosition, chessPiece, 0, max_depth,
                                  0,
                                  max_depth, 'S');
    printf("Best score is %d\n", result);
    free(chessBoard);

    return 0;
}