#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include "Board.h"
#include "GameLogic.h"
#include "Move.h"
#include "Bot.h"
#include "GameHistory.h"
#include "Analysis.h"

const int WINDOW_SIZE = 800;
const int BOARD_SIZE  = 8;
const int BOARD_PX    = WINDOW_SIZE * 0.8f;
const int TILE_SIZE   = BOARD_PX / BOARD_SIZE;
const int OFFSET      = (WINDOW_SIZE - BOARD_PX) / 2;

bool vsBot     = false;
Color botColor = Color::Black;
const int BOT_DEPTH = 3;

GameHistory gameHistory;
bool gameOver        = false;
bool showingAnalysis = false;
bool analyzingNow    = false;
std::vector<MoveAnalysis> analysisResults;
int analysisIndex = 0;

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Chess");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

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

    auto drawText = [&](const std::string& str, int size,
                        float x, float y, sf::Color color) {
        sf::Text t;
        t.setFont(font);
        t.setString(str);
        t.setCharacterSize(size);
        t.setFillColor(color);
        t.setPosition(x, y);
        window.draw(t);
    };

    // ── Ecran de start ───────────────────────────────────────────
    bool gameStarted = false;
    while (window.isOpen() && !gameStarted)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Num1)
                { vsBot = false; gameStarted = true; }
                if (event.key.code == sf::Keyboard::Num2)
                { vsBot = true;  gameStarted = true; }
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        window.clear(bgColor);

        if (fontLoaded)
        {
            auto makeText = [&](const std::string& str, int size, float y) {
                sf::Text t;
                t.setFont(font);
                t.setString(str);
                t.setCharacterSize(size);
                t.setFillColor(sf::Color(240, 217, 160));
                sf::FloatRect bounds = t.getLocalBounds();
                t.setPosition((WINDOW_SIZE - bounds.width) / 2.f, y);
                return t;
            };
            window.draw(makeText("Chess", 52, 180));
            window.draw(makeText("1  -  Doi jucatori", 28, 320));
            window.draw(makeText("2  -  Contra bot", 28, 370));
            window.draw(makeText("Esc  -  Iesire", 20, 500));
        }

        window.display();
    }

    // ── Stare joc ────────────────────────────────────────────────
    Board board;
    bool hasPieceSelected = false;
    int  selectedRow = -1, selectedCol = -1;
    std::vector<Move> legalMoves;

    bool awaitingPromotion = false;
    int  promotionRow = -1, promotionCol = -1;
    bool promotionIsWhite = true;
    std::string promotionChoices[4];

    bool botThinking = false;

    // ── Game loop ────────────────────────────────────────────────
    while (window.isOpen())
    {
        // ── Mutarea botului ──────────────────────────────────────
        if (vsBot && !gameOver && !awaitingPromotion &&
            board.currentTurn == botColor && !botThinking)
        {
            botThinking = true;
            Board boardSnapshotBot = board;
            Move botMove = Bot::getBestMove(board, botColor, BOT_DEPTH);
            board.applyMove(botMove);
            gameHistory.record(botMove, boardSnapshotBot, botColor);

            char landed = board.grid[botMove.toRow][botMove.toCol];
            if ((landed == 'p' || landed == 'P') &&
                (botMove.toRow == 0 || botMove.toRow == 7))
            {
                board.grid[botMove.toRow][botMove.toCol] =
                    (botColor == Color::White) ? 'Q' : 'q';
            }
            botThinking = false;

            if (!gameOver &&
                (GameLogic::isCheckmate(board, board.currentTurn) ||
                 GameLogic::isStalemate(board, board.currentTurn)))
            {
                gameOver     = true;
                analyzingNow = true;
            }
        }

        // ── Events ──────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (showingAnalysis)
                {
                    if (event.key.code == sf::Keyboard::D &&
                        analysisIndex < (int)analysisResults.size() - 1)
                        analysisIndex++;
                    if (event.key.code == sf::Keyboard::A && analysisIndex > 0)
                        analysisIndex--;
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                    continue;
                }

                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }

            if (showingAnalysis || gameOver) continue;
            if (vsBot && board.currentTurn == botColor) continue;

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                if (awaitingPromotion)
                {
                    int totalW = 4 * TILE_SIZE;
                    int startX = (WINDOW_SIZE - totalW) / 2;
                    int uiY    = (WINDOW_SIZE - TILE_SIZE) / 2;

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
                    continue;
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
                                Color movingColor   = board.currentTurn;
                                Board boardSnapshot = board;

                                board.applyMove(m);
                                gameHistory.record(m, boardSnapshot, movingColor);
                                moved = true;

                                char landed     = board.grid[m.toRow][m.toCol];
                                bool isPawn     = (landed == 'P' || landed == 'p');
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

                                if (!awaitingPromotion &&
                                    (GameLogic::isCheckmate(board, board.currentTurn) ||
                                     GameLogic::isStalemate(board, board.currentTurn)))
                                {
                                    gameOver     = true;
                                    analyzingNow = true;
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

        // ── Analiza ──────────────────────────────────────────────
        if (analyzingNow)
        {
            window.clear(bgColor);
            if (fontLoaded)
                drawText("Se analizeaza partida...", 28,
                         220, 370, sf::Color(240, 217, 160));
            window.display();

            Color humanColor = (vsBot) ? Color::White : Color::None;
            analysisResults  = Analysis::analyze(gameHistory, humanColor, BOT_DEPTH);
            analysisIndex    = 0;
            analyzingNow     = false;
            showingAnalysis  = true;
        }

        // ── Gaseste regele in sah ────────────────────────────────
        int checkKingRow = -1, checkKingCol = -1;
        if (!gameOver && GameLogic::isInCheck(board, board.currentTurn))
        {
            char kingPiece = (board.currentTurn == Color::White) ? 'K' : 'k';
            for (int r = 0; r < 8; r++)
                for (int c = 0; c < 8; c++)
                    if (board.grid[r][c] == kingPiece)
                    { checkKingRow = r; checkKingCol = c; }
        }

        window.clear(bgColor);

        // ── Randeaza tabla + piese ───────────────────────────────
        if (!showingAnalysis)
        {
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
                    sf::RectangleShape cell(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                    cell.setPosition(startX + i * TILE_SIZE, uiY);
                    cell.setFillColor(i % 2 == 0 ? lightColor : darkColor);
                    window.draw(cell);

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

            if (vsBot && fontLoaded && board.currentTurn == botColor)
                drawText("Bot gandeste...", 18, 10, 10,
                         sf::Color(240, 217, 160, 200));
        }

        // ── Ecran analiza ────────────────────────────────────────
        if (showingAnalysis && fontLoaded)
        {
            int boardSize = 480;
            int tileSize  = boardSize / 8;
            int boardOffX = 20;
            int boardOffY = 60;
            int panelX    = boardOffX + boardSize + 20;

            drawText("Analiza partidei  (" +
                std::to_string(analysisIndex + 1) + "/" +
                std::to_string(analysisResults.size()) + ")",
                20, 20, 20, sf::Color(240, 217, 160));

            if (analysisResults.empty())
            {
                drawText("Nicio eroare gasita!", 28,
                         200, 360, sf::Color(100, 220, 100));
            }
            else
            {
                const MoveAnalysis& ma = analysisResults[analysisIndex];

                // Tabla mica
                sf::RectangleShape sq(sf::Vector2f(tileSize, tileSize));
                for (int r = 0; r < 8; r++) {
                    for (int c = 0; c < 8; c++) {
                        bool light = (r + c) % 2 == 0;
                        sq.setFillColor(light ? lightColor : darkColor);
                        sq.setPosition(boardOffX + c * tileSize, boardOffY + r * tileSize);
                        window.draw(sq);

                        if ((r == ma.playedMove.fromRow && c == ma.playedMove.fromCol) ||
                            (r == ma.playedMove.toRow   && c == ma.playedMove.toCol))
                        {
                            sf::Color hlColor;
                            switch (ma.quality) {
                                case MoveQuality::Blunder:
                                    hlColor = sf::Color(220, 50,  50,  180); break;
                                case MoveQuality::Mistake:
                                    hlColor = sf::Color(220, 140, 50,  180); break;
                                case MoveQuality::Inaccuracy:
                                    hlColor = sf::Color(220, 220, 50,  180); break;
                                default:
                                    hlColor = sf::Color(100, 220, 100, 180); break;
                            }
                            sq.setFillColor(hlColor);
                            window.draw(sq);
                        }

                        if (ma.quality != MoveQuality::Good &&
                            r == ma.bestMove.toRow && c == ma.bestMove.toCol &&
                            !(ma.bestMove.toRow == ma.playedMove.toRow &&
                              ma.bestMove.toCol == ma.playedMove.toCol))
                        {
                            sq.setFillColor(sf::Color(100, 200, 100, 120));
                            window.draw(sq);
                        }

                        char piece = ma.boardBefore.grid[r][c];
                        if (piece != ' ' && textures.count(piece))
                        {
                            sf::Sprite sprite(textures[piece]);
                            auto texSize = textures[piece].getSize();
                            float scale  = (float)tileSize * 0.8f /
                                           std::max(texSize.x, texSize.y);
                            sprite.setScale(scale, scale);
                            float padding = (tileSize - texSize.x * scale) / 2.f;
                            sprite.setPosition(
                                boardOffX + c * tileSize + padding,
                                boardOffY + r * tileSize + padding);
                            window.draw(sprite);
                        }
                    }
                }

                // Panel dreapta
                std::string qualityStr;
                sf::Color   qualityColor;
                switch (ma.quality) {
                    case MoveQuality::Blunder:
                        qualityStr = "Blunder ??";
                        qualityColor = sf::Color(220, 50, 50); break;
                    case MoveQuality::Mistake:
                        qualityStr = "Mistake ?";
                        qualityColor = sf::Color(220, 140, 50); break;
                    case MoveQuality::Inaccuracy:
                        qualityStr = "Inaccuracy !?";
                        qualityColor = sf::Color(220, 220, 50); break;
                    default:
                        qualityStr = "Good";
                        qualityColor = sf::Color(100, 220, 100); break;
                }

                drawText(qualityStr, 24, panelX, boardOffY, qualityColor);
                drawText("Jucat: " + ma.notation, 16, panelX, boardOffY + 40,
                         sf::Color(240, 217, 160));

                if (ma.quality != MoveQuality::Good)
                {
                    drawText("Mai bine: " + Analysis::toNotationPublic(ma.bestMove),
                             16, panelX, boardOffY + 64, sf::Color(100, 220, 100));
                    drawText("Eval inainte: " + std::to_string(ma.evalBefore),
                             14, panelX, boardOffY + 104, sf::Color(180, 180, 180));
                    drawText("Eval dupa:    " + std::to_string(ma.evalAfter),
                             14, panelX, boardOffY + 124, sf::Color(180, 180, 180));
                    drawText("Pierdere:     " + std::to_string(ma.loss) + " pts",
                             14, panelX, boardOffY + 144, sf::Color(180, 180, 180));
                }

                // Lista erori
                drawText("Erori:", 14, panelX, boardOffY + 184,
                         sf::Color(160, 160, 160));
                for (int i = 0; i < (int)analysisResults.size() && i < 12; i++)
                {
                    const auto& a = analysisResults[i];
                    std::string qualStr;
                    sf::Color   col;
                    switch (a.quality) {
                        case MoveQuality::Blunder:
                            qualStr = "??"; col = sf::Color(220, 50,  50);  break;
                        case MoveQuality::Mistake:
                            qualStr = "?";  col = sf::Color(220, 140, 50);  break;
                        case MoveQuality::Inaccuracy:
                            qualStr = "!?"; col = sf::Color(220, 220, 50);  break;
                        default:
                            qualStr = ".";  col = sf::Color(100, 220, 100); break;
                    }

                    sf::Color rowColor = (i == analysisIndex)
                        ? sf::Color(255, 255, 255) : col;

                    drawText(std::to_string(i + 1) + ". " + a.notation + " " + qualStr,
                             13, panelX, boardOffY + 204 + i * 20, rowColor);
                }

                drawText("A / D  —  navigare", 13, panelX, 700, sf::Color(120, 120, 120));
                drawText("Esc    —  inchide",  13, panelX, 718, sf::Color(120, 120, 120));
            }
        }

        window.display();
    }

    return 0;
}