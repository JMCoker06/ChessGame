#include <iostream>
#include <vector>
#include <string>
#include <memory> // for unique_ptr
#include <cstddef> // for nullptr
#include <optional> // represents "no move"
#include <stdexcept> // for bounds checking exceptions
#include <cstdlib> // for nullptr support
#include <climits> // for additional compatibility
#include <map> 
#include <chrono> 
using namespace std;
enum class Color {White, Black};
struct Move // allows the player to be able to move their pieces in-game
{
    int fromRow, fromCol;
    int toRow, toCol;
};
struct MoveRecord
{
    Move move;
    Piece* capturedPiece; // nullptr if no capture
    bool wasEnPassant;
    int enPassantCapturedRow; // row of pawn captured
    int enPassantCapturedCol;
    bool wasPromotion;
    Piece* originalPawn; // pawn before promotion
    Color colorBeforeMove;
    Move previousLastMove; // restores Game's lastMove state
    bool previousLastMoveWasTwoSquarePawn;
    bool pieceHadMovedBefore; // King/Rook hasMoved flag restoration 
};
//Forward declarations
class Piece;
class Square;
class Board;
//Defining Piece class 
class Piece
{
    private:
    int row, col;
    Color color;
    public:
    Piece(int r, int c, Color color) : row(r), col(c), color(color) {}
    Color getColor() const { return color; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    void setRow(int r) { row = r; }
    void setCol(int c) { col = c; }
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const = 0;
    virtual ~Piece() = default;
};
//Defining Square class 
class Square
{
    private:
    Piece* piece = nullptr;
    bool isLight; // true = light square, false = dark square
    public:
    Square(bool light) : isLight(light) {}
    Piece* getPiece() const { return piece; }
    void setPiece(Piece* p) { piece = p; }
    bool getIsLight() const { return isLight; }
};
//Defining Board class
class Board
{
    private:
    unique_ptr<Piece> squares[8][8];
    public:
    Board();
    Piece* getPiece(int row, int col) const;
    void setPiece(int row, int col, Piece* piece);
    Piece* releasePiece(int row, int col); // for temporarily taking ownership away without deleting
    void print() const;
};
//Derived classes that define specific piece types
class Pawn : public Piece
{
    public:
    Pawn(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        // Pawn move logic here
        int r = getRow();
        int c = getCol();
        // White moves up the board (which decreases row index)
        // Black moves down the board (which increases row index)
        int direction = (getColor() == Color::White) ? -1 : 1;
        int startRow = (getColor() == Color::White) ? 6 : 1;
        //Move forward one square (if empty)
        int oneStep = r + direction;
        if (oneStep >= 0 && oneStep < 8 && !board.getPiece(oneStep, c))
        {
            moves.push_back({r, c, oneStep, c});
        }
        // Move foward two squares (if empty and on starting row)
        int twoStep = r + 2 * direction;
        if (twoStep >= 0 && twoStep < 8 && r == startRow && !board.getPiece(twoStep, c))
        {
            moves.push_back({r, c, twoStep, c});
        }
        // Diagonal captures (if enemy piece is present)
        int captureRow = r + direction; 
        if (captureRow >= 0 && captureRow < 8)
        {
            // Check left diagonal
            if (c > 0 && board.getPiece(captureRow, c - 1) && board.getPiece(captureRow, c - 1)->getColor() != getColor())
            {
                moves.push_back({r, c, captureRow, c - 1});
            }
            // Check right diagonal
            if (c < 7 && board.getPiece(captureRow, c + 1) && board.getPiece(captureRow, c + 1)->getColor() != getColor())
            {
                moves.push_back({r, c, captureRow, c + 1});
            }
        }
        // En passant (tracks the last move made)
        int enPassantRow = (getColor() == Color::White) ? 3 : 4;
        if (r == enPassantRow && lastMove was a two-square pawn push)
        {
            if (lastMove.toCol == c - 1 || lastMove.toCol == c + 1)
            {
                moves.push_back({r, c, r + direction, lastMove.toCol});
            }
        }
        return moves;
    }
};
class Bishop : public Piece
{
    public:
    Bishop(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        int r = getRow();
        int c = getCol();   
        // Bishop move logic here
        // 4 diagonal directions: (row offset, col offset)
        int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
        for (const auto& dir : directions)
        {
            int newRow = r + dir[0];
            int newCol = c + dir[1];
            while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                    break;
                }
                newRow += dir[0];
                newCol += dir[1];
            }
        }
        return moves;
    }
};
class Knight : public Piece
{
    public:
    Knight(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        // Knight move logic here
        int r = getRow();
        int c = getCol();
        // 8 possible knight moves: (row offset, col offset)
        int directions[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
        for (const auto& dir : directions)
        {
            int newRow = r + dir[0];
            int newCol = c + dir[1];
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                }
            }
        }
        return moves;
    }
};
class Rook : public Piece
{
    bool hasMoved = false; // for castling logic
    public:
    Rook(int r, int c, Color color) : Piece(r, c, color) {}
    void setHasMoved(bool moved) { hasMoved = moved; }
    bool getHasMoved() const { return hasMoved; }
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        // Rook move logic here
        int r = getRow();
        int c = getCol();   
        // 4 cardinal directions: (row offset, col offset)
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int i = 0; i < 4; i++)
        {
            int newRow = r + directions[i][0];
            int newCol = c + directions[i][1];
            while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                    break;
                }
                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
        return moves;
    }
};
class Queen : public Piece
{
    public:
    Queen(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        // Queen move logic here
        // Combine the move logic of the Rook and the Bishop
        int r = getRow();
        int c = getCol();
        // 4 cardinal directions: (row offset, col offset)
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int i = 0; i < 4; i++)
        {
            int newRow = r + directions[i][0];
            int newCol = c + directions[i][1];
            while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                    break;
                }
                newRow += directions[i][0];
                newCol += directions[i][1];
            }
        }
        // 4 diagonal directions: (row offset, col offset)
        int diagonalDirections[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
        for (int i = 0; i < 4; i++)
        {
            int newRow = r + diagonalDirections[i][0];
            int newCol = c + diagonalDirections[i][1];
            while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                    break;
                }
                newRow += diagonalDirections[i][0];
                newCol += diagonalDirections[i][1];
            }
        }
        return moves;
    }
};
class King : public Piece
{
    bool hasMoved = false; // for castling logic
    public:
    King(int r, int c, Color color) : Piece(r, c, color) {}
    void setHasMoved(bool moved) { hasMoved = moved; }
    bool getHasMoved() const { return hasMoved; }
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col, Move lastMove = {-1, -1, -1, -1}, bool lastMoveWasTwoSquarePawn = false) const override
    {
        vector<Move> moves;
        // King move logic here
        int r = getRow();
        int c = getCol();
        // 8 possible directions: (row offset, col offset)
        int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
        for (int i = 0; i < 8; i++)
        {
            int newRow = r + directions[i][0];
            int newCol = c + directions[i][1];
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (!board.getPiece(newRow, newCol))
                {
                    moves.push_back({r, c, newRow, newCol});
                }
                else
                {
                    if (board.getPiece(newRow, newCol)->getColor() != getColor())
                    {
                        moves.push_back({r, c, newRow, newCol});
                    }
                }
            }
        }
        return moves;
    }
};
class Game
{
    Board board;    // using the Board class
    Color currentTurn;
    Move lastMove;
    bool lastMoveWasTwoSquarePawnPush = false; // for en passant logic
    vector<pair<Move, Piece*>> moveHistory; // for undo functionality
    vector<MoveRecord> moveHistory;
    map<string, int> positionHistory;
    bool gameOver = false;
    chrono::seconds whiteTimeRemaining;
    chrono::seconds blackTimeRemaining;
    chrono::steady_clock::time_point turnStartTime;
    public:
    Game();
    ~Game();
    void initializeBoard();
    void printBoard() const;
    bool movePiece(int fromRow, int fromCol, int toRow, int toCol);
    void undoMove();
    bool canCastle(Color color, bool kingside) const;
    void performCastle(Color color, bool kingside);
    bool isInCheck(Color color) const;
    bool isSquareAttacked(int row, int col, Color attackerColor) const;
    bool isStalemate(Color color);
    bool isThreefoldRepetition() const;
    void checkGameState();
    void startTurnTimer();
    bool checkTimeAndDeductElapsed(); 
    chrono::seconds getTimeRemaining(Color color) const;
    vector<Move> filterLegalMoves(Piece* piece, const vector<Move>& candidates);
    bool isGameOver () const { return gameOver; }
    vector<Move> getAllLegalMoves(Color color);
    void undoMove();
    Piece* getPieceAt(int row, int col) const;
    string getPositionKey() const;
};
Board::Board()
{
    // Initialize the board with nullptrs (empty squares)
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            squares[r][c] = nullptr;
        }
    };
}
Piece* Board::releasePiece(int row, int col)
{
    return squares[row][col].release(); // gives up ownership without deleting
}

