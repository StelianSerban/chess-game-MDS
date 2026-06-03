#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include "Board.h"
#include "GameLogic.h"
#include "Move.h"

const int WINDOW_SIZE = 800;
const int BOARD_SIZE  = 8;
const int BOARD_PX    = WINDOW_SIZE * 0.8f;
const int TILE_SIZE   = BOARD_PX / BOARD_SIZE;
const int OFFSET      = (WINDOW_SIZE - BOARD_PX) / 2;

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tabla de Sah");
    window.setFramerateLimit(60);

    // Incarca texturile
    std::map<char, sf::Texture> textures;
    std::map<char, std::string> filenames = {
        {'K', "w_king_1x_ns.png"},   {'Q', "w_queen_1x_ns.png"},
        {'R', "w_rook_1x_ns.png"},   {'B', "w_bishop_1x_ns.png"},
        {'N', "w_knight_1x_ns.png"}, {'P', "w_pawn_1x_ns.png"},
        {'k', "b_king_1x_ns.png"},   {'q', "b_queen_1x_ns.png"},
        {'r', "b_rook_1x_ns.png"},   {'b', "b_bishop_1x_ns.png"},
        {'n', "b_knight_1x_ns.png"}, {'p', "b_pawn_1x_ns.png"},
    };
    for (auto& [ch, filename] : filenames)
        textures[ch].loadFromFile("sprites/PNGs/No shadow/1x/" + filename);

    // Culori tabla
    sf::Color lightColor(240, 217, 160);
    sf::Color darkColor(139, 69, 19);
    sf::Color bgColor(30, 20, 10);
    sf::Color selectedColor(100, 180, 100, 180);   // verde pentru piesa selectata
    sf::Color legalColor(180, 220, 100, 140);       // verde deschis pentru mutari legale
    sf::Color checkColor(220, 50, 50, 180);         // rosu pentru rege in sah

    sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));

    Board board;

    // Stare selectie
    bool hasPieceSelected = false;
    int  selectedRow = -1, selectedCol = -1;
    std::vector<Move> legalMoves;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                // Verifica daca click-ul e pe tabla
                if (mx >= OFFSET && mx < OFFSET + BOARD_PX &&
                    my >= OFFSET && my < OFFSET + BOARD_PX)
                {
                    int clickCol = (mx - OFFSET) / TILE_SIZE;
                    int clickRow = (my - OFFSET) / TILE_SIZE;

                    if (!hasPieceSelected)
                    {
                        // Selecteaza piesa daca e randul jucatorului curent
                        char piece = board.grid[clickRow][clickCol];
                        if (Board::pieceColor(piece) == board.currentTurn)
                        {
                            auto moves = GameLogic::getLegalMoves(board, clickRow, clickCol);
                            if (!moves.empty())
                            {
                                hasPieceSelected = true;
                                selectedRow      = clickRow;
                                selectedCol      = clickCol;
                                legalMoves       = moves;
                            }
                        }
                    }
                    else
                    {
                        // Cauta daca click-ul e pe o mutare legala
                        bool moved = false;
                        for (auto& m : legalMoves)
                        {
                            if (m.toRow == clickRow && m.toCol == clickCol)
                            {
                                board.applyMove(m);
                                moved = true;
                                break;
                            }
                        }

                        // Daca nu am mutat, incearca sa selectezi alta piesa
                        if (!moved)
                        {
                            char piece = board.grid[clickRow][clickCol];
                            if (Board::pieceColor(piece) == board.currentTurn)
                            {
                                auto moves = GameLogic::getLegalMoves(board, clickRow, clickCol);
                                if (!moves.empty())
                                {
                                    selectedRow = clickRow;
                                    selectedCol = clickCol;
                                    legalMoves  = moves;
                                    hasPieceSelected = true;
                                }
                                else
                                {
                                    hasPieceSelected = false;
                                    legalMoves.clear();
                                }
                            }
                            else
                            {
                                hasPieceSelected = false;
                                legalMoves.clear();
                            }
                        }
                        else
                        {
                            hasPieceSelected = false;
                            legalMoves.clear();
                        }
                    }

                    // Verifica checkmate / stalemate dupa fiecare mutare
                    if (!hasPieceSelected)
                    {
                        if (GameLogic::isCheckmate(board, board.currentTurn))
                        {
                            // TODO: afiseaza mesaj de mat
                        }
                        else if (GameLogic::isStalemate(board, board.currentTurn))
                        {
                            // TODO: afiseaza mesaj de pat
                        }
                    }
                }
            }
        }

        // Gaseste regele in sah pentru highlight
        int checkKingRow = -1, checkKingCol = -1;
        if (GameLogic::isInCheck(board, board.currentTurn))
        {
            char kingPiece = (board.currentTurn == Color::White) ? 'K' : 'k';
            for (int r = 0; r < 8; r++)
                for (int c = 0; c < 8; c++)
                    if (board.grid[r][c] == kingPiece)
                    { checkKingRow = r; checkKingCol = c; }
        }

        window.clear(bgColor);

        for (int row = 0; row < BOARD_SIZE; ++row)
        {
            for (int col = 0; col < BOARD_SIZE; ++col)
            {
                // Culoarea de baza a patratului
                bool isLight = (row + col) % 2 == 0;
                square.setFillColor(isLight ? lightColor : darkColor);
                square.setPosition(OFFSET + col * TILE_SIZE, OFFSET + row * TILE_SIZE);
                window.draw(square);

                // Highlight rege in sah
                if (row == checkKingRow && col == checkKingCol)
                {
                    square.setFillColor(checkColor);
                    window.draw(square);
                }

                // Highlight piesa selectata
                if (hasPieceSelected && row == selectedRow && col == selectedCol)
                {
                    square.setFillColor(selectedColor);
                    window.draw(square);
                }

                // Highlight mutari legale
                if (hasPieceSelected)
                {
                    for (auto& m : legalMoves)
                    {
                        if (m.toRow == row && m.toCol == col)
                        {
                            square.setFillColor(legalColor);
                            window.draw(square);
                            break;
                        }
                    }
                }

                // Randeaza piesa
                char piece = board.grid[row][col];
                if (piece != ' ' && textures.count(piece))
                {
                    sf::Sprite sprite(textures[piece]);
                    auto texSize = textures[piece].getSize();
                    float scale  = (float)TILE_SIZE * 0.8f / std::max(texSize.x, texSize.y);
                    sprite.setScale(scale, scale);
                    float padding = (TILE_SIZE - texSize.x * scale) / 2.f;
                    sprite.setPosition(
                        OFFSET + col * TILE_SIZE + padding,
                        OFFSET + row * TILE_SIZE + padding
                    );
                    window.draw(sprite);
                }
            }
        }

        window.display();
    }

    return 0;
}