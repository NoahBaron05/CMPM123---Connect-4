#pragma once
#include "Game.h"

class Connect4 : public Game
{
public:
    Connect4();
    ~Connect4();

    // Required virtual methods from Game base class
    void        setUpBoard() override;
    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    void        stopGame() override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    // AI methods
    void        updateAI() override;
    bool        gameHasAI() override;
    Grid*       getGrid() override { return _grid; }

private:
    // Constants for piece types
    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    // Player constants
    static const int RED_PLAYER = 0;
    static const int YELLOW_PLAYER = 1;

    // Helper methods
    Bit*        pieceForPlayer(const int playerNumber);
    bool        isValidMove(int srcX, int srcY, int dstX, int dstY, Player* player) const;
    void        getBoardPosition(BitHolder &holder, int &x, int &y) const;
    bool        isValidSquare(int x, int y) const;
    int         getLowestEmptyRow(int column) const;
    bool        isInsideBoard(int x, int y) const;
    bool        checkDirection(int startX, int startY, int deltaX, int deltaY, Player*& winner) const;

    // AI Helper methods
    std::string      makeMove(const std::string& state, int col, int playerNum);
    int              negamax(const std::string& state, int depth, int alpha, int beta, char player);
    int              evaluate(const std::string& state, char player);
    int              evaluateWindow(const std::vector<char>& window, char player);
    bool             isWinningState(const std::string& state, char player);

    // Board representation
    Grid*        _grid;

    // Game state
    int         _redPieces;
    int         _yellowPieces;
};