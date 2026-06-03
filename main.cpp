#include <SFML/Graphics.hpp>

// Majuscule = alb, minuscule = negru
// P=pion, R=turn, N=cal, B=nebun, Q=regina, K=rege
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
    const int WINDOW_SIZE = 800;
    const int BOARD_SIZE  = 8;

    // Tabla ocupa 80% din fereastra
    const int BOARD_PX   = WINDOW_SIZE * 0.8f;
    const int TILE_SIZE  = BOARD_PX / BOARD_SIZE;  // 80px per patrat
    const int OFFSET     = (WINDOW_SIZE - BOARD_PX) / 2; // centrat

    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tabla de Sah");
    window.setFramerateLimit(60);

    sf::Color lightColor(240, 217, 160);
    sf::Color darkColor(139, 69, 19);
    sf::Color bgColor(30, 20, 10); // fundal inchis

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
            }
        }

        window.display();
    }

    return 0;
}