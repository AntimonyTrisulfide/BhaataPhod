#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <SFML/System.hpp>
#include <cmath>
#include <SFML/Audio.hpp>

// Utility function to get the angle between two points
float getAngle(const sf::Vector2f& start, const sf::Vector2f& end) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    return std::atan2(dy, dx) * 180 / 3.14159265; // Convert to degrees
}

class UFO_Bullet {
public:
    UFO_Bullet(const sf::Texture& texture, const sf::Vector2f& position, float rotation) {
		sprite.setTexture(texture);
		sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
		sprite.setPosition(position);
		sprite.setRotation(rotation);
	}

    void update(float deltaTime) {
		float rotation = sprite.getRotation() - 90; // Adjust as needed
		float radian = rotation * 3.14159265 / 180;
		sprite.move(std::cos(radian) * 700.f * deltaTime, std::sin(radian) * 700.f * deltaTime); // Adjust the speed here
	}

    void draw(sf::RenderWindow& window) const {
		window.draw(sprite);
	}

    sf::FloatRect getBounds() const {
		return sprite.getGlobalBounds();
	}

    sf::Vector2f getPosition() const {
		return sprite.getPosition();
	}
private:
    sf::Sprite sprite;
};

class UFO_Boss {
public:
    UFO_Boss(const sf::Vector2f& position, const sf::Vector2u& windowSize, const sf::Texture& texture)
        : windowSize(windowSize) {

		sprite.setTexture(texture);
		sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
		sprite.setPosition(position);
	}

    void draw(sf::RenderWindow& window) const {
		window.draw(sprite);
	}

    sf::FloatRect getBounds() const {
		return sprite.getGlobalBounds();
	}

    sf::Vector2f getPosition() const {
		return sprite.getPosition();
	}

    void update(sf::Vector2f playerPosition, float deltaTime) {
		//UFO Boss follows the player
		sf::Vector2f directionToPlayer = playerPosition - sprite.getPosition();
		float length = std::sqrt(directionToPlayer.x * directionToPlayer.x + directionToPlayer.y * directionToPlayer.y);
		directionToPlayer /= length; // Normalize the vector
		sprite.move(directionToPlayer * 600.f * deltaTime);

		// Screen wrapping
        sf::Vector2f position = sprite.getPosition();
        if (position.x < 0) position.x = windowSize.x;
        if (position.x > windowSize.x) position.x = 0;
        if (position.y < 0) position.y = windowSize.y;
        if (position.y > windowSize.y) position.y = 0;
        sprite.setPosition(position);
	}

private:
	sf::Sprite sprite;
    bool wasShooting = false;
    sf::Vector2u windowSize;
};

class Medkit {
public:
    Medkit(const sf::Texture& texture, const sf::Vector2f& position) {
		sprite.setTexture(texture);
		sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
		sprite.setPosition(position);
	}

    void draw(sf::RenderWindow& window) const {
		window.draw(sprite);
	}

    sf::FloatRect getBounds() const {
		return sprite.getGlobalBounds();
	}

    sf::Vector2f getPosition() const {
		return sprite.getPosition();
	}
private:
    sf::Sprite sprite;
};

class Health {
public:
    Health(const sf::Texture& fullHeartTex, const sf::Texture& halfHeartTex, int maxHearts)
        : fullHeartTexture(fullHeartTex), halfHeartTexture(halfHeartTex), maxHearts(maxHearts), currentHearts(maxHearts * 2) {
        heartSize = sf::Vector2f(fullHeartTexture.getSize());
    }

    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < maxHearts; ++i) {
            sf::Sprite heartSprite;
            if (i * 2 + 1 < currentHearts) {
                heartSprite.setTexture(fullHeartTexture);
            }
            else if (i * 2 + 1 == currentHearts) {
                heartSprite.setTexture(halfHeartTexture);
            }
            else {
                break; // No more hearts to draw
            }
            heartSprite.setPosition(10.f + i * (heartSize.x + 30.f), 10.f); // Position hearts with some spacing
            window.draw(heartSprite);
        }
    }

    void takeDamage(int damage) {
        currentHearts -= damage;
        if (currentHearts < 0) currentHearts = 0;
    }

    void resetHealth() {
        currentHearts = maxHearts * 2;
    }

    int getCurrentHearts() const {
        return currentHearts;
    }

private:
    const sf::Texture& fullHeartTexture;
    const sf::Texture& halfHeartTexture;
    int maxHearts;
    int currentHearts;
    sf::Vector2f heartSize;
};


class Animation {
public:
    Animation(const sf::Texture& texture, int frameWidth, int frameHeight, int numFrames, float frameTime)
        : frameWidth(frameWidth), frameHeight(frameHeight), numFrames(numFrames), frameTime(frameTime), currentFrame(0), elapsedTime(0.f) {
        sprite.setTexture(texture);
        sprite.setOrigin(frameWidth / 2, frameHeight / 2);
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
    }

