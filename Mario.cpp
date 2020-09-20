#include "Mario.h"
#include "Animation.h"

#include <memory>

namespace
{
}

Mario::Mario(const std::shared_ptr<sf::Sprite> sprite) :
    Fallable(sprite),
    mForm(MarioForm::SMALL_MARIO),
    mJumping(false)
{
    // TODO: Pass in ctor
    setMaxVelocity(2.f);

    mActiveSprite->setPosition(100, 100);

    walkingAnimation.load(AnimationType::WALKING, mActiveSprite);
    jumpingAnimation.load(AnimationType::JUMPING, mActiveSprite);
    standingAnimation.load(AnimationType::STANDING, mActiveSprite);

    bigWalkingAnimation.load(AnimationType::BIG_WALKING, mActiveSprite);
    bigStandingAnimation.load(AnimationType::BIG_STANDING, mActiveSprite);
    bigJumpingAnimation.load(AnimationType::BIG_JUMPING, mActiveSprite);

    mActiveAnimation = &standingAnimation;
    mActiveAnimation->processAction();

    const auto spriteOrigin = GRIDBOX_SIZE / 2;
    mActiveSprite->setOrigin(spriteOrigin, 0);
}

void Mario::draw(sf::RenderWindow& window)
{
    window.draw(*mActiveSprite);
}

size_t Mario::getHeight() const
{
    if (mForm == MarioForm::SMALL_MARIO)
        return GRIDBOX_SIZE;
    else
        return 2 * GRIDBOX_SIZE;
}

void Mario::setAnimation()
{
    if (mJumping)
    {
        jump();
    }
    else if (mAcceleration.x != 0)
    {
        walk();
    }
    else
    {
        stopWalking();
    }

    if (mChangingDirection)
    {
        mActiveSprite->scale(-1.f, 1.f);
        mChangingDirection = false;
    }
}

void Mario::updateAnimation()
{
    mActiveAnimation->processAction();
}

void Mario::walk()
{
    switch (mForm)
    {
    case MarioForm::BIG_MARIO:
        mActiveAnimation = &bigWalkingAnimation;
        break;
    case MarioForm::SMALL_MARIO:
        mActiveAnimation = &walkingAnimation;
        break;
    }
}

void Mario::setForm(MarioForm form)
{
    const bool formIsChanging = form != mForm;
    if (formIsChanging)
    {
        if (form == MarioForm::BIG_MARIO)
        {
            const auto currentY = mActiveSprite->getPosition().y;
            const auto newY = currentY - GRIDBOX_SIZE;
            mActiveSprite->setPosition(mActiveSprite->getPosition().x, newY);
            mActiveAnimation = &bigStandingAnimation;
        }
        else if (form == MarioForm::SMALL_MARIO)
        {
            const auto currentY = mActiveSprite->getPosition().y;
            const auto newY = currentY + GRIDBOX_SIZE;
            mActiveSprite->setPosition(mActiveSprite->getPosition().x, newY);
            mActiveAnimation = &standingAnimation;
        }
    }

    mForm = form;
}

void Mario::stopWalking()
{
    switch (mForm)
    {
    case MarioForm::BIG_MARIO:
        mActiveAnimation = &bigStandingAnimation;
        break;
    case MarioForm::SMALL_MARIO:
        mActiveAnimation = &standingAnimation;
        break;
    }
}

void Mario::jump()
{
    switch (mForm)
    {
    case MarioForm::BIG_MARIO:
        mActiveAnimation = &bigJumpingAnimation;
        break;
    case MarioForm::SMALL_MARIO:
        mActiveAnimation = &jumpingAnimation;
        break;
    }
}

bool Mario::collideWithGround(const size_t groundY)
{
    bool atGround = Fallable::collideWithGround(groundY);
    if (atGround)
    {
        setJumping(false);
    }
    return atGround;
}

