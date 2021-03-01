#include "Entity.h"
#include <iostream>
#include <utility>

#include "SFML/Graphics.hpp"
#include "Utils.h"

namespace
{
int sign(float val)
{
    if (val > 0)
        return 1;
    if (val < 0)
        return -1;
    return 0;
}

EntitySide oppositeSide(EntitySide side)
{
    switch (side)
    {
    case EntitySide::TOP:
        return EntitySide::BOTTOM;

    case EntitySide::BOTTOM:
        return EntitySide::TOP;

    case EntitySide::LEFT:
        return EntitySide::RIGHT;

    case EntitySide::RIGHT:
        return EntitySide::LEFT;
    }
    throw std::runtime_error("Failed to match EntitySide");
}

// Jumping allows entities to clip through other entities
// To fix this, check the left and right sides for collisions first
const std::vector<EntitySide> SIDES{EntitySide::LEFT,
                                    EntitySide::RIGHT,
                                    EntitySide::BOTTOM,
                                    EntitySide::TOP};

const std::vector<EntityCorner> CORNERS{EntityCorner::UPPER_LEFT,
                                        EntityCorner::UPPER_RIGHT,
                                        EntityCorner::LOWER_RIGHT,
                                        EntityCorner::LOWER_LEFT};
}

std::string convertSideToString(EntitySide side)
{
    switch (side)
    {
    case EntitySide::TOP:
        return "TOP";

    case EntitySide::BOTTOM:
        return "BOTTOM";

    case EntitySide::LEFT:
        return "LEFT";

    case EntitySide::RIGHT:
        return "RIGHT";
    }
    throw std::runtime_error("Failed to match EntitySide");
}

bool isEnemy(EntityType type)
{
    switch (type)
    {
    case EntityType::GOOMBA:
        return true;
    default:
        return false;
    }
}

const float Entity::NO_MAX_VELOCITY_VALUE = -1;
const float Entity::MAX_FALLING_VELOCITY = 4.5;

Entity::Entity(sf::Sprite sprite,
               size_t spriteWidth,
               size_t spriteHeight,
               const Hitbox& hitbox,
               EntityType type,
               const sf::Vector2f& position,
               float maxVelocity) :
    mActiveSprite(std::move(sprite)),
    mVelocity(0, 0),
    mAcceleration(0, GRAVITY_ACCELERATION),
    mChangingDirection(false),
    mActiveAnimation(nullptr),
    mSpriteWidth(spriteWidth),
    mSpriteHeight(spriteHeight),
    mHitbox(hitbox),
    mInputEnabled(true),
    mLookDirection(1),
    mMaxVelocity(maxVelocity),
    mType(type)
{
    mActiveSprite.setPosition(upperCenterToUpperLeft(position));

    // sets origin of sprite to be midpoint of top edge
    // So that scaling by -1 works properly
    const auto horizontalMidpoint = spriteWidth / 2;
    mActiveSprite.setOrigin(horizontalMidpoint, 0);
}

sf::Vector2f Entity::upperCenterToUpperLeft(
        const sf::Vector2f& originalPosition) const
{
    return sf::Vector2(originalPosition.x + (mSpriteWidth / 2.f),
                       originalPosition.y);
}

Entity::~Entity() = default;

EntityType Entity::getType() const
{
    return mType;
}

std::optional<Collision> Entity::detectCollision(const Entity& other) const
{
    float lhsTopEdge = getTop() + mHitbox.mUpperLeftOffset.y;
    float lhsBottomEdge = lhsTopEdge + mHitbox.mSize.y;
    float lhsLeftEdge = getLeft() + mHitbox.mUpperLeftOffset.x;
    float lhsRightEdge = lhsLeftEdge + mHitbox.mSize.x;

    float eTopEdge = other.getTop();
    float eLeftEdge = other.getLeft();
    float eRightEdge = eLeftEdge + other.getWidth();
    float eBottomEdge = eTopEdge + other.getHeight();
    if (lhsLeftEdge < eRightEdge && lhsRightEdge > eLeftEdge &&
        lhsTopEdge < eBottomEdge && lhsBottomEdge > eTopEdge)
    {
        sf::Vector2f enemyEdge1, enemyEdge2;
        for (const auto& corner : CORNERS)
        {
            sf::Vector2f marioPathEnd;
            getHitboxCorner(corner, marioPathEnd);

            // Subtract the recorded mDeltaP to get original position this frame
            const auto marioPathStart = marioPathEnd - mDeltaP;

            for (const auto& side : SIDES)
            {
                other.getHitboxSide(side, true, enemyEdge1, enemyEdge2);
                if (Utils::IsIntersecting(marioPathStart,
                                          marioPathEnd,
                                          enemyEdge1,
                                          enemyEdge2))
                {
                    // We've detected which side of the enemy we're hitting
                    // Invert it to get which side of Mario is colliding
                    float xIntersection = 0.f;
                    if (side == EntitySide::LEFT)
                    {
                        xIntersection = eLeftEdge;
                    }
                    else if (side == EntitySide ::RIGHT)
                    {
                        xIntersection = eRightEdge;
                    }
                    return std::optional<Collision>{
                            Collision{oppositeSide(side),
                                      other.getType(),
                                      eTopEdge,
                                      xIntersection}};
                }
            }
        }
    }
    return {};
}