    void update(float deltaTime) {
        elapsedTime += deltaTime;
        if (elapsedTime >= frameTime) {
            currentFrame = (currentFrame + 1) % numFrames;
            elapsedTime = 0.f;
            int column = currentFrame % (sprite.getTexture()->getSize().x / frameWidth);
            int row = currentFrame / (sprite.getTexture()->getSize().x / frameWidth);
            sprite.setTextureRect(sf::IntRect(column * frameWidth, row * frameHeight, frameWidth, frameHeight));
        }
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    bool isFinished() const {
        return currentFrame == numFrames - 1;
    }

private:
    sf::Sprite sprite;
    int frameWidth;
    int frameHeight;
    int numFrames;
    float frameTime;
    int currentFrame;
    float elapsedTime;
};

class Player {
public:
    Player(const sf::Texture& idleTexture, const sf::Texture& thrustingTexture, const sf::Vector2f& position, const sf::Vector2u& windowSize)
        : textureIdle(idleTexture), textureThrusting(thrustingTexture), velocity(0.f, 0.f), thrust(20.f), // Increase thrust // Decrease thrust duration
        windowSize(windowSize), isThrusting(false) {
        sprite.setTexture(textureIdle);
        sprite.setOrigin(textureIdle.getSize().x / 2, textureIdle.getSize().y / 2);
        sprite.setPosition(position);

        if (!thrustbuffer.loadFromFile("Materials/thrust.wav")) {
            std::cerr << "Error loading resources from file" << std::endl;
            exit(-1);
        }
        thrustsound.setBuffer(thrustbuffer);
    }
    sf::SoundBuffer thrustbuffer;
    sf::Sound thrustsound;

    void update(float deltaTime) {
        // Change the sprite based on whether the player is thrusting
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            isThrusting = true;
            sprite.setTexture(textureThrusting);
            if (sprite.getTexture() == &textureThrusting) {
                thrustsound.play();
            }
            if (sprite.getTexture() == &textureIdle) {
                thrustsound.stop();
            }
            //thrustTimer += deltaTime;
            //if (thrustTimer >= thrustDuration) {
            //    isThrusting = false;
            //    thrustsound.stop();
            //    sprite.setTexture(textureIdle);
            //}
        }
        else {
            isThrusting = false;
            //thrustTimer = 0.f;
            sprite.setTexture(textureIdle);
        }

        // Apply thrust
        if (isThrusting) {
            float rotation = sprite.getRotation() - 90;
            float radian = rotation * 3.14159265 / 180;
            velocity.x += std::cos(radian) * thrust * deltaTime;
            velocity.y += std::sin(radian) * thrust * deltaTime;
        }
        else {
            // Slow down the ship when not thrusting (optional)
            velocity *= 0.95f; // Adjust the factor as needed
        }

        const float maxSpeed = 500.f;
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > maxSpeed) {
            velocity /= speed; // Normalize the vector
			velocity *= maxSpeed;
		}

        // Apply inertia
        //sprite.move(velocity * deltaTime);
        sprite.move(velocity);

        // Screen wrapping
        sf::Vector2f position = sprite.getPosition();
        if (position.x < 0) position.x = windowSize.x;
        if (position.x > windowSize.x) position.x = 0;
        if (position.y < 0) position.y = windowSize.y;
        if (position.y > windowSize.y) position.y = 0;
        sprite.setPosition(position);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    float getRotation() const {
        return sprite.getRotation();
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    sf::Sprite getSprite() const {
        return sprite;
    }

    void setPosition(const sf::Vector2f& position) {
		sprite.setPosition(position);
	}

    void updateRotation(const sf::Vector2f& mousePosition) {
        sf::Vector2f direction = mousePosition - sprite.getPosition();
        float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265 + 90; // Convert to degrees and adjust
        sprite.setRotation(angle);
    }

private:
    sf::Sprite sprite;
    sf::Texture textureIdle;
    sf::Texture textureThrusting;
    sf::Vector2f velocity;
    float thrust;
    float thrustTimer = 0;
    sf::Vector2u windowSize;
    bool isThrusting;
};

class Projectile {
public:
    Projectile(const sf::Texture& texture, const sf::Vector2f& position, float rotation) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(position);
        sprite.setRotation(rotation);
    }

    void update(float deltaTime) {
        float rotation = sprite.getRotation() - 90; // Adjust as needed
        float radian = rotation * 3.14159265 / 180;
        sprite.move(std::cos(radian) * 600.f * deltaTime, std::sin(radian) * 600.f * deltaTime); // Adjust the speed here
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

private:
    sf::Sprite sprite;
    sf::Vector2u windowSize;
};

enum class EnemyType {
    Normal,
    Fast,
    Direct
};

class Enemy {
public:
    Enemy(const sf::Texture& texture, const sf::Texture& texture2, const sf::Vector2f& position, const sf::Vector2f& initialDirection, EnemyType type)
        : type(type), direction(initialDirection) {

        if (type == EnemyType::Direct) {
            sprite.setTexture(texture2);
        }
        else {
            sprite.setTexture(texture);
        }
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setPosition(position);
    }

