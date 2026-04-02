#include "Ship.h"
#include <SFML/Graphics.hpp>
#include <cmath>

#include <iostream>

#include "IsometricUtils.h"
#include "TextureCache.h"

Ship::Ship(float x, float y, float speed)
    : Actor(x, y, 20),
      speed(speed),
      moveUp(false),
      moveDown(false),
      moveLeft(false),
      moveRight(false),
      shootPressed(false),
      fireRate(0.15f),
      timeSinceLastShot(0.0f),
      sprite(nullptr),
      mode(Mode::Air),
      facing(Facing::Down),
      aimUp(false),
      aimDown(false),
      aimLeft(false),
      aimRight(false),
      groundCurrentFrame(0),
      groundAnimTimer(0.0f),
      groundFrameDuration(0.08f) {
    if (loadTexture() && texAir) {
        sprite = std::make_unique<sf::Sprite>(*texAir);

        sf::FloatRect bounds = sprite->getLocalBounds();
        sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));

        sprite->setPosition(position);
    }
}

Ship::Facing Ship::getFacing() const {
    return facing;
}

void Ship::handleAimInput(const sf::Keyboard::Key& key, bool isPressed) {
    bool prevUp = aimUp;
    bool prevDown = aimDown;
    bool prevLeft = aimLeft;
    bool prevRight = aimRight;

    switch (key) {
        case sf::Keyboard::Key::I:
            aimUp = isPressed;
            break;
        case sf::Keyboard::Key::K:
            aimDown = isPressed;
            break;
        case sf::Keyboard::Key::J:
            aimLeft = isPressed;
            break;
        case sf::Keyboard::Key::L:
            aimRight = isPressed;
            break;
        default:
            break;
    }

    if (prevUp != aimUp || prevDown != aimDown || prevLeft != aimLeft || prevRight != aimRight) {
        if (aimUp && aimRight) facing = Facing::UpRight;
        else if (aimUp && aimLeft) facing = Facing::UpLeft;
        else if (aimDown && aimRight) facing = Facing::DownRight;
        else if (aimDown && aimLeft) facing = Facing::DownLeft;
        else if (aimUp) facing = Facing::Up;
        else if (aimDown) facing = Facing::Down;
        else if (aimRight) facing = Facing::Right;
        else if (aimLeft) facing = Facing::Left;
    }
}

Ship::Mode Ship::getMode() const {
    return mode;
}

bool Ship::loadTexture() {
    auto& cache = TextureCache::instance();
    bool any = false;

    texAir = cache.load("assets/characters/player/player_sky.png");
    if (texAir) {
        std::cout << "Loaded air ship texture: assets/characters/player/player_sky.png" << std::endl;
        any = true;
    }

    groundTexDownDiag = cache.load("assets/characters/player/player_ground_down_d.png");
    if (groundTexDownDiag) {
        std::cout << "Loaded ground down-diag texture" << std::endl;
        any = true;
    }
    groundTexStraight = cache.load("assets/characters/player/player_ground_straight.png");
    if (groundTexStraight) {
        std::cout << "Loaded ground straight texture" << std::endl;
        any = true;
    }
    groundTexUpDiag = cache.load("assets/characters/player/player_ground_up_d.png");
    if (groundTexUpDiag) {
        std::cout << "Loaded ground up-diag texture" << std::endl;
        any = true;
    }

    return any;
}

void Ship::update(float deltaTime) {
    position += velocity * deltaTime;

    updateShooting(deltaTime);

    if (sprite) {
        if (mode == Mode::Ground) {
            groundAnimTimer += deltaTime;
            if (groundAnimTimer >= groundFrameDuration) {
                groundAnimTimer = 0.0f;
                groundCurrentFrame = (groundCurrentFrame + 1) % GROUND_TOTAL_FRAMES;
            }

            const sf::Texture* useTex = nullptr;
            float rotationDeg = 0.0f;
            bool flipX = false;

            switch (facing) {
                case Facing::Down:
                    useTex = groundTexStraight;
                    rotationDeg = 0.0f;
                    break;
                case Facing::Right:
                    useTex = groundTexStraight;
                    rotationDeg = -90.0f;
                    break;
                case Facing::Up:
                    useTex = groundTexStraight;
                    rotationDeg = 180.0f;
                    break;
                case Facing::Left:
                    useTex = groundTexStraight;
                    rotationDeg = 90.0f;
                    break;
                case Facing::DownLeft:
                    useTex = groundTexDownDiag;
                    rotationDeg = 0.0f;
                    break;
                case Facing::DownRight:
                    useTex = groundTexDownDiag;
                    rotationDeg = 0.0f;
                    flipX = true;
                    break;
                case Facing::UpRight:
                    useTex = groundTexUpDiag;
                    rotationDeg = 0.0f;
                    break;
                case Facing::UpLeft:
                    useTex = groundTexUpDiag;
                    rotationDeg = 0.0f;
                    flipX = true;
                    break;
            }

            if (useTex && useTex->getSize().x > 0) {
                sprite->setTexture(*useTex, true);
                int frameW = static_cast<int>(useTex->getSize().x) / GROUND_FRAME_COLS;
                int frameH = static_cast<int>(useTex->getSize().y) / GROUND_FRAME_ROWS;
                int col = groundCurrentFrame % GROUND_FRAME_COLS;
                int row = groundCurrentFrame / GROUND_FRAME_COLS;
                sprite->setTextureRect(
                    sf::IntRect(sf::Vector2i(col * frameW, row * frameH), sf::Vector2i(frameW, frameH)));
                sprite->setOrigin(sf::Vector2f(frameW / 2.0f, frameH / 2.0f));
                sprite->setPosition(position);
                sprite->setRotation(sf::degrees(rotationDeg));
                sprite->setScale(sf::Vector2f(flipX ? -1.0f : 1.0f, 1.0f));
            } else {
                sprite->setPosition(position);
            }
        } else {
            sprite->setPosition(position);
        }
    }
}

