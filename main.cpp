#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <algorithm>

const int WINDOW_SIZE = 800;
const int BOARD_SIZE  = 8;
const int BOARD_PX    = WINDOW_SIZE * 0.8f;
const int TILE_SIZE   = BOARD_PX / BOARD_SIZE;
const int OFFSET      = (WINDOW_SIZE - BOARD_PX) / 2;

char board[8][8] = {
    {'r','n','b','q','k','b','n','r'},
    {'p','p','p','p','p','p','p','p'},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {'P','P','P','P','P','P','P','P'},
    {'R','N','B','Q','K','B','N','R'}
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tabla de Sah");
    window.setFramerateLimit(60);

    std::map<char, sf::Texture> textures;
    std::map<char, std::string> filenames = {
        {'K', "w_king_1x_ns.png"},
        {'Q', "w_queen_1x_ns.png"},
        {'R', "w_rook_1x_ns.png"},
        {'B', "w_bishop_1x_ns.png"},
        {'N', "w_knight_1x_ns.png"},
        {'P', "w_pawn_1x_ns.png"},
        {'k', "b_king_1x_ns.png"},
        {'q', "b_queen_1x_ns.png"},
        {'r', "b_rook_1x_ns.png"},
        {'b', "b_bishop_1x_ns.png"},
        {'n', "b_knight_1x_ns.png"},
        {'p', "b_pawn_1x_ns.png"},
    };

    for (auto& [ch, filename] : filenames)
        textures[ch].loadFromFile("sprites/PNGs/No shadow/1x/" + filename);

    sf::Color lightColor(240, 217, 160);
    sf::Color darkColor(139, 69, 19);
    sf::Color bgColor(30, 20, 10);

    sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));

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
        }

        window.clear(bgColor);

        for (int row = 0; row < BOARD_SIZE; ++row)
        {
            for (int col = 0; col < BOARD_SIZE; ++col)
            {
                bool isLight = (row + col) % 2 == 0;
                square.setFillColor(isLight ? lightColor : darkColor);
                square.setPosition(OFFSET + col * TILE_SIZE, OFFSET + row * TILE_SIZE);
                window.draw(square);

                char piece = board[row][col];
                if (piece != ' ' && textures.count(piece))
                {
                    sf::Sprite sprite(textures[piece]);
                    auto texSize = textures[piece].getSize();

                    float scale = (float)TILE_SIZE * 0.8f / std::max(texSize.x, texSize.y);
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