    void update(const sf::Vector2f& playerPosition, float deltaTime) {
        float speed = 0.f;
        switch (type) {
        case EnemyType::Normal:
            speed = 100.f;
            break;
        case EnemyType::Fast:
            speed = 600.f;
            break;
        case EnemyType::Direct:
            speed = 600.f;
            break;
        }

        if (type != EnemyType::Direct) { // Check if enemy type is not Direct
            // Move in the initial direction only
            sprite.move(direction * speed * deltaTime);
        }
        else {
            // Move directly towards the player
            sf::Vector2f directionToPlayer = playerPosition - sprite.getPosition();
            float length = std::sqrt(directionToPlayer.x * directionToPlayer.x + directionToPlayer.y * directionToPlayer.y);
            directionToPlayer /= length; // Normalize the vector
            sprite.move(directionToPlayer * speed * deltaTime);
        }
    }


    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }
    sf::Sprite getSprite() const {
        return sprite;
    }
    EnemyType getType() const {
        return type;
    }

private:
    sf::Sprite sprite;
    EnemyType type;
    sf::Vector2f direction; // Store the initial direction
};

class Powerup {
public:
    Powerup(const sf::Texture& texture, const sf::Vector2f& position) {
		sprite.setTexture(texture);
		sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
		sprite.setPosition(position);
	}

    void draw(sf::RenderWindow& window) const {
		window.draw(sprite);
	}

    sf::FloatRect getBounds() const {
		return sprite.getGlobalBounds();
	}

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

private:
    sf::Sprite sprite;
};