Piece *Board::getPiece(int row, int col) const
{
    return squares[row][col].get(); //extracts raw pointer from unique_ptr
}
void Board::setPiece(int row, int col, Piece* piece)
{
    squares[row][col].reset(piece); // transfer ownership to unique ptr
};
void Board::print() const
{
    for (int r = 0; r < 8; r++)
    {
        cout << 8 - r << " ";
        for (int c = 0; c < 8; c++)
        {
            Piece* piece = getPiece(r, c);
            if (piece)
            {
                char symbol = '?';
                if      (dynamic_cast<Pawn*>(piece))   symbol = 'P';
                else if (dynamic_cast<Rook*>(piece))   symbol = 'R';
                else if (dynamic_cast<Knight*>(piece)) symbol = 'N';
                else if (dynamic_cast<Bishop*>(piece)) symbol = 'B';
                else if (dynamic_cast<Queen*>(piece))  symbol = 'Q';
                else if (dynamic_cast<King*>(piece))   symbol = 'K';
                if (piece->getColor() == Color::Black)
                    symbol = tolower(symbol);
                cout << symbol << " ";
            }
            else
            {
                cout << ". ";
            }
        }
        cout << "\n";
    }
    cout << "  a b c d e f g h\n";
}
Game::Game() : currentTurn(Color::White), lastMove{0, 0, 0, 0}, lastMoveWasTwoSquarePawnPush(false), 
whiteTimeRemaining(chrono::seconds(600)), blackTimeRemaining(chrono::seconds(600))
{
    initializeBoard();
}
Game::~Game()
{
    // Destructor logic if needed (e.g., cleanup)
}
void Game::initializeBoard()
{
    // Implementation for initializing the board
    //Black Pieces
    board.setPiece(0, 0, new Rook(0, 0, Color::Black));
    board.setPiece(0, 1, new Knight(0, 1, Color::Black));
    board.setPiece(0, 2, new Bishop(0, 2, Color::Black));
    board.setPiece(0, 3, new Queen(0, 3, Color::Black));
    board.setPiece(0, 4, new King(0, 4, Color::Black));
    board.setPiece(0, 5, new Bishop(0, 5, Color::Black));
    board.setPiece(0, 6, new Knight(0, 6, Color::Black));
    board.setPiece(0, 7, new Rook(0, 7, Color::Black));
    for (int c = 0; c < 8; c++)
    {
        board.setPiece(1, c, new Pawn(1, c, Color::Black));
    }

    //White Pieces
    board.setPiece(7, 0, new Rook(7, 0, Color::White));
    board.setPiece(7, 1, new Knight(7, 1, Color::White));
    board.setPiece(7, 2, new Bishop(7, 2, Color::White));
    board.setPiece(7, 3, new Queen(7, 3, Color::White));
    board.setPiece(7, 4, new King(7, 4, Color::White));
    board.setPiece(7, 5, new Bishop(7, 5, Color::White));
    board.setPiece(7, 6, new Knight(7, 6, Color::White));
    board.setPiece(7, 7, new Rook(7, 7, Color::White));
    for (int c = 0; c < 8; c++)
    {
        board.setPiece(6, c, new Pawn(6, c, Color::White));
    }
};
void Game::printBoard() const
{
    board.print();

    auto formatTime = [](chrono::seconds s)
    {
        int totalSeconds = max(0, (int)s.count());
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds);
    };

    cout << "White time: " << formatTime(whiteTimeRemaining) << "\n";
    cout << "Black time: " << formatTime(blackTimeRemaining) << "\n";
}
bool Game::movePiece(int fromRow, int fromCol, int toRow, int toCol)
{
    Piece* piece = board.getPiece(fromRow, fromCol);

    if (piece == nullptr || piece->getColor() != currentTurn)
        return false;

    vector<Move> candidates = piece->getLegalMoves(board, fromRow, fromCol, lastMove, lastMoveWasTwoSquarePawnPush);
    vector<Move> legalMoves = filterLegalMoves(piece, candidates);

    for (int i = 0; i < legalMoves.size(); i++)
    {
        const Move& move = legalMoves[i];
        if (move.toRow == toRow && move.toCol == toCol)
        {
            MoveRecord record;
            record.move = move;
            record.previousLastMove = lastMove;
            record.previousLastMoveWasTwoSquarePawn = lastMoveWasTwoSquarePawnPush;
            record.colorBeforeMove = currentTurn;
            record.wasPromotion = false;
            record.originalPawn = nullptr;
            // Detecting en passant (pawn moves diagonally to empty square)
            bool isEnPassant = dynamic_cast<Pawn*>(piece) != nullptr && fromCol != toCol && board.getPiece(toRow, toCol) == nullptr;
            if (isEnPassant)
            {
                record.enPassantCapturedRow = fromRow;
                record.enPassantCapturedCol = toCol;
                // Captured pawn must be sitting on same row as moving pawn
                // Must be moving to same column
                record.capturedPiece = board.releasePiece(fromRow, toCol); //removing captured pawn
            }
            else
            {
                record.capturedPiece = board.getPiece(toRow, toCol);
                board.releasePiece(toRow, toCol);
            }

            // Records King/Rook hasMoved state before changing it
            King* kingPtr = dynamic_cast<King*>(piece);
            Rook* rookPtr = dynamic_cast<Rook*>(piece);
            if (kingPtr) record.pieceHadMovedBefore = kingPtr->getHasMoved();
            else if (rookPtr) record.pieceHadMovedBefore = rookPtr->getHasMoved();
            
            // Release ownership before moving
            // Perform move
            board.releasePiece(fromRow, fromCol);
            board.releasePiece(toRow, toCol);  // discards captured piece

            board.setPiece(toRow, toCol, piece);
            board.setPiece(fromRow, fromCol, nullptr);
            piece->setRow(toRow);
            piece->setCol(toCol);

            if (kingPtr) kingPtr->setHasMoved(true);
            if (rookPtr) rookPtr->setHasMoved(true);

            if (dynamic_cast<Pawn*>(piece) && (toRow == 0 || toRow == 7))
            {
                char choice;
                cout << "Promote pawn to (Q)ueen, (R)ook, (B)ishop, or (K)night: ";
                cin >> choice;

                Piece* promoted = nullptr;
                switch (toupper(choice))
                {
                    case "R": promoted = new Rook(toRow, toCol, piece->getColor()); break;
                    case "B": promoted = new Bishop(toRow, toCol, piece->getColor()); break;
                    case "K": promoted = new Knight(toRow, toCol, piece->getColor()); break;
                    default: promoted = new Queen(toRow, toCol, piece->getColor()); break;
                }
                board.releasePiece(toRow, toCol);
                board.setPiece(toRow, toCol, promoted);
            }
            lastMove = move;
            lastMoveWasTwoSquarePawnPush = (dynamic_cast<Pawn*>(piece) != nullptr &&
                                            abs(move.toRow - move.fromRow) == 2);
            currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
            moveHistory.push_back(record);
            //Record position for threefold repetition
            string posKey = getPositionKey();
            positionHistory[posKey]++;

            checkGameState();
            return true;
        }
    }
    return false;
}
void Game::undoMove()
{
    if (moveHistory.empty()) return;
    MoveRecord record = moveHistory.back();
    moveHistory.pop_back();
    Move m = record.move;
    // Get piece that is currently sitting at destination
    Piece* movedPiece = board.releasePiece(m.toRow, m.toCol);
    if (record.wasPromotion)
    {
        delete movedPiece;
        movedPiece = record.originalPawn;
    }

    board.setPiece(m.fromRow, m.fromCol, movedPiece);
    movedPiece->setRow(m.fromRow);
    movedPiece->setCol(m.fromCol);
    // Restore hasMoved flag
    if (King* k = dynamic_cast<King*>(movedPiece))
    {
        k->setHasMoved(record.pieceHadMovedBefore);
    }
    if (Rook* r = dynamic_cast<Rook*>(movedPiece))
    {
        r->setHasMoved(record.pieceHadMovedBefore);
    }
    // Restore captured piece
    if (record.wasEnPassant)
    {
        // Captured pawn back to original square
        board.setPiece(record.enPassantCapturedRow, record.enPassantCapturedCol, record.capturedPiece);
        board.setPiece(m.toRow, m.toCol, nullptr);     
    }
    else
    {
        // Normal capture (or nullptr if no capture) goes back to destination
        board.setPiece(m.toRow, m.toCol, record.capturedPiece);
    }

    // Restore game state
    currentTurn = record.colorBeforeMove;
    lastMove = record.previousLastMove;
    lastMoveWasTwoSquarePawnPush = record.previousLastMoveWasTwoSquarePawn;
}
vector<Move> Game::filterLegalMoves(Piece* piece, const vector<Move>& candidates)
{
    vector<Move> legal;
    for (const Move& m : candidates)
    {
        // Release ownership without deleting anything
        Piece* captured = board.releasePiece(m.toRow, m.toCol);
        board.releasePiece(m.fromRow, m.fromCol);

        // Simulate the move
        board.setPiece(m.toRow, m.toCol, piece);
        board.setPiece(m.fromRow, m.fromCol, nullptr);
        piece->setRow(m.toRow);
        piece->setCol(m.toCol);

        if (!isInCheck(piece->getColor()))
            legal.push_back(m);

        // Undo the move
        board.releasePiece(m.toRow, m.toCol);
        board.setPiece(m.fromRow, m.fromCol, piece);
        board.setPiece(m.toRow, m.toCol, captured);
        piece->setRow(m.fromRow);
        piece->setCol(m.fromCol);
    }
    return legal;
}
vector<Move> Game::getAllLegalMoves(Color color)
{
    vector<Move> allLegalMoves;
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            Piece* piece = board.getPiece(r, c);
            if (piece && piece->getColor() == color)
            {
                vector<Move> candidates = piece->getLegalMoves(board, r, c, lastMove, lastMoveWasTwoSquarePawnPush);
                vector<Move> legalMoves = filterLegalMoves(piece, candidates);
                allLegalMoves.insert(allLegalMoves.end(), legalMoves.begin(), legalMoves.end());
            }
        }
    }
    return allLegalMoves;
}
Piece* Game::getPieceAt(int row, int col) const
{
    return board.getPiece(row, col);
}
bool Game::canCastle(Color color, bool kingside) const
{
    // Castling logic here
    int row = (color == Color::White) ? 7 : 0;
    int kingCol = 4;
    int rookCol = kingside ? 7 : 0;
    // Check if the king and the rook involved have not moved
    King* king = dynamic_cast<King*>(board.getPiece(row, kingCol));
    Rook* rook = dynamic_cast<Rook*>(board.getPiece(row, rookCol));
    if (!king || !rook || king->getHasMoved() || rook->getHasMoved())
    {
        return false;
    }
    // Check if the squares between them are empty
    int startCol = min(kingCol, rookCol);
    int endCol = max(kingCol, rookCol);
    for (int col = startCol + 1; col < endCol; col++)
    {
        if (board.getPiece(row, col))
        {
            return false;
        }
    }
    // Check if the king is not in check, and does not pass through or end up in check
    if (isInCheck(color))
    {
        return false;
    }
    // Simulate the king moving through the squares it would pass through during castling
    for (int col = kingCol; col != rookCol; col += (kingside ? 1 : -1))
    {
        // Temporarily move the king to the next square
        // (In a full implementation, you would need to actually update the board state and check for checks)
        if (isInCheck(color))
        {
            return false;
        }
    }
    return true; // Castling is allowed
}
void Game::performCastle(Color color, bool kingside)
{
    // Perform castling move here
    int row = (color == Color::White) ? 7 : 0;
    int kingCol = 4;
    int rookCol = kingside ? 7 : 0;
    // Move King and Rook to their new positions
    int newKingCol = kingside ? 6 : 2;
    int newRookCol = kingside ? 5 : 3;
    // Move the king
    Piece* king = board.getPiece(row, kingCol);
    board.setPiece(row, newKingCol, king);
    board.setPiece(row, kingCol, nullptr);
    // Move the rook
    Piece* rook = board.getPiece(row, rookCol);
    board.setPiece(row, newRookCol, rook);
    board.setPiece(row, rookCol, nullptr);
    // Update the hasMoved flags
    dynamic_cast<King*>(king)->setHasMoved(true);
    dynamic_cast<Rook*>(rook)->setHasMoved(true);
    }
