#include "Animation.h"

#include <iostream>

Animation::Animation() = default;

void Animation::load(AnimationType type,
                     const std::shared_ptr<sf::Sprite>& activeSprite)
{
    mSpriteIndex = 0;
    mTicsPerFrame = 2;
    mRemainingTicsThisFrame = 2;
    mActiveSprite = activeSprite;
    repeat = true;

    switch (type)
    {
    case AnimationType::WALKING:
        generateRectangles(4, 80, 34, 16, 17);
        break;
    case AnimationType::JUMPING:
        generateRectangles(2, 148, 34, 16, 17);
        repeat = false;
        break;
    case AnimationType::STANDING:
        generateRectangles(1, 80, 34, 16, 17);
        break;

    case AnimationType::BIG_WALKING:
        generateRectangles(4, 80, 1, 32, 17);
        break;
    case AnimationType ::BIG_JUMPING:
        generateRectangles(2, 148, 1, 32, 17);
        repeat = false;
        break;
    case AnimationType::BIG_STANDING:
        generateRectangles(1, 80, 1, 32, 17);
        break;

    case AnimationType::GOOMBA_WALKING:
        generateRectangles(2, 0, 16, 16, 16);
        mTicsPerFrame = 4;
        mRemainingTicsThisFrame = 4;
        break;

    case AnimationType::GOOMBA_DEATH:
        generateRectangles(1, 32, 16, 16, 16);
        break;

    case AnimationType::MARIO_DEATH:
        generateRectangles(1, 182, 34, 16, 16);
        break;
    }
}

void Animation::generateRectangles(size_t numRectangles,
                                   size_t initialOffset,
                                   size_t yOffset,
                                   size_t rectHeight,
                                   size_t rectXOffset)
{
    mActionRectangles.resize(numRectangles);
    for (size_t ii = 0; ii < numRectangles; ++ii)
    {
        mActionRectangles[ii] = sf::IntRect(initialOffset + (rectXOffset * ii),
                                            yOffset,
                                            16,
                                            rectHeight);
    }

    mActiveSprite->setTextureRect(mActionRectangles[0]);
}

void Animation::processAction()
{
    --mRemainingTicsThisFrame;
    if (mRemainingTicsThisFrame == 0)
    {
        ++mSpriteIndex;
        if (mSpriteIndex >= mActionRectangles.size())
        {
            if (repeat)
            {
                mSpriteIndex = 0;
            }
            else
            {
                mSpriteIndex = mActionRectangles.size() - 1;
            }
        }
        mRemainingTicsThisFrame = mTicsPerFrame;
    }
    mActiveSprite->setTextureRect(mActionRectangles[mSpriteIndex]);
}

size_t Animation::getSpriteIndex() const
{
    return mSpriteIndex;
}