class Game {
public:
    Game() : window(sf::VideoMode::getDesktopMode(), "Bhaata Phod", sf::Style::Fullscreen), score(0), isPaused(false), isStarted(false), isGameOver(false),
        health(fullHeartTexture, halfHeartTexture, 5) { // Initialize the Health instance
        window.setFramerateLimit(60);

        if (!playerTextureIdle.loadFromFile("Materials/spaceship-nofire.png") ||
            !playerTextureThrusting.loadFromFile("Materials/spaceship.png") ||
            !projectileTexture.loadFromFile("Materials/bullet.png") ||
            !UFOBulletTexture.loadFromFile("Materials/UFOBullet.png") ||
            !enemyTexture.loadFromFile("Materials/asteroid.png") ||
            !powerUpTexture.loadFromFile("Materials/powerup.png") ||
            !powerUpTexture1.loadFromFile("Materials/powerup1.png") ||
            !explosionTexture.loadFromFile("Materials/explosion.png") ||
            !shockwaveTexture.loadFromFile("Materials/shockwave.png") ||
            !fullHeartTexture.loadFromFile("Materials/full_heart.png") ||
            !halfHeartTexture.loadFromFile("Materials/half_heart.png") ||
            !shootbuffer.loadFromFile("Materials/LASER.wav") ||
            !mainmenubuffer.loadFromFile("Materials/TitleMenu.wav") ||
            !gameloopbuffer.loadFromFile("Materials/GameLoop.wav") ||
            !explosionbuffer.loadFromFile("Materials/explosion.wav") ||
            !shockwavebuffer.loadFromFile("Materials/shockwave.wav") ||
            !enemy2Texture.loadFromFile("Materials/BOMB.png") ||
            !backgroundTexture.loadFromFile("Materials/mainbackground.png") ||
            !creditsTexture.loadFromFile("Materials/credits.png") ||
            !CreditButtonTexture.loadFromFile("Materials/creditbutton.png") ||
            !startButtonTexture.loadFromFile("Materials/startbutton.png") ||
            !ruleTexture.loadFromFile("Materials/rules.png") ||
            !medkitTexture.loadFromFile("Materials/MedKit.png") ||
            !UFOtexture.loadFromFile("Materials/UFO.png") ||
            !exitButtonTexture.loadFromFile("Materials/exitbutton.png") ||
            !credits.loadFromFile("Materials/Credits.wav") ||
            !UFOBattlebuffer.loadFromFile("Materials/UFO Battle.wav")) {
            std::cerr << "Error loading resources from file" << std::endl;
            //open a error window
            sf::RenderWindow errorwindow(sf::VideoMode(800, 600), "Error", sf::Style::Default);
            if (!font.loadFromFile("Materials/NES.ttf")) {
				std::cerr << "Error loading font" << std::endl;
				exit(-1);
			}
            sf::Text errorText;
            errorText.setFont(font);
            errorText.setString("Error loading resources from file");
            errorText.setCharacterSize(50);
            errorText.setFillColor(sf::Color::Red);
            errorText.setPosition(800 / 2 - errorText.getLocalBounds().width / 2, 600 / 2 - errorText.getLocalBounds().height / 2);
            errorwindow.clear();
            errorwindow.draw(errorText);
            errorwindow.display();
            //wait for any key to be pressed
            while (true) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					errorwindow.close();
					break;
				}
			}
            errorwindow.close();
            sf::sleep(sf::seconds(5));
            exit(-1);
        }
        shoot.setBuffer(shootbuffer);
        explosion.setBuffer(explosionbuffer);
        mainmenu.setBuffer(mainmenubuffer);
        gameloop.setBuffer(gameloopbuffer);
        shockwavesound.setBuffer(shockwavebuffer);
        creditsmusic.setBuffer(credits);
        UFOBattle.setBuffer(UFOBattlebuffer);

        //loop the game loop sound

        player = std::make_unique<Player>(playerTextureIdle, playerTextureThrusting, sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2), window.getSize());
        spawnInitialEnemies(10); // Adjust the number of initial enemies as needed
    }

    void mainScreen() {
        TextureSize = backgroundTexture.getSize(); //Get size of texture.

        //Set WindowsSize to the size of the desktop screen.
        WindowSize = sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);

        float ScaleX = (float)WindowSize.x / TextureSize.x;
        float ScaleY = (float)WindowSize.y / TextureSize.y;     //Calculate scale.

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(ScaleX, ScaleY);      //Set scale. 
        backgroundSprite.setTexture(backgroundTexture);
        startButtonSprite.setTexture(startButtonTexture);
        exitButtonSprite.setTexture(exitButtonTexture);
        creditbuttonSprite.setTexture(CreditButtonTexture);

        // Set the position of buttons
        startButtonSprite.setPosition(1200.f - (603.f / 2.f), 1100.f - (385.f / 2.f)); // Adjust position as needed
        exitButtonSprite.setPosition(1200.f - (339.f / 2.f), 1400.f - (223.f / 2.f));  // Adjust position as needed
        // Set the position of the credit button to the bottom right
        creditbuttonSprite.setPosition(1800.f, 1200.f);  // Adjust position as needed
        

        window.clear();
        window.draw(backgroundSprite);
        window.draw(startButtonSprite);
        window.draw(exitButtonSprite);
        window.draw(creditbuttonSprite);
        window.display();
        mainmenu.setLoop(true);
        mainmenu.play();

        while (window.isOpen() && !isStarted) {
            sf::Event event;

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                if (event.type == sf::Event::Resized) {
                    window.setSize(sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
                }

                // Handle button clicks
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        if (startButtonSprite.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            // Start the game
                            mainmenu.stop();
                            // show the rule and control screen
                            window.clear();
                            
                            ruleSprite.setTexture(ruleTexture);
                            ruleSprite.setScale(ScaleX, ScaleY);
                            window.draw(ruleSprite);

                            window.display();
                            //wait here till a input
                            while (true) {
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                                    isStarted = true;
                                    window.clear();
                                    run();
                                    break;
                                }
                            }
                        }
                        else if (exitButtonSprite.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            window.close();
                        }
                        else if (creditbuttonSprite.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            //show the credits
                            mainmenu.stop();
                            creditsmusic.setLoop(true);
							creditsmusic.play();
							window.clear();
							creditsSprite.setTexture(creditsTexture);
							creditsSprite.setScale(ScaleX, ScaleY);
							window.draw(creditsSprite);
							window.display();
                            while (true) {
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                                    creditsmusic.stop();
									window.clear();
									mainScreen();
									break;
								}
							}
						
                        }
                    }
                }
            }
        }
    }


    void run() {
        sf::Clock clock;
        gameloop.setLoop(true);
        gameloop.play();
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            processEvents();
            if (!isPaused) {
                update(deltaTime);
                render();
            }
            else {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    gameloop.stop();
                    window.clear();
                    reset();
                    mainScreen();
                }
            }

            if (isGameOver) {
                gameloop.stop();
                break;
            }
        }
    }

    void spawnInitialEnemies(int count) {
        for (int i = 0; i < count; ++i) {
            sf::Vector2f position;
            switch (rand() % 4) {
            case 0: // Left edge
                position = sf::Vector2f(0, rand() % window.getSize().y);
                break;
            case 1: // Right edge
                position = sf::Vector2f(window.getSize().x, rand() % window.getSize().y);
                break;
            case 2: // Top edge
                position = sf::Vector2f(rand() % window.getSize().x, 0);
                break;
            case 3: // Bottom edge
                position = sf::Vector2f(rand() % window.getSize().x, window.getSize().y);
                break;
            }

            float angle = static_cast<float>(rand() % 360);
            sf::Vector2f directionToPlayer(std::cos(angle * 3.14159265 / 180), std::sin(angle * 3.14159265 / 180));

            enemies.push_back(Enemy(enemyTexture, enemy2Texture, position, directionToPlayer, EnemyType::Normal));
        }
    }


