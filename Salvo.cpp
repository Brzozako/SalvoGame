#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

enum class CellStatus {
    Empty, Ship, Hit, Miss
};

struct Cell {
    sf::RectangleShape shape;
    CellStatus status;
};

struct Ship {
    int length;
    int x, y; // Pocz�tkowe wsp�rz�dne
    bool horizontal; // Czy statek jest umieszczony poziomo
};

class Board {
private:
    std::vector<std::vector<Cell>> cells;
    std::vector<Ship> ships;
    int cellSize;
    int boardSize;
public:
    Board(int boardSize, int cellSize, std::vector<int> shipSizes);
    void draw(sf::RenderWindow& window, int offsetX, int offsetY);
    void saveToFile(const std::string& filename);
    CellStatus getCellStatus(int x, int y) const { return cells[x][y].status; }
    CellStatus shoot(int x, int y);
    int getBoardSize() const { return cells.size(); }
    int getCellSize() const { return cellSize; }
    bool canPlaceShip(int x, int y, int shipSize, int direction);
    void placeShip(int x, int y, int shipSize, int direction);
    void placeShipsRandomly();
    bool allShipsSunk() const;
};

CellStatus Board::shoot(int x, int y) {
    if (cells[x][y].status == CellStatus::Ship) {
        cells[x][y].status = CellStatus::Hit;
        return CellStatus::Hit;
    }
    else if (cells[x][y].status == CellStatus::Empty) {
        cells[x][y].status = CellStatus::Miss;
        return CellStatus::Miss;
    }
    else {
        return cells[x][y].status;
    }
}


Board::Board(int size, int cellSize, const std::vector<int> shipSizes) : cells(size, std::vector<Cell>(size)), cellSize(cellSize), boardSize(size) {
    for (int shipSize : shipSizes) {
        ships.push_back({ shipSize, 0, 0, false });
    }
    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            cells[i][j].shape.setSize(sf::Vector2f(cellSize, cellSize));
            cells[i][j].shape.setPosition(i * cellSize, j * cellSize);
            cells[i][j].status = CellStatus::Empty;
        }
    }
    placeShipsRandomly();
}

bool Board::canPlaceShip(int x, int y, int shipSize, int direction) {
    if (direction == 0) { // Horizontal
        for (int i = 0; i < shipSize; ++i) {
            if (x + i >= cells.size() || cells[x + i][y].status != CellStatus::Empty) {
                return false;
            }
        }
    }
    else { // Vertical
        for (int i = 0; i < shipSize; ++i) {
            if (y + i >= cells.size() || cells[x][y + i].status != CellStatus::Empty) {
                return false;
            }
        }
    }
    return true;
}

void Board::placeShip(int x, int y, int shipSize, int direction) {
    if (direction == 0) { // Horizontal
        for (int i = 0; i < shipSize; ++i) {
            cells[x + i][y].status = CellStatus::Ship;
        }
    }
    else { // Vertical
        for (int i = 0; i < shipSize; ++i) {
            cells[x][y + i].status = CellStatus::Ship;
        }
    }
}

void Board::placeShipsRandomly() {
    for (auto& ship : ships) {
        while (true) {
            int x = rand() % cells.size();
            int y = rand() % cells.size();
            int direction = rand() % 2; // 0 - poziomo, 1 - pionowo

            if (canPlaceShip(x, y, ship.length, direction)) {
                placeShip(x, y, ship.length, direction);
                ship.x = x;
                ship.y = y;
                ship.horizontal = (direction == 0);
                break;
            }
        }
    }
}

void Board::draw(sf::RenderWindow& window, int offsetX, int offsetY) {
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(cellSize, cellSize));

    for (int i = 0; i < boardSize; ++i) {
        for (int j = 0; j < boardSize; ++j) {
            switch (cells[i][j].status) {
            case CellStatus::Ship:
                rectangle.setFillColor(sf::Color::White); // Ustaw kolor na bia�y dla statk�w (zamiast niebieskiego)
                break;
            case CellStatus::Hit:
                rectangle.setFillColor(sf::Color::Red); // Ustaw kolor na czerwony dla trafionych statk�w
                break;
            case CellStatus::Miss:
                rectangle.setFillColor(sf::Color::Blue); // Ustaw kolor na niebieski dla pud�a
                break;
            default:
                rectangle.setFillColor(sf::Color::White); // Ustaw kolor na bia�y dla pustych miejsc
                break;
            }

            rectangle.setPosition(offsetX + i * cellSize, offsetY + j * cellSize);
            window.draw(rectangle);
        }
    }

    sf::RectangleShape line;
    line.setFillColor(sf::Color::Black);

    // Rysowanie linii pionowych
    for (int i = 0; i <= boardSize; ++i) {
        line.setSize(sf::Vector2f(1, boardSize * cellSize));
        line.setPosition(offsetX + i * cellSize, offsetY);
        window.draw(line);
    }

    // Rysowanie linii poziomych
    for (int j = 0; j <= boardSize; ++j) {
        line.setSize(sf::Vector2f(boardSize * cellSize, 1));
        line.setPosition(offsetX, offsetY + j * cellSize);
        window.draw(line);
    }
}


