#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>	
#include <iostream>
#include <vector>

class Gomoku {
private:
	sf::RenderWindow window;
	int board[15][15];
	struct Piece {
		sf::Vector2f position;
		sf::Color color;
	};
	std::vector<Piece> pieces;
	bool isBlackTurn;
	bool isGameOver;

	const float GRID_SIZE = 50.f;
	const float MARGIN = 50.f;

	sf::Font font;
	sf::Text statusText;
	sf::Text winText;

	sf::SoundBuffer moveBuffer;
	sf::Sound moveSound;

	sf::SoundBuffer winBuffer;
	sf::Sound winSound;



public:
	Gomoku() :
		window(sf::VideoMode({ 800, 800 }), "Gomoku Game"),
		isBlackTurn(true),
		isGameOver(false),
		font("arial.ttf"),
		statusText(font),
		winText(font),
		moveSound(moveBuffer),
		winSound(winBuffer)
	{
		window.setFramerateLimit(60);

		for (int i = 0; i < 15; i++)
			for (int j = 0; j < 15; j++)
				board[i][j] = 0;

		
		moveBuffer.loadFromFile("place_stone.wav");
		winBuffer.loadFromFile("win.wav");

		
		statusText.setCharacterSize(20);
		statusText.setFillColor(sf::Color::Black);
		statusText.setPosition({ 15.f, 10.f });

		
		winText.setCharacterSize(80);
		winText.setFillColor(sf::Color::Red);
		winText.setOutlineColor(sf::Color::White);
		winText.setOutlineThickness(2.f);
		winText.setString("");
	}

	void run() {
		while (window.isOpen()) {
			processEvents();
			render();
		}
	}

private:
	void processEvents() {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) window.close();

			
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->code == sf::Keyboard::Key::Z) {
					undoMove();
				}
			}
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->code == sf::Keyboard::Key::R) {
					resetGame();
				}
			}


			if (!isGameOver) {
				handleMouseClick(event);
			}

		}
	}

	
	void undoMove() {
		if (!pieces.empty()) {
			Piece lastPiece = pieces.back();

			int gridX = static_cast<int>((lastPiece.position.x - MARGIN + GRID_SIZE / 2.f) / GRID_SIZE);
			int gridY = static_cast<int>((lastPiece.position.y - MARGIN + GRID_SIZE / 2.f) / GRID_SIZE);

			board[gridX][gridY] = 0;       
			pieces.pop_back();            
			isBlackTurn = !isBlackTurn;   
			isGameOver = false;            
		}
	}

	void resetGame() {
		for (int i = 0; i < 15; i++)
			for (int j = 0; j < 15; j++)
				board[i][j] = 0;

		pieces.clear();

		isBlackTurn = true;
		isGameOver = false;
	}

	void handleMouseClick(const std::optional<sf::Event>& event) {
		if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
			if (mouseEvent->button == sf::Mouse::Button::Left) {
				sf::Vector2i mousePos = mouseEvent->position;

				int gridX = static_cast<int>((mousePos.x - MARGIN + GRID_SIZE / 2.f) / GRID_SIZE);
				int gridY = static_cast<int>((mousePos.y - MARGIN + GRID_SIZE / 2.f) / GRID_SIZE);

				if (gridX >= 0 && gridX < 15 && gridY >= 0 && gridY < 15) {
					if (board[gridX][gridY] == 0) {
						int colorCode = isBlackTurn ? 1 : 2;
						board[gridX][gridY] = colorCode;
						

						
						if (checkWin(gridX, gridY, colorCode)) {
							std::string winner = (colorCode == 1 ? "BLACK WINS!" : "WHITE WINS!");
							winText.setString(winner);

							
							sf::FloatRect textBounds = winText.getLocalBounds();

							winText.setOrigin({
								textBounds.position.x + textBounds.size.x / 2.0f,
								textBounds.position.y + textBounds.size.y / 2.0f
								});

							
							winText.setPosition({ 400.0f, 400.0f });

							winSound.play();
							isGameOver = true;
						}else {
							moveSound.play();
						}

						float centerX = MARGIN + gridX * GRID_SIZE;
						float centerY = MARGIN + gridY * GRID_SIZE;

						sf::Color currentColor = isBlackTurn ? sf::Color::Black : sf::Color::White;
						pieces.push_back({ {centerX, centerY}, currentColor });

						isBlackTurn = !isBlackTurn;
					}
				}
			}
		}
	}

	void updateText() {
		if (!isGameOver) {
			std::string info = isBlackTurn ? "Current: Black" : "Current: White";
			info += "  |  [Z] Undo  [R] Restart";
			statusText.setString(info);
			statusText.setFillColor(sf::Color::Black);
		}
		else {
			statusText.setString("GAME FINISHED  |  Press [R] to Restart");
			statusText.setFillColor(sf::Color(120, 120, 120));
		}
	}

	void render() {
		window.clear(sf::Color(240, 200, 150));
		drawGrid();
		drawPieces();

		updateText(); 
		window.draw(statusText);

		if (isGameOver) {
			window.draw(winText);
		}

		window.display();
	}

	void drawGrid() {
		for (int i = 0; i < 15; ++i) {
			sf::Vertex lineStart, lineEnd;
			lineStart.color = lineEnd.color = sf::Color::Black;

			lineStart.position = { MARGIN, MARGIN + i * GRID_SIZE };
			lineEnd.position = { MARGIN + 14 * GRID_SIZE, MARGIN + i * GRID_SIZE };
			sf::Vertex hLine[] = { lineStart, lineEnd };
			window.draw(hLine, 2, sf::PrimitiveType::Lines);

			lineStart.position = { MARGIN + i * GRID_SIZE, MARGIN };
			lineEnd.position = { MARGIN + i * GRID_SIZE, MARGIN + 14 * GRID_SIZE };
			sf::Vertex vLine[] = { lineStart, lineEnd };
			window.draw(vLine, 2, sf::PrimitiveType::Lines);
		}
	}

	void drawPieces() {
		for (const auto& p : pieces) {
			sf::CircleShape piece(18.f);
			piece.setFillColor(p.color);
			piece.setOutlineThickness(2.f);
			piece.setOutlineColor(sf::Color(50, 50, 50));
			piece.setOrigin({ 18.f, 18.f });
			piece.setPosition(p.position);
			window.draw(piece);
		}
	}

	
	bool checkWin(int x, int y, int color) {
		int dx[] = { 1, 0, 1, 1 };
		int dy[] = { 0, 1, 1, -1 };

		for (int i = 0; i < 4; i++) {
			int count = 1;
			int nx = x + dx[i];
			int ny = y + dy[i];
			while (nx >= 0 && nx < 15 && ny >= 0 && ny < 15 && board[nx][ny] == color) {
				count++;
				nx += dx[i];
				ny += dy[i];
			}

			nx = x - dx[i];
			ny = y - dy[i];
			while (nx >= 0 && nx < 15 && ny >= 0 && ny < 15 && board[nx][ny] == color) {
				count++;
				nx -= dx[i];
				ny -= dy[i];
			}
			if (count >= 5) return true;
		}
		return false;
	}
};

int main() {
	Gomoku game;
	game.run();
	return 0;
}