bool Game::isInCheck(Color color) const
{
    // Check detection logic here
    // This would involve checking if the king of the given color is under attack by any enemy pieces
    // Find King's position
    int kingRow = -1, kingCol = -1;
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            Piece* piece = board.getPiece(r, c);
            if (piece && piece->getColor() == color)
            {
                King* king = dynamic_cast<King*>(piece);
                if (king)
                {
                    kingRow = r;
                    kingCol = c;
                    break;
                }
            }
        }
        if (kingRow != -1) break;
    }
    // Check if any enemy piece can attack the King's square
    return isSquareAttacked(kingRow, kingCol, (color == Color::White) ? Color::Black : Color::White);
}
bool Game::isSquareAttacked(int row, int col, Color attackerColor) const
{
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            Piece* piece = board.getPiece(r, c);
            if (piece && piece->getColor() == attackerColor)
            {
                vector<Move> moves = piece->getLegalMoves(board, r, c, lastMove, lastMoveWasTwoSquarePawnPush);
                for (const Move& move : moves)
                {
                    if (move.toRow == row && move.toCol == col)
                    {
                        return true; // Square is attacked
                    }
                }
            }
        }
    }
    return false; // Placeholder implementation
};
// Stalemate: no legal moves AND not in check 
bool Game::isStalemate(Color color)
{
    if(isInCheck(color))
    {
        return false; // Not stalemate if in check
    }
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            Piece* piece = board.getPiece(r, c);
            if (piece && piece->getColor() == color)
            {
                vector<Move> moves = piece->getLegalMoves(board, r, c, lastMove, lastMoveWasTwoSquarePawnPush);
                if (!moves.empty())
                {
                    return false; // Not stalemate if there are legal moves
                }
            }
        }
    }
    return true; // Stalemate if no legal moves and not in check
}
bool Game::isThreefoldRepetition() const;
{
    string currentKey = getPositionKey();
    auto it = positionHistory.find(currentKey);
    return it != positionHistory.end() && it->second >= 3;
}
void Game::checkGameState()
{
    bool inCheck = isInCheck(currentTurn);
    bool hasMoves = !getAllLegalMoves(currentTurn).empty();
    if (inCheck && !hasMoves)
    {
        cout << (currentTurn == Color::White ? "Black" : "White") << " wins by checkmate!\n";
    }
    else if (!inCheck && !hasMoves)
    {
        cout << "Stalemate! It's a draw.\n";
    }
    else if (isThreefoldRepetition()) // Threefold repetiton check
    {
        cout << "Draw by threefold repetition!\n";
        gameOver = true;
    }
    if (isStalemate(currentTurn))
    {
        cout << "Stalemate! It's a draw." << endl;
        // Handle stalemate (e.g., end game, reset, etc.)
    }
    else if (isInCheck(currentTurn))
    {
        cout << "Check!" << endl;
        // Handle check (e.g., notify player, etc.)
    }
}
void Game::startTurnTimer()
{
    turnStartTime = chrono::steady_clock::now();
}
bool Game::checkTimeAndDeductElapsed()
{
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(now - turnStartTime)

    if (currentTurn == Color::White)
    {
        if (whiteTimeRemaining <= chrono::seconds(0))
        {
            cout << "White ran out of time! Black wins!\n";
            gameOver = true;
            return false;
        }
    }
    else
    {
        blackTImeRemaining -= elapsed;
        if (blackTimeRemaining <= chrono::seconds(0))
        {
            cout << "Black ran out of time! White wins!\n";
            gameOver = true;
            return false;
        }
    }
    return true;
}
chrono::seconds Game::getTimeRemaining(Color color) const
{
    return (color = Color::White) ? whiteTimeRemaining : blackTimeRemaining;
}
void Game::undoMove()
{
    if (moveHistory.empty()) return; // No moves to undo

    string currentKey = getPositionKey();
    positionHistory[currentKey]--;
    if (positionHistory[currentKey] <= 0)
        positionHistory.erase(currentKey);

    auto [move, capturedPiece] = moveHistory.back();
    moveHistory.pop_back();

    // Move the piece back to its original position
    Piece* piece = board.releasePiece(move.toRow, move.toCol);
    board.releasePiece(move.fromRow, move.fromCol); // Clear the from square (should be empty)  
    board.setPiece(move.fromRow, move.fromCol, piece);
    board.setPiece(lastMove.toRow, lastMove.toCol, capturedPiece); // Restore captured piece if there was one
    piece->setRow(move.fromRow);
    piece->setCol(move.fromCol);

    // Restore captured piece if there was one
    if (capturedPiece)
    {
        board.setPiece(move.toRow, move.toCol, capturedPiece);
    }

    // Switch back the turn
    currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
}
string Game::getPositionKey() const
{
    string key;
    // Encoding every square on the board
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            Piece* p = board.getPiece(r, c);
            if (p == nullptr)
            {
                key += '.';
            }
            else
            {
                char symbol = '?';
                if      (dynamic_cast<Pawn*>(p))   symbol = 'P';
                else if (dynamic_cast<Rook*>(p))   symbol = 'R';
                else if (dynamic_cast<Knight*>(p)) symbol = 'N';
                else if (dynamic_cast<Bishop*>(p)) symbol = 'B';
                else if (dynamic_cast<Queen*>(p))  symbol = 'Q';
                else if (dynamic_cast<King*>(p))   symbol = 'K';

                if (p -> getColor() = Color::Black)
                    symbol = tolower(symbol);
                key += symbol;
            }
        }
    }
    // Same pieces, different turn = different position
    key += (currentTurn == Color::White) ? 'W' : 'B';

    // Castling rights
    Piece* whiteKing = nullptr, *blackKing = nullptr;
    Piece* whiteKingsideRook = board.getPiece(7, 7);
    Piece* whiteQueensideRook = board.getPiece(7, 0);
    Piece* blackKingsideRook = board.getPiece(0, 7);
    Piece* blackQueensideRook = board.getPiece(0, 0);

    for (int c = 0; c < 8; c++)
    {
        if (King* k = dynamic_cast<King*>(board.getPiece(7, c))) whiteKing = k;
        if (King* k = dynamic_cast<King*>(board.getPiece(0, c))) blackKing = k;
    }

    auto hasMoved = [](Piece* p) -> bool 
    {
        if (King* k = dynamic_cast<King*>(p)) return k->getHasMoved();
        if (Rook* r = dynamic_cast<Rook*>(p)) return r->getHasMoved();
        return true; //treat missing pieces as "moved"
    };

    key += hasMoved(whiteKing) ? '0' : '1';
    key += hasMoved(whiteKingsideRook) ? '0' : '1';
    key += hasMoved(whiteQueensideRook) ? '0' : '1';
    key += hasMoved(blackKing) ? '0' : '1'; 
    key += hasMoved(blackKingsideRook) ? '0' : '1';
    key += hasMoved(blackQueensideRook) ? '0' : '1';

    if (lastMoveWasTwoSquarePawnPush)
    {
        key += 'E';
        key += to_string(lastMove.toCol);
    }
    return key;
}
class AI
{
    Color aiColor;
    int searchDepth;
    private:
    int minimax(Game& game, int depth, int alpha, int beta, bool maximizingPlayer)
    {
        if (depth == 0)
        {
            // Evaluate board state and return score
            return 0; // Placeholder evaluation
        }
        vector<Move> allMoves = game.getAllLegalMoves(maximizingPlayer ? aiColor : (aiColor == Color::White ? Color::Black : Color::White));
        if (maximizingPlayer)
        {
            int maxEval = INT_MIN;
            for (const Move& move : game.getAllLegalMoves(aiColor))
            {
                game.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
                int eval = minimax(game, depth - 1, alpha, beta, false);
                maxEval = max(maxEval, eval);
                // Undo the move here
                game.undoMove(move); // You would need to implement an undoMove function to revert the game state after simulating a move
                alpha = max(alpha, eval);
                if (beta <= alpha)
                {
                    break; // Alpha-Beta pruning
                }
            }
            return maxEval;
        }
        else
        {
            Color opponentColor = (aiColor == Color::White) ? Color::Black : Color::White;
            int minEval = INT_MAX;
            for (const Move& move : allMoves)
            {
                game.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
                int eval = minimax(game, depth - 1, alpha, beta, true);
                minEval = min(minEval, eval);
                // Undo the move here
                game.undoMove(move); // You would need to implement an undoMove function to revert the game state after simulating a move
                beta = min(beta, eval);
                if (beta <= alpha)
                {
                    break; // Alpha-Beta pruning
                }
            }
            return minEval;
        }
    }
    int evaluateBoard(const Game& game)
    {
        // Implement a heuristic evaluation function to score the board state
        // Piece values
        const int pawnValue = 10;
        const int knightValue = 30;
        const int bishopValue = 30;
        const int rookValue = 50;
        const int queenValue = 90;
        const int kingValue = 2000; // High value to prioritize king safety

        int score = 0;
        for (int r = 0; r < 8; r++)
        {
            for (int c = 0; c < 8; c++)
            {
                Piece* piece = game.getPiece(r, c);
                if (piece)
                {
                    switch (piece->getType())
                    {
                        case PieceType::Pawn:
                            score += piece->getColor() == aiColor ? pawnValue : -pawnValue;
                            break;
                        case PieceType::Knight:
                            score += piece->getColor() == aiColor ? knightValue : -knightValue;
                            break;
                        case PieceType::Bishop:
                            score += piece->getColor() == aiColor ? bishopValue : -bishopValue;
                            break;
                        case PieceType::Rook:
                            score += piece->getColor() == aiColor ? rookValue : -rookValue;
                            break;
                        case PieceType::Queen:
                            score += piece->getColor() == aiColor ? queenValue : -queenValue;
                            break;
                        case PieceType::King:
                            score += piece->getColor() == aiColor ? kingValue : -kingValue;
                            break;
                    }
                }
            }
        }
        return score;
    }
    public:
    AI(Color color, int depth) : aiColor(color), searchDepth(depth) {}
    Move getBestMove(const Game& game)
    {
        vector<Move> allMoves = game.getAllLegalMoves(aiColor);
        Move bestMove = allMoves[0];
        int bestScore = INT_MIN;
        for (const Move& move : allMoves)
        {
            game.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol);
            int score = minimax(game, searchDepth - 1, false);
            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }
        }
        return bestMove;
    }
};
int main()
{
    cout << "Welcome to Chess!\n";
    cout << "Play against: Human or CPU\n";
    int choice;
    cin >> choice;
    Game game;
    game.printBoard();
    AI* ai = nullptr;
    if (choice == "CPU")
    {
       cout << "Color: White or Black\n";
       int color_choice;
       cin >> color_choice;
       if (color_choice == "White")
       {
            ai = new AI(Color::Black, 4); // looks 4 moves ahead
       }
       else if (color_choice == "Black")
       {
            ai = new AI (Color::White, 4); // looks 4 moves ahead
       }
    }
    while (!game.isGameOver) // using getter
    {
        // Human turn
        string input, toSquare;
        cout << "\nEnter move, or 'quit' to end match: ";
        
        game.startTurnTimer(); // starts timing
        
        cin >> input;
        if (input == "quit") break;
        cin >> toSquare;

        if (!game.checkTimeAndDeductElapsed())
            break;

        int fromCol = input[0] - 'a';
        int fromRow = 8 - (input[1] - '0');
        int toCol = toSquare[0] - 'a';
        int toRow = 8 - (toSquare[1] - '0');

        if (!game.movePiece(fromRow, fromCol, toRow, toCol))
        {
            cout << "Invalid move, try again.\n";
            continue;
        }
        game.printBoard();

        if (game.isGameOver()) break;
        // CPU turn
        if (ai != nullptr)
        {
            game.startTurnTimer();
            cout << "CPU is thinking...\n";
            Move aiMove = ai->getBestMove(game);
            game.movePiece(aiMove.fromRow, aiMove.fromCol, aiMove.toRow, aiMove.toCol);
            game.printBoard();
        }
    }

    // Example game loop
    while (!game.isGameOver()) // using getter
    {
        string input;
        cout << "\nEnter move (e.g. e2 e4), or 'quit' to exit: ";
        cin >> input;
        if (input == "quit") break;

        string toSquare;
        cin >> toSquare;

        // Convert chess notation to row/col
        // e.g. "e2" -> col = 4, row = 6
        int fromCol = input[0] - 'a';
        int fromRow = 8 - (input[1] - '0');
        int toCol   = toSquare[0] - 'a';
        int toRow   = 8 - (toSquare[1] - '0');

        if (game.movePiece(fromRow, fromCol, toRow, toCol))
        {
            game.printBoard();
        }
        else
        {
            cout << "Invalid move, try again." << endl;
        }
    }
    delete ai;
    return 0;
}