void Board::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << filename << std::endl;
        return;
    }

    for (const auto& row : cells) {
        for (const auto& cell : row) {
            file << static_cast<int>(cell.status) << ' ';
        }
        file << '\n';
    }
}

class Game {
private:
    sf::RenderWindow window;
    Board playerBoard;
    Board aiBoard;
public:
    Game();
    void run();
    void handleMouseClick(int x, int y);
    void aiTurn();
};

void Game::aiTurn() {
    int x, y;
    while (true) {
        // Wybierz losowe wsp�rz�dne
        x = rand() % playerBoard.getBoardSize();
        y = rand() % playerBoard.getBoardSize();

        // Sprawd�, czy AI strzela�o ju� w te miejsce
        if (playerBoard.getCellStatus(x, y) == CellStatus::Empty ||
            playerBoard.getCellStatus(x, y) == CellStatus::Ship) {
            break;
        }
    }

    // Strzelaj w wybrane miejsce
    CellStatus result = playerBoard.shoot(x, y);

    // Informuj AI o wyniku strza�u
    if (result == CellStatus::Miss) {
        std::cout << "AI pud�o!\n";
    }
    else if (result == CellStatus::Hit) {
        std::cout << "AI trafi�!\n";
    }
}



void Game::handleMouseClick(int x, int y) {
    // Przekszta�� wsp�rz�dne myszy na wsp�rz�dne kom�rki
    int cellX = x / aiBoard.getCellSize();
    int cellY = y / aiBoard.getCellSize();

    // Sprawd�, czy klikni�cie by�o w obr�bie planszy AI
    if (cellX < 0 || cellX >= aiBoard.getBoardSize() || cellY < 0 || cellY >= aiBoard.getBoardSize()) {
        std::cout << "Klikni�cie poza plansz� AI!\n";
        return;
    }

    // Strzelaj do wybranej kom�rki
    CellStatus result = aiBoard.shoot(cellX, cellY);

    // Informuj gracza o wyniku strza�u
    if (result == CellStatus::Miss) {
        std::cout << "Pud�o!\n";
    }
    else if (result == CellStatus::Hit) {
        std::cout << "Trafiony!\n";
    }
    else {
        std::cout << "Ju� tam strzela�e�!\n";
    }

    // AI wykonuje ruch po ruchu gracza
    aiTurn();
    if (playerBoard.allShipsSunk()) {
        std::cout << "Wszystkie Twoje statki zosta�y zniszczone! Przegra�e�!\n";
        window.close();
    }
}



Game::Game() : window(sf::VideoMode(800, 600), "Battleship Game"), playerBoard(10, 30, { 6, 4, 4, 3, 3, 2, 2 }), aiBoard(10, 30, { 6, 4, 4, 3, 3, 2, 2 }) {
    // Zainicjalizuj gr�
}

bool Board::allShipsSunk() const {
    for (const auto& row : cells) {
        for (const auto& cell : row) {
            if (cell.status == CellStatus::Ship) {
                return false; // Znaleziono statek, kt�ry nie zosta� jeszcze zniszczony
            }
        }
    }
    return true; // Wszystkie statki zosta�y zniszczone
}


void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                    if (aiBoard.allShipsSunk()) {
                        std::cout << "Wszystkie statki AI zosta�y zniszczone! Wygra�e�!\n";
                        window.close();
                    }
                }
            }
        }

        window.clear();
        playerBoard.draw(window, 0, 0); // bez przesuni�cia
        aiBoard.draw(window, aiBoard.getCellSize() * aiBoard.getBoardSize() + 50, 0); // przesuni�cie o rozmiar planszy + 50 pikseli
        window.display();
    }
    playerBoard.saveToFile("player_board.txt");
    aiBoard.saveToFile("ai_board.txt");
}


int main() {
    Game game;
    game.run();
    return 0;
}