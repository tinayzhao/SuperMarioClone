#ifndef SUPERMARIOBROS_MARIO_H
#define SUPERMARIOBROS_MARIO_H

#include "Animation.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
class Mario
{
public:
    Mario();
    void draw(sf::RenderWindow& window);

    void walk();

    void setPosition(size_t x, size_t y);
    size_t getX() const;
    size_t getY() const;

    void stopWalking();

    void jump();
    void grow();

private:
    std::shared_ptr<sf::Sprite> mActiveSprite;
    sf::Texture mTexture;

    Animation walkingAnimation;
    Animation jumpingAnimation;
    Animation standingAnimation;

    Animation bigWalkingAnimation;
};

#endif  // SUPERMARIOBROS_MARIO_H
