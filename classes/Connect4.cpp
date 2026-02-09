#include "Connect4.h"

Connect4::Connect4() : Game() {
    _grid = new Grid(7, 6);
}

Connect4::~Connect4() {
    delete _grid;
}

void Connect4::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;

    //Initialize all squares
    _grid->initializeSquares(80, "square.png");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
}

Bit* Connect4::pieceForPlayer(const int playerNumber) {
    Bit *bit = new Bit();
    bit->LoadTextureFromFile(playerNumber == RED_PLAYER ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

bool Connect4::actionForEmptyHolder(BitHolder &holder) {
    if (holder.bit()) return false;

    int x, y;
    getBoardPosition(holder, x, y);

    int targetRow = getLowestEmptyRow(x);
    if (targetRow == -1) return false; // Column is full

    Bit* bit = pieceForPlayer(getCurrentPlayer()->playerNumber());
    if (bit) {
        ChessSquare* topSquare = _grid->getSquare(x, 0);
        ChessSquare* targetSquare = _grid->getSquare(x, targetRow);
        bit->setPosition(topSquare->getPosition());
        targetSquare->setBit(bit);
        bit->moveTo(targetSquare->getPosition());
        endTurn();
        return true;
    }

    return false;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src) {
    // Connect-4 pieces are never moved once placed
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    // Connect-4 has no piece movement
    return false;
}

void Connect4::getBoardPosition(BitHolder &holder, int &x, int &y) const
{
    x = -1;
    y = -1;

    _grid->forEachSquare([&](ChessSquare* square, int sx, int sy)
    {
        if (square == &holder)
        {
            x = sx;
            y = sy;
        }
    });
}

int Connect4::getLowestEmptyRow(int column) const
{
    if (!_grid) return -1;

    for (int row = _grid->getHeight() - 1; row >= 0; row--)
    {
        ChessSquare* square = _grid->getSquare(column, row);
        if (square && square->bit() == nullptr)
        {
            return row;
        }
    }

    return -1; // column full
}

Player* Connect4::checkForWinner() {
    Player* winner = nullptr;

    for (int y = 0; y < _grid->getHeight(); y++){
        for (int x = 0; x < _grid->getWidth(); x++){
            if (checkDirection(x, y, 1, 0, winner)) return winner;
            if (checkDirection(x, y, 0, 1, winner)) return winner;
            if (checkDirection(x, y, 1, 1, winner)) return winner;
            if (checkDirection(x, y, 1, -1, winner)) return winner;
        }
    }

    return nullptr;
}

bool Connect4::isInsideBoard(int x, int y) const {
    return x >= 0 && 
           x < _grid->getWidth() && 
           y >= 0 && 
           y < _grid->getHeight();
}

bool Connect4::checkDirection(int startX, int startY, int deltaX, int deltaY, Player*& winner) const {
    ChessSquare* startSquare = _grid->getSquare(startX, startY);
    if (!startSquare || !startSquare->bit()) return false;

    Player* owner = startSquare->bit()->getOwner();
    int count = 1;

    for (int i = 1; i < 4; i++){
        int x = startX + deltaX * i;
        int y = startY + deltaY * i;

        if (!isInsideBoard(x, y)) return false;

        ChessSquare* square = _grid->getSquare(x, y);
        if (!square || !square->bit()) return false;

        if (square->bit()->getOwner() != owner) return false;
        
        count ++;
    }

    if (count != 4) return false;

    winner = owner;
    return true;

}

bool Connect4::checkForDraw() {
    for (int y = 0; y < _grid->getHeight(); y++) {
        for (int x = 0; x < _grid->getWidth(); x++) {
            if (!_grid->getSquare(x, y)->bit()) {
                return false; // Found an empty square, not a draw
            }
        }
    }
    return true; // No empty squares, it's a draw
}

std::string Connect4::initialStateString() {
    return "0000000"
           "0000000"
           "0000000"
           "0000000"
           "0000000"
           "0000000";
}

std::string Connect4::stateString() {
    std::string s = initialStateString();
    int width = _grid->getWidth();
    int height = _grid->getHeight();

    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit* bit = square->bit();
        if (bit) {
            // Compute correct index in the string
            int index = y * width + x;
            s[index] = '0' + (bit->getOwner()->playerNumber() + 1);
        }
    });

    return s;
}

void Connect4::setStateString(const std::string &s) {
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        size_t index = y * _grid->getWidth() + x;
        if (index < s.size()){
            char c = s[index];
            if (c >= '1' && c <= '2'){
                int playerNum = c - '1';
                Bit* bit = pieceForPlayer(playerNum);
                square->setBit(bit);
            } else {
                square->setBit(nullptr);
            }
        }
    });
}

void Connect4::stopGame() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

void Connect4::updateAI() {
    Player* aiPlayer = getCurrentPlayer();
    if (!aiPlayer->isAIPlayer()) return;

    int bestMove = -1;
    int bestScore = -1000000;
    int depth = 5;

    std::string state = stateString();
    char aiChar = '2';
    char humanChar = '1';

    for (int col = 0; col < 7; col++) {
        // Check if column is full
        int row = -1;
        for (int r = 5; r >= 0; r--) {
            if (state[r*7 + col] == '0') {
                row = r;
                break;
            }
        }
        if (row == -1) continue; // Column is full

        // Simulate AI move
        std::string child = makeMove(state, col, 2); // AI is '2'

        // Check for winning move immeadiately before negamax recursion
        if (isWinningState(child, '2')) {
            actionForEmptyHolder(*_grid->getSquare(col, row));
            return; // Take the win immediately
        }
        int score = -negamax(child, depth - 1, -1000000, 1000000, humanChar);

        if (score > bestScore) {
            bestScore = score;
            bestMove = col;
        }
    }

    // Place AI piece
    if (bestMove != -1) {
        int row = getLowestEmptyRow(bestMove);
        if (row != -1) {
            actionForEmptyHolder(*_grid->getSquare(bestMove, row));
        }
    }
}

