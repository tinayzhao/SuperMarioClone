#include "Goomba.h"

#include <utility>

namespace
{
const auto MARIO_HEIGHT = 16;
}

Goomba::Goomba(std::shared_ptr<sf::Sprite> sprite) :
    Entity(std::move(sprite), 16, 16, Hitbox({8, 5}, {4, 7}))
    //Entity(std::move(sprite), 16, 16, Hitbox({16, 16}, {0, 0}))
{
    mActiveSprite->setPosition(150, 50);

    walkingAnimation.load(AnimationType::GOOMBA_WALKING, mActiveSprite);
    const auto spriteOrigin = MARIO_HEIGHT / 2;
    mActiveSprite->setOrigin(spriteOrigin, 0);

    mActiveAnimation = &walkingAnimation;
}
