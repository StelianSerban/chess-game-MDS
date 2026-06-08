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

    sf::Color lightColor(240, 217, 160);
    sf::Color darkColor(139, 69, 19);
    sf::Color bgColor(30, 20, 10);
    sf::Color selectedColor(100, 180, 100, 180);
    sf::Color legalColor(180, 220, 100, 140);
    sf::Color checkColor(220, 50, 50, 180);
    sf::Color promotionBg(30, 30, 30, 220);

    sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));

    Board board;

    bool hasPieceSelected = false;
    int  selectedRow = -1, selectedCol = -1;
    std::vector<Move> legalMoves;

    // Stare promovare
    bool awaitingPromotion = false;
    int  promotionRow = -1, promotionCol = -1;
    bool promotionIsWhite = true;
    // Piesele din care alege (in ordine: Regina, Turn, Nebun, Cal)
    std::string promotionChoices[4];

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

                // Daca asteptam promovare, trateaza click-ul pe UI
                if (awaitingPromotion)
                {
                    // UI-ul are 4 patrate centrate orizontal
                    int totalW   = 4 * TILE_SIZE;
                    int startX   = (WINDOW_SIZE - totalW) / 2;
                    int uiY      = (WINDOW_SIZE - TILE_SIZE) / 2;

                    for (int i = 0; i < 4; i++)
                    {
                        int px = startX + i * TILE_SIZE;
                        if (mx >= px && mx < px + TILE_SIZE &&
                            my >= uiY && my < uiY + TILE_SIZE)
                        {
                            board.grid[promotionRow][promotionCol] =
                                promotionChoices[i][0];
                            awaitingPromotion = false;
                            break;
                        }
                    }
                    continue; // Nu procesa alt input cat timp e promovare
                }

                if (mx >= OFFSET && mx < OFFSET + BOARD_PX &&
                    my >= OFFSET && my < OFFSET + BOARD_PX)
                {
                    int clickCol = (mx - OFFSET) / TILE_SIZE;
                    int clickRow = (my - OFFSET) / TILE_SIZE;

                    if (!hasPieceSelected)
                    {
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
                        bool moved = false;
                        for (auto& m : legalMoves)
                        {
                            if (m.toRow == clickRow && m.toCol == clickCol)
                            {
                                board.applyMove(m);
                                moved = true;

                                // Verifica promovare
                                char landed = board.grid[m.toRow][m.toCol];
                                bool isPawn = (landed == 'P' || landed == 'p');
                                bool isPromoRow = (m.toRow == 0 || m.toRow == 7);

                                if (isPawn && isPromoRow)
                                {
                                    awaitingPromotion = true;
                                    promotionRow      = m.toRow;
                                    promotionCol      = m.toCol;
                                    promotionIsWhite  = (landed == 'P');

                                    if (promotionIsWhite) {
                                        promotionChoices[0] = "Q";
                                        promotionChoices[1] = "R";
                                        promotionChoices[2] = "B";
                                        promotionChoices[3] = "N";
                                    } else {
                                        promotionChoices[0] = "q";
                                        promotionChoices[1] = "r";
                                        promotionChoices[2] = "b";
                                        promotionChoices[3] = "n";
                                    }
                                }
                                break;
                            }
                        }

                        if (!moved)
                        {
                            char piece = board.grid[clickRow][clickCol];
                            if (Board::pieceColor(piece) == board.currentTurn)
                            {
                                auto moves = GameLogic::getLegalMoves(board, clickRow, clickCol);
                                if (!moves.empty())
                                {
                                    selectedRow      = clickRow;
                                    selectedCol      = clickCol;
                                    legalMoves       = moves;
                                    hasPieceSelected = true;
                                }
                                else { hasPieceSelected = false; legalMoves.clear(); }
                            }
                            else { hasPieceSelected = false; legalMoves.clear(); }
                        }
                        else { hasPieceSelected = false; legalMoves.clear(); }
                    }
                }
            }
        }

        // Gaseste regele in sah
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

        // Randeaza tabla + piese
        for (int row = 0; row < BOARD_SIZE; ++row)
        {
            for (int col = 0; col < BOARD_SIZE; ++col)
            {
                bool isLight = (row + col) % 2 == 0;
                square.setFillColor(isLight ? lightColor : darkColor);
                square.setPosition(OFFSET + col * TILE_SIZE, OFFSET + row * TILE_SIZE);
                window.draw(square);

                if (row == checkKingRow && col == checkKingCol)
                { square.setFillColor(checkColor); window.draw(square); }

                if (hasPieceSelected && row == selectedRow && col == selectedCol)
                { square.setFillColor(selectedColor); window.draw(square); }

                if (hasPieceSelected)
                    for (auto& m : legalMoves)
                        if (m.toRow == row && m.toCol == col)
                        { square.setFillColor(legalColor); window.draw(square); break; }

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
                        OFFSET + row * TILE_SIZE + padding);
                    window.draw(sprite);
                }
            }
        }

        // UI promovare — overlay semi-transparent + 4 piese
        if (awaitingPromotion)
        {
            sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
            overlay.setFillColor(promotionBg);
            window.draw(overlay);

            int totalW = 4 * TILE_SIZE;
            int startX = (WINDOW_SIZE - totalW) / 2;
            int uiY    = (WINDOW_SIZE - TILE_SIZE) / 2;

            for (int i = 0; i < 4; i++)
            {
                // Fundal patrat
                sf::RectangleShape cell(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                cell.setPosition(startX + i * TILE_SIZE, uiY);
                cell.setFillColor(i % 2 == 0 ? lightColor : darkColor);
                window.draw(cell);

                // Piesa
                char ch = promotionChoices[i][0];
                if (textures.count(ch))
                {
                    sf::Sprite sprite(textures[ch]);
                    auto texSize = textures[ch].getSize();
                    float scale  = (float)TILE_SIZE * 0.8f / std::max(texSize.x, texSize.y);
                    sprite.setScale(scale, scale);
                    float padding = (TILE_SIZE - texSize.x * scale) / 2.f;
                    sprite.setPosition(startX + i * TILE_SIZE + padding, uiY + padding);
                    window.draw(sprite);
                }
            }
        }

        window.display();
    }

    return 0;
}