#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <time.h>

int bestDepth;
int *bestMoves;

typedef struct {
    int rowIndex;
    int colIndex;
} Position;

typedef struct {
    char *board;
    int boardSize;

    int depth;
    int maxDepth;
    int *moves;

    int knightIndex;
    int bishopIndex;
    char figureMove;

    int boardFigures;
    int takeFiguresCount;

} ChessSearch;

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

void swapState(ChessSearch *xp, ChessSearch *yp) {
    ChessSearch temp = *xp;
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

Position mapIndexInverse(int index, int chessboardSize) {
    Position position;
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
        Position position = mapIndexInverse(successor, chessboardSize);
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

void printFigureCapturedMove(int moveOrder, char figure, Position startPosition, Position endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]*\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printFigureMove(int moveOrder, char figure, Position startPosition, Position endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printMoves(int *moves, int movesCount, int chessboardSize, int knightIndex, int bishopIndex) {
    char figure;
    Position knightPos = mapIndexInverse(knightIndex, chessboardSize);
    Position bishopPos = mapIndexInverse(bishopIndex, chessboardSize);

    for (int i = 0; i < movesCount; i++) {
        figure = i % 2 == 0 ? 'S' : 'J';
        Position nextMovePos;
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
            Position startPosition;
            if (moves[i - 2] > 0) {
                //nextMovePos = mapIndexInverse(moves[i - 2], boardSize);
                startPosition = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                //nextMovePos = mapIndexInverse(moves[i - 2] * -1, boardSize);
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
    Position position = mapIndexInverse(startIndex, chessboardSize);
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
    Position position = mapIndexInverse(startIndex, chessboardSize);

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
    //int index = mapIndex(position.rowIndex, position.colIndex, boardSize);
    if (chessboard[startIndex] == 'J') {
        return nextKnight(chessboard, chessboardSize, successors, startIndex);
    } else if (chessboard[startIndex] == 'S') {
        return nextBishop(chessboard, chessboardSize, successors, startIndex);
    } else {
        printf("Invalid position '%c' for J/S.\n", chessboard[startIndex]);
        return -1;
    }
}

ChessSearch createStateFromPredecessor(ChessSearch state, int nextMoveIndex) {
    ChessSearch nextState;

    char nextMove;
    int nextBishopIndex = state.bishopIndex;
    int nextKnightIndex = state.knightIndex;
    if (state.figureMove == 'S') {
        nextBishopIndex = nextMoveIndex;
        nextMove = 'J';
    } else {
        nextKnightIndex = nextMoveIndex;
        nextMove = 'S';
    }

    nextState.boardSize = state.boardSize;
    nextState.bishopIndex = nextBishopIndex;
    nextState.knightIndex = nextKnightIndex;
    nextState.figureMove = nextMove;
    nextState.depth = state.depth + 1;
    nextState.maxDepth = state.maxDepth;
    nextState.boardFigures = state.boardFigures;

    return nextState;
}

void dfsChessboard(ChessSearch chessSearch) {
    // lower bound
    if (chessSearch.depth == chessSearch.boardFigures && chessSearch.boardFigures == chessSearch.takeFiguresCount) {
#pragma omp critical
        {
            if (chessSearch.depth == chessSearch.boardFigures &&
                chessSearch.boardFigures == chessSearch.takeFiguresCount) {
                bestDepth = chessSearch.depth;
                copyIntArray(chessSearch.moves, bestMoves, chessSearch.maxDepth);
            }
        }
        return;
    }

    // upper bound (init best depth is max depth)
    if (chessSearch.depth >= bestDepth) {
        return;
    }

    // update best depth
    if (chessSearch.boardFigures == chessSearch.takeFiguresCount && chessSearch.depth < bestDepth) {
#pragma omp critical
        {
            if (chessSearch.boardFigures == chessSearch.takeFiguresCount && chessSearch.depth < bestDepth) {
                bestDepth = chessSearch.depth;
                copyIntArray(chessSearch.moves, bestMoves, chessSearch.maxDepth);
            }
        }
        return;
    }

    // Branch and bound
    if (chessSearch.depth + (chessSearch.boardFigures - chessSearch.takeFiguresCount) >= bestDepth) {
        return;
    }

    int startMoveIndex;
    char nextMove;
    int *successors;
    if (chessSearch.figureMove == 'S') {
        nextMove = 'J';
        startMoveIndex = chessSearch.bishopIndex;
        successors = (int *) malloc(sizeof(int) * (2 * chessSearch.boardSize - 2));
    } else {
        nextMove = 'S';
        startMoveIndex = chessSearch.knightIndex;
        successors = (int *) malloc(sizeof(int) * 8);
    }

    // get all possible moves
    int successorSize = next(chessSearch.board, chessSearch.boardSize, successors, startMoveIndex);
    val(chessSearch.board, chessSearch.boardSize, successors, successorSize, chessSearch.figureMove);

    for (int i = 0; i < successorSize; i++) {
        // copy board
        char *chessboardCopy = (char *) malloc(sizeof(char) * chessSearch.boardSize * chessSearch.boardSize);
        if (chessboardCopy == NULL) {
            printf("Memory leak.\n");
            exit(1);
        }
        copyArray(chessSearch.board, chessboardCopy, chessSearch.boardSize * chessSearch.boardSize);

        int startPosition = chessSearch.figureMove == 'S' ? chessSearch.bishopIndex : chessSearch.knightIndex;

        int capturedFigure = move(chessboardCopy, chessSearch.boardSize, startPosition, successors[i]);
        if (capturedFigure == 0) {
            chessSearch.moves[chessSearch.depth] = successors[i] * -1;
        } else if (capturedFigure == 1) {
            chessSearch.moves[chessSearch.depth] = successors[i];
        }

        // update index position after move
        int nextBishopIndex = chessSearch.bishopIndex;
        int nextKnightIndex = chessSearch.knightIndex;
        if (chessSearch.figureMove == 'S') {
            nextBishopIndex = successors[i];
        } else {
            nextKnightIndex = successors[i];
        }

        ChessSearch state;
        state.board = chessboardCopy;
        state.boardSize = chessSearch.boardSize;
        state.boardFigures = chessSearch.boardFigures;

        state.bishopIndex = nextBishopIndex;
        state.knightIndex = nextKnightIndex;
        state.maxDepth = chessSearch.maxDepth;
        state.depth = chessSearch.depth + 1;

        state.moves = chessSearch.moves;
        state.takeFiguresCount = chessSearch.takeFiguresCount + capturedFigure;
        state.figureMove = nextMove;

        dfsChessboard(state);
        free(chessboardCopy);
    }

    free(successors);
}

int getNextStates(ChessSearch previousState, ChessSearch *nextStates) {
    int startMoveIndex;
    char nextMove;
    int *nextMoves = (int *) malloc(sizeof(int) * (2 * previousState.boardSize - 2));

    if (previousState.figureMove == 'S') {
        nextMove = 'J';
        startMoveIndex = previousState.bishopIndex;
    } else {
        nextMove = 'S';
        startMoveIndex = previousState.knightIndex;
    }

    int nextMovesSize = next(previousState.board, previousState.boardSize, nextMoves, startMoveIndex);
    val(previousState.board, previousState.boardSize, nextMoves, nextMovesSize, nextMove);

    for (int i = 0; i < nextMovesSize; i++) {
        char *chessboardCopy = (char *) malloc(sizeof(char) * previousState.boardSize * previousState.boardSize);
        int *movesCopy = (int *) malloc(sizeof(int) * previousState.maxDepth);

        copyArray(previousState.board, chessboardCopy, previousState.boardSize * previousState.boardSize);
        copyIntArray(previousState.moves, movesCopy, previousState.maxDepth);

        int startPosition = previousState.figureMove == 'S' ? previousState.bishopIndex : previousState.knightIndex;
        int capturedFigure = move(chessboardCopy, previousState.boardSize, startPosition, nextMoves[i]);
        if (capturedFigure == 0) {
            movesCopy[previousState.depth] = nextMoves[i] * -1;
        } else if (capturedFigure == 1) {
            movesCopy[previousState.depth] = nextMoves[i];
        }

        ChessSearch nextState = createStateFromPredecessor(previousState, nextMoves[i]);
        nextState.board = chessboardCopy;
        nextState.moves = movesCopy;
        nextState.takeFiguresCount = capturedFigure;

        nextStates[i] = nextState;
    }

    return nextMovesSize;
}

ChessSearch parseInitialState(int argc, char *argv[]) {
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

    ChessSearch initialState;
    initialState.board = chessboard;
    initialState.boardSize = chessboardSize;
    initialState.boardFigures = boardFigures;

    initialState.bishopIndex = bishopStartIndex;
    initialState.knightIndex = knightStartIndex;
    initialState.maxDepth = maxDepth;
    initialState.depth = 0;

    initialState.moves = moves;
    initialState.takeFiguresCount = 0;
    initialState.figureMove = 'S';

    return initialState;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);

//    int chessboardStartArg = 3;
//    char *p_chessboardSize;
//    p_chessboardSize = *(argv + 1);
//
//    char *p_maxDepth;
//    p_maxDepth = *(argv + 2);
//
//    int chessboardSize = atoi(p_chessboardSize);
//    int maxDepth = atoi(p_maxDepth);
//
//    int oneDimensionSize = chessboardSize * chessboardSize;
//
//    char *chessboard = (char *) malloc(sizeof(char) * oneDimensionSize);
//    int *moves = (int *) malloc(sizeof(int) * maxDepth);
//
//    int knightStartIndex;
//    int bishopStartIndex;
//    int boardFigures = 0;
//
//// Parse instance from CMD
//    for (int i = chessboardStartArg; i < argc; i++) {
//        int size = strlen(argv[i]);
//        for (int j = 0; j < size; j++) {
//            int index = mapIndex(i - chessboardStartArg, j, chessboardSize);
//            chessboard[index] = argv[i][j];
//            if (chessboard[index] == 'J') {
//                knightStartIndex = mapIndex(i - chessboardStartArg, j, chessboardSize);
//            }
//            if (chessboard[index] == 'S') {
//                bishopStartIndex = mapIndex(i - chessboardStartArg, j, chessboardSize);
//            }
//            if (chessboard[index] == 'P')
//                boardFigures++;
//        }
//    }
//
//    ChessSearch initialState;
//    initialState.board = chessboard;
//    initialState.boardSize = chessboardSize;
//    initialState.boardFigures = boardFigures;
//
//    initialState.bishopIndex = bishopStartIndex;
//    initialState.knightIndex = knightStartIndex;
//    initialState.maxDepth = maxDepth;
//    initialState.depth = 0;
//
//    initialState.moves = moves;
//    initialState.takeFiguresCount = 0;
//    initialState.figureMove = 'S';

    ChessSearch initialState = parseInitialState(argc, argv);
    bestDepth = maxDepth;
    bestMoves = (int *) malloc(sizeof(int) * maxDepth);

//    dfsChessboard(chessboard, chessboardSize, moves, knightStartIndex, bishopStartIndex, boardFigures,
//                  0, maxDepth, 0, 'S');

    clock_t start = clock();

    ChessSearch currState = initialState;
    int statesCount = 100;
    ChessSearch states[statesCount];

    int counter = 0;
    int curr = 0;
    while (counter < statesCount) {
        ChessSearch nextStates[statesCount];
        int nextStateSize = getNextStates(currState, nextStates);
        for (int i = 0; i < nextStateSize; i++) {
            if (counter < statesCount) {
                states[counter] = nextStates[i];
                counter++;
            }
        }
        currState = states[curr];
        curr++;
    }

    for (int i = 0; i < statesCount - 1; i++) {
        for (int j = 0; j < statesCount - i - 1; j++) {
            if (states[j].takeFiguresCount <= states[j + 1].takeFiguresCount) {
                swapState(&states[j], &states[j + 1]);
            }
        }
    }

    printf("Running parallel\n");
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < statesCount - 1; i++) {
        dfsChessboard(states[i]);
    }

    //dfsChessboard(initialState);
    printMoves(bestMoves, bestDepth, chessboardSize, knightStartIndex, bishopStartIndex);

    clock_t end = clock();

    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Best score is %d. The program finished after %.6f seconds.\n", bestDepth, seconds);

    free(chessboard);
    free(moves);
    free(bestMoves);

    return 0;
}