private:
    sf::SoundBuffer shootbuffer, explosionbuffer, mainmenubuffer, gameloopbuffer, shockwavebuffer, credits, UFOBattlebuffer;
    sf::Sound shoot, explosion, mainmenu, gameloop, shockwavesound, creditsmusic, UFOBattle;
    bool isGameOver;
    bool isStarted;
    bool MusicisPaused = false;
    Health health;
    sf::Font font;
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text shockwavecountText;
    sf::Text pauseText;
    sf::Text exitText;
    sf::Text medkitText;
    sf::Texture startButtonTexture;
    sf::Sprite startButtonSprite;
    sf::Texture exitButtonTexture;
    sf::Sprite exitButtonSprite;
    sf::Texture creditsTexture;
    sf::Sprite creditsSprite;
    sf::Texture CreditButtonTexture;
    sf::Sprite creditbuttonSprite;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Sprite ruleSprite;
    sf::Texture ruleTexture;
    sf::Vector2u TextureSize;  //Added to store texture size.
    sf::Vector2u WindowSize;   //Added to store window size.
    


    bool textintialized = false;

    void reset() {
        score = 0;
        //reset the posiiton of the player
        player->setPosition(sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2));
        isPaused = false;
        isStarted = false;
        isGameOver = false;
        health.resetHealth();
        projectiles.clear();
        enemies.clear();
        animations.clear();
        poweranimations.clear();
        powerupvector.clear();
        medkitvector.clear();
        UFO_Bosses.clear();
        UFO_Bullets.clear();
        spawnInitialEnemies(10);
    }


    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::Resized) {
                window.setSize(sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
            }
            if (event.type == sf::Event::KeyPressed && isStarted) {
                if (event.key.code == sf::Keyboard::Escape) {
                    isPaused = !isPaused;
                    if (isPaused) {
                        // Display pause text
                        pauseText.setString("Game Paused");
                        pauseText.setFillColor(sf::Color::White);
                        pauseText.setPosition(window.getSize().x / 2 - pauseText.getLocalBounds().width / 2, window.getSize().y / 2 - pauseText.getLocalBounds().height / 2);
                        //give exit option
                        exitText.setString("Press Space to Exit");
                        exitText.setFillColor(sf::Color::White);
                        exitText.setPosition(window.getSize().x / 2 - exitText.getLocalBounds().width / 2, window.getSize().y / 2 - exitText.getLocalBounds().height / 2 + 100);
                        window.display();
                        window.draw(pauseText);
                        window.draw(exitText);
                        window.display();
                    }
                }
                if (event.key.code == sf::Keyboard::Numpad0 && (shockwavecount > 0)) {
                    Animation shockwave(shockwaveTexture, 864, 864, 7, 0.075f);
                    shockwave.setPosition(player->getPosition());
                    shockwavesound.play();
                    poweranimations.push_back(shockwave);
                    shockwavecount--;
                }
            }
        }
    }


    void update(float deltaTime) {
        if (!textintialized) {
            if (!font.loadFromFile("Materials/NES.ttf")) {
                std::cerr << "Error loading font" << std::endl;
                exit(-1);
            }
            else {
                gameOverText.setFont(font);
                scoreText.setFont(font);
                pauseText.setFont(font);
                exitText.setFont(font);
                medkitText.setFont(font);
                shockwavecountText.setFont(font);
                textintialized = true;
            }
        }

        if (UFO_Bosses.size() > 0 && !MusicisPaused) {
            gameloop.pause();
            MusicisPaused = true;
            UFOBattle.play();
		}
        else if (UFO_Bosses.size() == 0 && MusicisPaused) {
			MusicisPaused = false;
            UFOBattle.stop();
			gameloop.play();
		}


        if (!isGameOver) {
            gameOverText.setString("Game Over");
            gameOverText.setCharacterSize(100);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition(window.getSize().x / 2 - gameOverText.getLocalBounds().width / 2, window.getSize().y / 2 - gameOverText.getLocalBounds().height / 2);
        }


        if (health.getCurrentHearts() <= 0) {
            isGameOver = true;
        }

        if (isGameOver) {
            // Clear all game elements
            projectiles.clear();
            enemies.clear();
            animations.clear();
            return; // Skip updating the rest of the game elements
        }

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        player->updateRotation(static_cast<sf::Vector2f>(mousePosition));
        player->update(deltaTime);

        for (size_t i = 0; i < projectiles.size(); ++i) {
            //remove the Normal Bullets that are out of bounds
            sf::Vector2f position = projectiles[i].getPosition();
            if (position.x < 0 || position.x > WindowSize.x || position.y < 0 || position.y > WindowSize.y) {
                projectiles.erase(projectiles.begin() + i);
                --i;
            }
        }

        for (auto& UFO_Boss : UFO_Bosses) {
			UFO_Boss.update(player->getPosition(), deltaTime);
		}

        for (auto& projectile : projectiles) {
            projectile.update(deltaTime);
        }

        for (auto& enemy : enemies) {
            enemy.update(player->getPosition(), deltaTime);
        }

        // Check for collisions and create animations
        checkCollisions();

        // Update animations
        for (size_t i = 0; i < animations.size(); ++i) {
            animations[i].update(deltaTime);
            if (animations[i].isFinished()) {
                animations.erase(animations.begin() + i);
                --i;
            }
        }

        for (size_t i = 0; i < poweranimations.size(); ++i) {
			poweranimations[i].update(deltaTime);
            if (poweranimations[i].isFinished()) {
				poweranimations.erase(poweranimations.begin() + i);
				--i;
			}
		}

        // Spawn medkit 
        if (medspawn == true && score != 0) {
			sf::Vector2f position;
            //position at a random location
            position = sf::Vector2f(rand() % window.getSize().x, rand() % window.getSize().y);
			Medkit newmedkit(medkitTexture, position);
			medkitvector.push_back(newmedkit);
		}

        //Spawn a UFO 
        if (medspawn == true && score != 0) {
		    for(int i = 0; i < 2; i++){
                sf::Vector2f position;
                //random possibility of spawning at the frame boundaries
                switch (rand() % 4) {
                    case 0:
					    position = sf::Vector2f(0, rand() % window.getSize().y);
					    break;
                    case 1:
                        position = sf::Vector2f(window.getSize().x, rand() % window.getSize().y);
                        break;
                    case 2:
					    position = sf::Vector2f(rand() % window.getSize().x, 0);
					    break;
                    case 3:
                        position = sf::Vector2f(rand() % window.getSize().x, window.getSize().y);
                        break;
                }
			
			    UFO_Boss newUFO(position, window.getSize(), UFOtexture);
			    UFO_Bosses.push_back(newUFO);
            }
            medspawn = false;
		}


        // Timer for spawning enemies
        static float enemySpawnTimer = 0.f;
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= 1.f) {
            EnemyType type = static_cast<EnemyType>(rand() % 3);
            // Make direct enemies spawn very less often
            if (type == EnemyType::Direct) {
                if (rand() % 10 > 1) {
                    type = EnemyType::Normal;
                }
            }

            sf::Vector2f position;
            switch (rand() % 4) {
            case 0:
                position = sf::Vector2f(0, rand() % window.getSize().y);
                break;
            case 1:
                position = sf::Vector2f(window.getSize().x, rand() % window.getSize().y);
                break;
            case 2:
                position = sf::Vector2f(rand() % window.getSize().x, 0);
                break;
            case 3:
                position = sf::Vector2f(rand() % window.getSize().x, window.getSize().y);
                break;
            }
            sf::Vector2f playerPosition = player->getPosition();
            sf::Vector2f directionToPlayer = playerPosition - position;
            float length = std::sqrt(directionToPlayer.x * directionToPlayer.x + directionToPlayer.y * directionToPlayer.y);
            directionToPlayer /= length; // Normalize the vector
            const float offset = 50.0f; // Adjust this offset value as needed

            if (type == EnemyType::Normal) {
                // Create the first enemy at the original position
                enemies.push_back(Enemy(enemyTexture, enemy2Texture, position, directionToPlayer, type));

                // Adjust the position for the second enemy to be next to the first one
                sf::Vector2f adjacentPosition = position;
                adjacentPosition.x += offset; // Adjust this line for horizontal placement
                // adjacentPosition.y += offset; // Uncomment and adjust for vertical placement

                // Create the second enemy at the adjusted position
                enemies.push_back(Enemy(enemyTexture, enemy2Texture, adjacentPosition, directionToPlayer, type));
            }
            else {
                enemies.push_back(Enemy(enemyTexture, enemy2Texture, position, directionToPlayer, type));
            }
            enemySpawnTimer = 0.f;
        }

        // Shooting logic
        bool isShooting = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        static float shootCooldown = 0.2f; // Adjust as needed for your game
        static float shootTimer = shootCooldown;

        shootTimer += deltaTime;

        if (shootTimer < shootCooldown) {
            // Shooting on cooldown, do nothing
            wasShooting = true;
        }
        else if (isShooting && !wasShooting) {
            sf::Vector2f playerPosition = player->getPosition();
            sf::Vector2f direction = static_cast<sf::Vector2f>(mousePosition) - playerPosition;
            float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265 + 90; // Convert to degrees and adjust
            projectiles.push_back(Projectile(projectileTexture, playerPosition, angle));
            shootTimer = 0.f;
            shoot.play();
        }

        if (!isShooting) {
            wasShooting = false;
        }


        //shoot the player every 2 seconds
        static float enemyshootCooldown = 0.8f;
        static float enemyshootTimer = enemyshootCooldown;
        enemyshootTimer += deltaTime;

        //shoot the player
        for (size_t i = 0; i < UFO_Bosses.size(); ++i) {
            if (enemyshootTimer >= enemyshootCooldown) {
                sf::Vector2f playerPosition = player->getPosition();
                sf::Vector2f direction = playerPosition - UFO_Bosses[i].getPosition();
                float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265 + 90; // Convert to degrees and adjust
                UFO_Bullets.push_back(UFO_Bullet(UFOBulletTexture, UFO_Bosses[i].getPosition(), angle));
                enemyshootTimer = 0.f;
            }
        }

        // Update UFO_Bullets
        for (size_t i = 0; i < UFO_Bullets.size(); ++i) {
            UFO_Bullets[i].update(deltaTime);
        }

        // Remove UFO_Bullets that are out of bounds
        for (size_t i = 0; i < UFO_Bullets.size(); ++i) {
            //remove the UFO_Bullets that are out of bounds
            sf::Vector2f position = UFO_Bullets[i].getPosition();
            if (position.x < 0 || position.x > WindowSize.x || position.y < 0 || position.y > WindowSize.y) {
                UFO_Bullets.erase(UFO_Bullets.begin() + i);
                --i;
            }
        }


    }


    void checkCollisions() {
        // Check for collisions between projectiles and enemies
        for (size_t i = 0; i < projectiles.size(); ++i) {
            for (size_t j = 0; j < enemies.size(); ++j) {
                if (projectiles[i].getBounds().intersects(enemies[j].getBounds())) {
                    // Create explosion animation
                    //score according to enemy type
                    if (enemies[j].getType() == EnemyType::Normal) {
                        score += 20;
                    }
                    else if (enemies[j].getType() == EnemyType::Fast) {
                        score += 40;
                    }
                    else if (enemies[j].getType() == EnemyType::Direct) {
                        score += 80;
                        if (rand() % 100 < 10) {
                            //spawn a powerup
                            medspawn = true;
                            sf::Vector2f position = enemies[j].getPosition();
                            Powerup newpowerup(powerUpTexture, position);
                            powerupvector.push_back(newpowerup);
                        }
                    }
                    explosion.play();
                    Animation explosionAnim(explosionTexture, 126, 138, 8, 0.05f); // Assuming each frame is 64x64 and there are 16 frames
                    explosionAnim.setPosition(enemies[j].getPosition());
                    animations.push_back(explosionAnim);

                    // Remove projectile and enemy
                    projectiles.erase(projectiles.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    --i;
                    break;
                }
            }
        }

        // Check for collision between shockwave and UFO_Bosses
        for (size_t i = 0; i < poweranimations.size(); ++i) {
            for (size_t j = 0; j < UFO_Bosses.size(); ++j) {
                if (poweranimations[i].getBounds().intersects(UFO_Bosses[j].getBounds())) {
					// Create explosion animation
					score += 100;
					explosion.play();
					Animation explosionAnim(explosionTexture, 126, 138, 8, 0.05f); // Assuming each frame is 64x64 and there are 16 frames
					explosionAnim.setPosition(UFO_Bosses[j].getPosition());
					animations.push_back(explosionAnim);

					// Remove projectile and UFO_Boss
					poweranimations.erase(poweranimations.begin() + i);
					UFO_Bosses.erase(UFO_Bosses.begin() + j);
					--i;
					break;
				}
			}
		}


        // Check for collisions between projectiles and UFO_Bosses
        for (size_t i = 0; i < projectiles.size(); ++i) {
            for (size_t j = 0; j < UFO_Bosses.size(); ++j) {
                if (projectiles[i].getBounds().intersects(UFO_Bosses[j].getBounds())) {
					// Create explosion animation
					score += 100;
					explosion.play();
					Animation explosionAnim(explosionTexture, 126, 138, 8, 0.05f); // Assuming each frame is 64x64 and there are 16 frames
					explosionAnim.setPosition(UFO_Bosses[j].getPosition());
					animations.push_back(explosionAnim);

					// Remove projectile and UFO_Boss
					projectiles.erase(projectiles.begin() + i);
					UFO_Bosses.erase(UFO_Bosses.begin() + j);

					--i;
					break;
				}
			}
		}


        // Check for collisions between player and enemies
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (player->getBounds().intersects(enemies[i].getBounds())) {
                // Create collision animation
                explosion.play();
                Animation collision(explosionTexture, 126, 138, 8, 0.05f); // Reusing explosion texture for collision
                collision.setPosition(player->getPosition());
                animations.push_back(collision);
                

                // Handle player damage
                health.takeDamage(1); // Each collision takes half a heart (1 unit)

                // Remove enemy
                enemies.erase(enemies.begin() + i);

                --i;
                break;
            }
        }

        //check for collision between player and UFO_Boss
        for (size_t i = 0; i < UFO_Bosses.size(); ++i) {
            if (player->getBounds().intersects(UFO_Bosses[i].getBounds())) {
				// Create collision animation
				explosion.play();
				Animation collision(explosionTexture, 126, 138, 8, 0.05f); // Reusing explosion texture for collision
				collision.setPosition(player->getPosition());
				animations.push_back(collision);

				// Handle player damage
				health.takeDamage(4); // Each collision takes half a heart (1 unit)

				// Remove UFO_Boss
				UFO_Bosses.erase(UFO_Bosses.begin() + i);
				--i;
				break;
			}
		}

        //check for collision between player and UFO_Bullet
        for (size_t i = 0; i < UFO_Bullets.size(); ++i) {
            if (player->getBounds().intersects(UFO_Bullets[i].getBounds())) {
				// Create collision animation
				explosion.play();
				Animation collision(explosionTexture, 126, 138, 8, 0.05f); // Reusing explosion texture for collision
				collision.setPosition(player->getPosition());
				animations.push_back(collision);

				// Handle player damage
				health.takeDamage(1); // Each collision takes half a heart (1 unit)

				// Remove UFO_Bullet
				UFO_Bullets.erase(UFO_Bullets.begin() + i);
				--i;
				break;
			}
		}

        //check for collision between player and powerupsprite
        for (size_t i = 0; i < powerupvector.size(); ++i) {
            if (player->getBounds().intersects(powerupvector[i].getBounds())) {
                shockwavecount++;
				// Remove powerup
				powerupvector.erase(powerupvector.begin() + i);
				--i;
				break;
			}
		}
  

        //check for collision between powerup global bounds and enemy global bounds
        for (size_t i = 0; i < poweranimations.size(); ++i) {
            for (size_t j = 0; j < enemies.size(); ++j) {
                //intersection of enemies and powerup
                if (poweranimations[i].getBounds().intersects(enemies[j].getBounds())) {
					// Create explosion animation
					//score according to enemy type
                    if (enemies[j].getType() == EnemyType::Normal) {
						score += 20;
					}
                    else if (enemies[j].getType() == EnemyType::Fast) {
						score += 40;
					}
                    else if (enemies[j].getType() == EnemyType::Direct) {
						score += 80;
					}
					explosion.play();
					Animation explosionAnim(explosionTexture, 126, 138, 8, 0.05f); // Assuming each frame is 64x64 and there are 16 frames
					explosionAnim.setPosition(enemies[j].getPosition());
					animations.push_back(explosionAnim);

					// Remove projectile and enemy
					enemies.erase(enemies.begin() + j);
					--i;
					break;
				
                }
            }
        }

        //check for collision between player and medkit
        for (size_t i = 0; i < medkitvector.size(); ++i) {
            if (player->getBounds().intersects(medkitvector[i].getBounds())) {
				health.takeDamage(-2); // Each medkit heals 1 unit
				medkituse++;
				// Remove medkit
				medkitvector.erase(medkitvector.begin() + i);
				--i;
				break;
			}
		}
    }

    void render() {
        window.clear();

        if (isGameOver) {
            window.clear();
            window.draw(gameOverText);
            scoreText.setCharacterSize(45);
            scoreText.setPosition(window.getSize().x / 2.f - scoreText.getLocalBounds().width / 2.f, window.getSize().y / 2.f - scoreText.getLocalBounds().height + 250.f / 2.f);
            window.draw(scoreText);
            window.display();
            //wait 2 sec without clock
            sf::sleep(sf::seconds(5));
            gameloop.stop();
            window.clear();
            reset();
            mainScreen();

        }

        else {
            // Draw the game elements only if the game is not over
            player->draw(window);

            //display score shockwave and medkit
            medkitText.setString("Medkit Used: " + std::to_string(medkituse));
            medkitText.setCharacterSize(30);
            medkitText.setPosition(window.getSize().x - medkitText.getLocalBounds().width - 20, 80);
            window.draw(medkitText);
            shockwavecountText.setString("Shockwave Count: " + std::to_string(shockwavecount));
            shockwavecountText.setCharacterSize(30);
            shockwavecountText.setPosition(window.getSize().x - shockwavecountText.getLocalBounds().width - 20, 50);
            window.draw(shockwavecountText);
            scoreText.setString("Score: " + std::to_string(score));
            scoreText.setPosition(window.getSize().x - scoreText.getLocalBounds().width - 20, 20);
            scoreText.setCharacterSize(30);
            scoreText.setFillColor(sf::Color::White);
            window.draw(scoreText);

            for (const auto& UFO_Bullet : UFO_Bullets) {
				UFO_Bullet.draw(window);
			}

            for (const auto& projectile : projectiles) {
                projectile.draw(window);
            }

            for (const auto& UFO_Boss : UFO_Bosses) {
                UFO_Boss.draw(window);
            }

            for (const auto& enemy : enemies) {
                enemy.draw(window);
            }

            for (const auto& powerup : powerupvector) {
				powerup.draw(window);
			}

            for (const auto& poweranimation : poweranimations) {
				poweranimation.draw(window);
			}

            for (const auto& medkit : medkitvector) {
                medkit.draw(window);
            }


            // Draw animations
            for (const auto& animation : animations) {
                animation.draw(window);
            }

            // Draw health bar
            health.draw(window);
        }

        window.display();
    }


    sf::RenderWindow window;
    std::unique_ptr<Player> player;
    std::vector<Projectile> projectiles;
    std::vector<UFO_Bullet> UFO_Bullets;
    std::vector<Enemy> enemies;
    std::vector<Animation> animations, poweranimations;
    std::vector<Powerup> powerupvector;
    std::vector<Medkit> medkitvector;
    std::vector<UFO_Boss> UFO_Bosses;
    sf::Texture medkitTexture;
    sf::Texture playerTextureIdle;
    sf::Texture playerTextureThrusting;
    sf::Texture projectileTexture;
    sf::Texture enemyTexture;
    sf::Texture enemy2Texture;
    sf::Texture powerUpTexture;
    sf::Texture powerUpTexture1;
    sf::Texture explosionTexture;
    sf::Texture shockwaveTexture;
    sf::Texture fullHeartTexture;
    sf::Texture halfHeartTexture;
    sf::Texture UFOtexture;
    sf::Texture UFOBulletTexture;
    int score = 0;
    bool medspawn = false;
    int shockwavecount = 1;
    int medkituse = 0;
    bool isPaused;
    bool wasShooting = false;
};

int main() {
    Game game;
    game.mainScreen();
    return 0;
}