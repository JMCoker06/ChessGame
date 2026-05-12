#include <iostream>
#include <vector>
#include <string>
#include <memory> // for unique_ptr
#include <optional> // represents "no move"
#include <stdexcept> // for bounds checking exceptions
using namespace std;
enum class Color {White, Black};
struct Move //allows the player to be able to move their pieces in-game
{
    int fromRow, fromCol;
    int toRow, toCol;
};
//Defining Square class 
class Square
{
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
    Piece* Board::getPiece(int row, int col) const;
    void Board::setPiece(int row, int col, Piece* piece);
    void print() const;
};
//Defining Piece class 
class Piece
{
    private:
    Color color;
    public:
    Piece(Color color) : color(color) {}
    Color getColor() const { return color; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const = 0;
    virtual ~Piece() = default;
};
//Derived classes that define specific piece types
class Pawn : public Piece
{
    public:
    Pawn(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
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
        // int enPassantRow = (getColor() == Color::White) ? 3 : 4;
        // if (r == enPassantRow && lastMove was a two-square pawn push)
        // {
        //     if (lastMove.toCol == c - 1 || lastMove.toCol == c + 1)
        //     {
        //         moves.push_back({r, c, r + direction, lastMove.toCol});
        //     }
        // }
        return moves;
    }
};
class Bishop : public Piece
{
    public:
    Bishop(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
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
            }
        }
        return moves;
    }
};
class Knight : public Piece
{
    public:
    Knight(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
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
    public:
    Rook(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
    {
        vector<Move> moves;
        // Rook move logic here
        int r = getRow();
        int c = getCol();   
        // 4 cardinal directions: (row offset, col offset)
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
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
            }
        }
        return moves;
    }
};
class Queen : public Piece
{
    public:
    Queen(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
    {
        vector<Move> moves;
        // Queen move logic here
        // Combine the move logic of the Rook and the Bishop
        int r = getRow();
        int c = getCol();
        // 4 cardinal directions: (row offset, col offset)
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
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
            }
        }
        // 4 diagonal directions: (row offset, col offset)
        int diagonalDirections[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
        for (const auto& dir : diagonalDirections)
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
            }
        }
        return moves;
    }
};
class King : public Piece
{
    public:
    King(int r, int c, Color color) : Piece(r, c, color) {}
    virtual vector<Move> getLegalMoves(const Board& board, int row, int col) const override
    {
        vector<Move> moves;
        // King move logic here
        int r = getRow();
        int c = getCol();
        // 8 possible directions: (row offset, col offset)
        int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
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
class Game
{
    Board board;    // using the Board class
    Color currentTurn;
    Move lastMove;
    bool lastMovewasTwoSquarePawnPush;
};

Piece *Board::getPiece(int row, int col) const
{
    return nullptr;
}