void Entity::onCollision(const Collision& collision)
{
    // do nothing
    (void)collision;
}

bool Entity::collideWithEntity(std::vector<std::unique_ptr<Entity>>& enemies)
{
    for (auto& enemy : enemies)
    {
        const auto possibleCollision = detectCollision(*enemy);
        if (possibleCollision.has_value())
        {
            const auto collision = possibleCollision.value();
            onCollision(Collision{collision.side,
                                  collision.entityType,
                                  collision.yIntersection,
                                  collision.xIntersection});
            enemy->onCollision(Collision{oppositeSide(collision.side),
                                         this->getType(),
                                         collision.yIntersection,
                                         collision.xIntersection});

            return true;
        }
    }
    return false;
}

void Entity::updateAnimation()
{
    setAnimationFromState();
    mActiveAnimation->processAction();
}

void Entity::setAnimationFromState()
{
    // Do nothing
}

void Entity::draw(sf::RenderWindow& window) const
{
    window.draw(mActiveSprite);
#ifdef DRAW_HITBOX
    mHitbox.draw(window);
#endif
}

float Entity::getBottom() const
{
    return getY() + getHeight();
}

float Entity::getTop() const
{
    return getY();
}

float Entity::getLeft() const
{
    const auto halfWidth = static_cast<long>(getWidth() / 2);
    return getX() - halfWidth;
}

float Entity::getRight() const
{
    const auto halfWidth = static_cast<long>(getWidth() / 2);
    return getX() + halfWidth;
}

sf::Vector2f Entity::getVelocity() const
{
    return mVelocity;
}

void Entity::setVelocity(const sf::Vector2f& newVelocity)
{
    if (mInputEnabled)
        mVelocity = newVelocity;
}

sf::Vector2f Entity::getAcceleration() const
{
    return mAcceleration;
}

void Entity::die()
{
}

void Entity::setAcceleration(const sf::Vector2f& acceleration)
{
    if (mInputEnabled)
    {
        if (acceleration == this->mAcceleration)
            return;

        if (acceleration.x != 0 &&
            (sign(acceleration.x) != sign(static_cast<float>(mLookDirection))))
        {
            mChangingDirection = true;
            mLookDirection *= -1;
        }

        mAcceleration = acceleration;
    }
}

const Hitbox& Entity::getHitbox() const
{
    return mHitbox;
}

void Entity::getHitboxSide(const EntitySide& side,
                           bool extendEdges,
                           sf::Vector2f& p1,
                           sf::Vector2f& p2) const
{
    mHitbox.getSide(side, extendEdges, p1, p2);
}

void Entity::getHitboxCorner(const EntityCorner& corner,
                             sf::Vector2f& point) const
{
    mHitbox.getCorner(corner, point);
}

void Entity::addPositionDelta(float deltaX, float deltaY)
{
    setPosition(getX() + deltaX, getY() + deltaY);
    mDeltaP.x += deltaX;
    mDeltaP.y += deltaY;
}

void Entity::updatePosition()
{
    mVelocity.x += mAcceleration.x;
    mVelocity.y += mAcceleration.y;

    if (mMaxVelocity != NO_MAX_VELOCITY_VALUE)
    {
        if (mVelocity.x > 0 && mVelocity.x > mMaxVelocity)
            mVelocity.x = mMaxVelocity;

        if (mVelocity.x < 0 && mVelocity.x < -mMaxVelocity)
            mVelocity.x = -mMaxVelocity;
    }

    if (mVelocity.y > MAX_FALLING_VELOCITY) mVelocity.y = MAX_FALLING_VELOCITY;

    addPositionDelta(mVelocity.x, mVelocity.y);
}

float Entity::getX() const
{
    return mActiveSprite.getPosition().x;
}

float Entity::getY() const
{
    return mActiveSprite.getPosition().y;
}

float Entity::getHeight() const
{
    return mSpriteHeight;
}

float Entity::getWidth() const
{
    return mSpriteWidth;
}

void Entity::setPosition(float x, float y)
{
    mActiveSprite.setPosition(x, y);
}

void Entity::setCleanupFlag()
{
    mCleanupFlag = true;
}

bool Entity::needsCleanup()
{
    return mCleanupFlag;
}