int Connect4::evaluate(const std::string& state, char player) {
    int score = 0;

    int columnWeights[7] = {3, 4, 5, 7, 5, 4, 3};

    for(int y = 0; y < 6; y++){
        for(int x = 0; x < 7; x++){
            if(state[y*7 + x] == player) {
                score += columnWeights[x];
            }
        }
    }

    // Horizontal
    for(int y = 0; y < 6; y++){
        for(int x = 0; x <= 3; x++){
            std::vector<char> window = {
                state[y*7 + x], state[y*7 + x+1], state[y*7 + x+2], state[y*7 + x+3]
            };
            score += evaluateWindow(window, player);
        }
    }

    // Vertical
    for(int x = 0; x < 7; x++){
        for(int y = 0; y <= 2; y++){
            std::vector<char> window = {
                state[y*7 + x], state[(y+1)*7 + x], state[(y+2)*7 + x], state[(y+3)*7 + x]
            };
            score += evaluateWindow(window, player);
        }
    }

    // Diagonal 
    for(int y = 0; y <= 2; y++){
        for(int x = 0; x <= 3; x++){
            std::vector<char> window = {
                state[y*7 + x], state[(y+1)*7 + x+1], state[(y+2)*7 + x+2], state[(y+3)*7 + x+3]
            };
            score += evaluateWindow(window, player);
        }
    }

    // Diagonal /
    for(int y = 0; y <= 2; y++){
        for(int x = 3; x < 7; x++){
            std::vector<char> window = {
                state[y*7 + x], state[(y+1)*7 + x-1], state[(y+2)*7 + x-2], state[(y+3)*7 + x-3]
            };
            score += evaluateWindow(window, player);
        }
    }

    return score;
}

int Connect4::evaluateWindow(const std::vector<char>& window, char player) {
    char opponent = (player == '1') ? '2' : '1';
    int score = 0;

    int playerCount = std::count(window.begin(), window.end(), player);
    int oppCount = std::count(window.begin(), window.end(), opponent);
    int emptyCount = std::count(window.begin(), window.end(), '0');

    if(playerCount == 4) score += 1000000;
    else if(playerCount == 3 && emptyCount == 1) score += 100;
    else if(playerCount == 2 && emptyCount == 2) score += 10;

    if(oppCount == 3 && emptyCount == 1) score -= 80; 
    else if(oppCount == 2 && emptyCount == 2) score -= 5;

    return score;
}

std::string Connect4::makeMove(const std::string& state, int col, int playerNum){
    int width = 7;
    int height = 6;
    std::string newState = state;

    for(int row = height-1; row >= 0; row--){
        int index = row * width + col;
        if(newState[index] == '0'){
            newState[index] = '0' + playerNum;
            break;
        }
    }

    return newState;
}

int Connect4::negamax(const std::string& state, int depth, int alpha, int beta, char player){
    if (depth == 0) return evaluate(state, player);

    int maxEval = -1000000;
    char opponent = (player == '1') ? '2' : '1';

    for (int col = 0; col < 7; col++) {
        // Skip full column
        if (state[col] != '0') continue;

        std::string child = makeMove(state, col, player - '0'); // player '1'->1
        int eval = -negamax(child, depth - 1, -beta, -alpha, opponent);

        maxEval = std::max(maxEval, eval);
        alpha = std::max(alpha, eval);
        if (alpha >= beta) break; // alpha-beta pruning
    }

    return maxEval;
}

bool Connect4::isWinningState(const std::string& state, char player) {
    // Horizontal
    for (int y = 0; y < 6; y++)
        for (int x = 0; x <= 3; x++)
            if (state[y*7+x] == player && state[y*7+x+1] == player &&
                state[y*7+x+2] == player && state[y*7+x+3] == player)
                return true;

    // Vertical
    for (int x = 0; x < 7; x++)
        for (int y = 0; y <= 2; y++)
            if (state[y*7+x] == player && state[(y+1)*7+x] == player &&
                state[(y+2)*7+x] == player && state[(y+3)*7+x] == player)
                return true;

    // Diagonal
    for (int y = 0; y <= 2; y++)
        for (int x = 0; x <= 3; x++)
            if (state[y*7+x] == player && state[(y+1)*7+x+1] == player &&
                state[(y+2)*7+x+2] == player && state[(y+3)*7+x+3] == player)
                return true;

    // Diagonal
    for (int y = 0; y <= 2; y++)
        for (int x = 3; x < 7; x++)
            if (state[y*7+x] == player && state[(y+1)*7+x-1] == player &&
                state[(y+2)*7+x-2] == player && state[(y+3)*7+x-3] == player)
                return true;

    return false;
}

bool Connect4::gameHasAI() {
    return _gameOptions.AIPlaying; // true if AI is playing at all
}