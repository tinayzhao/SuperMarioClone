#ifndef SUPERMARIOBROS_MARIO_H
#define SUPERMARIOBROS_MARIO_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Animation.h"
#include "Fallable.h"

enum class MarioForm
{
    BIG_MARIO,
    SMALL_MARIO
};

class Mario : public Fallable
{
public:
    explicit Mario(const std::string& resourcesDir);
    void draw(sf::RenderWindow& window);

    void walk();

    size_t getHeight() const override;

    void stopWalking();

    void jump();
    void setForm(MarioForm form);

    // TODO: Make the difference between these more clear
    void setAnimation();
    void updateAnimation();

    bool collideWithGround(size_t groundY) override;

    inline void setJumping(bool isJumping)
    {
        mJumping = isJumping;
    }

private:
    sf::Texture mTexture;

    MarioForm mForm;

    Animation walkingAnimation;
    Animation jumpingAnimation;
    Animation standingAnimation;

    Animation bigWalkingAnimation;
    Animation bigJumpingAnimation;
    Animation bigStandingAnimation;

    Animation* mActiveAnimation;

    bool mJumping;
};

#endif  // SUPERMARIOBROS_MARIO_H