void Ship::handleInput(const sf::Keyboard::Key& key, bool isPressed) {
    switch (key) {
        case sf::Keyboard::Key::W:
        case sf::Keyboard::Key::Up:
            moveUp = isPressed;
            break;
        case sf::Keyboard::Key::S:
        case sf::Keyboard::Key::Down:
            moveDown = isPressed;
            break;
        case sf::Keyboard::Key::A:
        case sf::Keyboard::Key::Left:
            moveLeft = isPressed;
            break;
        case sf::Keyboard::Key::D:
        case sf::Keyboard::Key::Right:
            moveRight = isPressed;
            break;
        case sf::Keyboard::Key::Space:
            shootPressed = isPressed;
            break;
        case sf::Keyboard::Key::G:
            if (isPressed) {
                Mode newMode = (mode == Mode::Air) ? Mode::Ground : Mode::Air;
                if (newMode == Mode::Air && sprite && texAir) {
                    sprite->setTexture(*texAir, true);
                    aimUp = aimDown = aimLeft = aimRight = false;
                    sf::FloatRect bounds = sprite->getLocalBounds();
                    sprite->setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));
                    sprite->setRotation(sf::degrees(0.0f));
                    sprite->setScale(sf::Vector2f(1.0f, 1.0f));
                }
                mode = newMode;
            }
            break;
        default:
            break;
    }
}

void Ship::updateInput() {
    updateMovement();
}

void Ship::updateMouseAim(const sf::RenderWindow& window) {
    if (mode != Mode::Ground) return;

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);

    float dx = mouseWorldPos.x - position.x;
    float dy = mouseWorldPos.y - position.y;
    float angle = std::atan2(dy, dx);

    setFacingFromAngle(angle);
}

void Ship::setFacingFromAngle(float angle) {
    float degrees = angle * 180.0f / M_PI;
    while (degrees < 0) degrees += 360.0f;
    while (degrees >= 360.0f) degrees -= 360.0f;

    if (degrees >= 337.5f || degrees < 22.5f) facing = Facing::Right;
    else if (degrees >= 22.5f && degrees < 67.5f) facing = Facing::DownRight;
    else if (degrees >= 67.5f && degrees < 112.5f) facing = Facing::Down;
    else if (degrees >= 112.5f && degrees < 157.5f) facing = Facing::DownLeft;
    else if (degrees >= 157.5f && degrees < 202.5f) facing = Facing::Left;
    else if (degrees >= 202.5f && degrees < 247.5f) facing = Facing::UpLeft;
    else if (degrees >= 247.5f && degrees < 292.5f) facing = Facing::Up;
    else if (degrees >= 292.5f && degrees < 337.5f) facing = Facing::UpRight;
}

void Ship::updateMovement() {
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    if (moveUp) velocity.y -= speed;
    if (moveDown) velocity.y += speed;
    if (moveLeft) velocity.x -= speed;
    if (moveRight) velocity.x += speed;

    if (velocity.x != 0.0f && velocity.y != 0.0f) {
        float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        velocity.x = (velocity.x / length) * speed;
        velocity.y = (velocity.y / length) * speed;
    }
}

void Ship::draw(sf::RenderWindow& window) {
    if (sprite) {
        window.draw(*sprite);
    }
}

void Ship::setPosition(float x, float y) {
    Actor::setPosition(x, y);
    if (sprite) {
        sprite->setPosition(position);
    }
}

float Ship::getSpeed() const {
    return speed;
}

void Ship::setSpeed(float speed) {
    this->speed = speed;
}

bool Ship::shouldShoot() {
    if (shootPressed && timeSinceLastShot >= fireRate) {
        timeSinceLastShot = 0.0f;
        return true;
    }
    return false;
}

void Ship::updateShooting(float deltaTime) {
    timeSinceLastShot += deltaTime;
}

float Ship::getForwardAngle() const {
    const float PI = 3.14159265358979323846f;
    if (mode == Mode::Ground) {
        switch (facing) {
            case Facing::Right: return 0.0f;
            case Facing::DownRight: return PI / 4.0f;
            case Facing::Down: return PI / 2.0f;
            case Facing::DownLeft: return 3.0f * PI / 4.0f;
            case Facing::Left: return PI;
            case Facing::UpLeft: return -3.0f * PI / 4.0f;
            case Facing::Up: return -PI / 2.0f;
            case Facing::UpRight: return -PI / 4.0f;
        }
    }
    return -std::atan2(IsometricUtils::TILE_HEIGHT, IsometricUtils::TILE_WIDTH);
}

sf::FloatRect Ship::getBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    return sf::FloatRect(position, sf::Vector2f(0.f, 0.f));
}
