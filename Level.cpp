#include "Level.h"
#include "Event.h"
#include "SpriteMaker.h"
#include "Text.h"

#include <entities/Block.h>
#include <entities/Items.h>
#include <cmath>

Level::Level(std::unique_ptr<Mario> mario,
             std::vector<std::unique_ptr<Entity>>&& entities) :
    mMario(std::move(mario))
{
    mPoints = std::make_shared<Points>(0, sf::Vector2f{10, 18});
    for (auto& entity : entities)
        addEntity(std::move(entity));

    addHUDOverlay();
}

void Level::addHUDOverlay()
{
    mTextElements.push_back(
            std::make_shared<Text>("Mario", sf::Vector2f{10, 10}));
    mTextElements.push_back(mPoints);
}

bool Level::physicsAreOn() const
{
    return !mMario->isGrowing() && !mMario->isTransitioningToFire();
}

void Level::executeFrame(const KeyboardInput& input)
{
    // Reset mDeltaP
    mMario->mDeltaP.x = 0;
    mMario->mDeltaP.y = 0;

    if (physicsAreOn())
    {
        auto& entities = getEntities();
        for (auto& entity : entities)
        {
            entity->mDeltaP.x = 0;
            entity->mDeltaP.y = 0;
        }

        setMarioMovementFromController(input);
        mMario->updatePosition();

        for (auto& entity : entities)
        {
            entity->updatePosition();
            entity->doInternalCalculations();
        }

        mMario->collideWithEntity(entities);
        for (size_t ii = 0; ii < entities.size(); ++ii)
            for (size_t jj = ii + 1; jj < entities.size(); ++jj)
                entities[ii]->collideWithEntity(entities[jj]);

        for (auto& entity : entities)
        {
            entity->updateAnimation();
        }

        entities.erase(std::remove_if(entities.begin(),
                                      entities.end(),
                                      [](std::unique_ptr<Entity>& entity) {
                                          return entity->needsCleanup();
                                      }),
                       entities.end());
    }

    mMario->updateAnimation();

    for (const auto& event : getEventQueue())
    {
        switch (event.type)
        {
        case EventType::ENEMY_KILLED:
            onEnemyKilled(event.asEnemyKilled());
            break;
        case EventType::ITEM_SPAWNED:
            onItemSpawned(event.asItemSpawned());
            break;
        case EventType::BLOCK_SHATTERED:
            onBlockShattered(event.asBlockShattered());
        }
    }
    getEventQueue().clear();
}

void Level::onBlockShattered(const Event::BlockShattered& event)
{
    const auto left = event.position.x;
    const auto top = event.position.y;
    // Spawn block shards after destruction
    // Upper left
    addEntity(std::make_unique<BlockShard>(getSpriteMaker()->blockTexture,
                                           sf::Vector2f{left, top},
                                           sf::Vector2f(0, 0),
                                           sf::Vector2f(-1, -5)));

    // Upper right
    addEntity(std::make_unique<BlockShard>(getSpriteMaker()->blockTexture,
                                           sf::Vector2f{left + 8, top},
                                           sf::Vector2f(8, 0),
                                           sf::Vector2f(1, -5)));

    // Lower right
    addEntity(std::make_unique<BlockShard>(getSpriteMaker()->blockTexture,
                                           sf::Vector2f{left + 8, top + 8},
                                           sf::Vector2f(8, 8),
                                           sf::Vector2f(1, -5)));

    // Lower left
    addEntity(std::make_unique<BlockShard>(getSpriteMaker()->blockTexture,
                                           sf::Vector2f{left, top + 8},
                                           sf::Vector2f(0, 8),
                                           sf::Vector2f(-1, -5)));

    mPoints->addPoints(50);
}

void Level::addEntityToFront(std::unique_ptr<Entity> entity)
{
    auto& entities = getEntities();
    entities.insert(entities.begin(), std::move(entity));
}

void Level::onEnemyKilled(const Event::EnemyKilled& event)
{
    mPoints->addPoints(event.points);
}

void Level::onItemSpawned(const Event::ItemSpawned& event)
{
    // Adding to front to ensure that mushroom is drawn before the block
    // i.e. the block obscures the mushroom from view
    switch (event.type)
    {
    case EntityType::MUSHROOM:
        addEntityToFront(std::make_unique<Mushroom>(
                getSpriteMaker()->itemAndObjectTexture,
                event.position,
                event.blockTop));
        break;
    case EntityType::FIREFLOWER:
        addEntityToFront(std::make_unique<Fireflower>(
                getSpriteMaker()->itemAndObjectTexture,
                event.position,
                event.blockTop));
        break;
    default:
        throw std::runtime_error("Unhandled entity type");
    }
}

void Level::drawFrame(sf::RenderWindow& window)
{
    window.clear(sf::Color(0, 0, 255, 255));
    mMario->draw(window);
    for (auto& entity : getEntities())
    {
        entity->draw(window);
    }
    for (auto& text : mTextElements)
        text->draw(window);
}

void Level::setMarioMovementFromController(const KeyboardInput& currentInput)
{
    sf::Vector2f acceleration = mMario->getAcceleration();
    sf::Vector2f velocity = mMario->getVelocity();

    velocity.y = setVerticalVelocityDueToJumpStart(currentInput, velocity);
    if (currentInput.A.pressedThisFrame())
    {
        mMario->setJumping(true);
    }

    acceleration.y = calculateVerticalAcceleration(currentInput, velocity.x);

    if (currentInput.right.keyIsDown)
    {
        if (!currentInput.left.keyIsDown)
            acceleration.x = 1;
    }
    if (currentInput.left.keyIsDown)
    {
        if (!currentInput.right.keyIsDown)
            acceleration.x = -1;
    }
    if ((currentInput.left.releasedThisFrame() &&
         !currentInput.right.keyIsDown) ||
        (currentInput.right.releasedThisFrame() &&
         !currentInput.left.keyIsDown))
    {
        // TODO: Should we decelerate to 0?
        velocity.x = 0;
        acceleration.x = 0;
    }

    mMario->setAcceleration(acceleration);
    mMario->setVelocity(velocity);
}

float Level::calculateVerticalAcceleration(const KeyboardInput& currentInput,
                                           float xVelocity) const
{
    float result = 0;
    const auto xSpeed = std::fabs(xVelocity);
    if (currentInput.A.keyIsDown)
    {
        if (xSpeed < 1.0)
        {
            result = 1.0 / 8.0;
        }
        else if (xSpeed < 37.0 / 16.0)
        {
            result = 0.1172;
        }
        else
        {
            result = 0.15625;
        }
    }
    else
    {
        if (xSpeed < 1.0)
        {
            result = 7.0 / 16.0;
        }
        else if (xSpeed < 37.0 / 16.0)
        {
            result = 6.0 / 16.0;
        }
        else
        {
            result = 9.0 / 16.0;
        }
    }
    return result;
}

float Level::setVerticalVelocityDueToJumpStart(
        const KeyboardInput& currentInput, const sf::Vector2f& velocity) const
{
    float result = velocity.y;
    if (!mMario->isJumping() && currentInput.A.pressedThisFrame())
    {
        if (std::fabs(velocity.x) < 37.0 / 16.0)
        {
            result = -4.0;
        }
        else
        {
            result = -5.0;
        }
    }
    return result;
}

const Mario& Level::getMario() const
{
    return *mMario;
}

std::vector<Event> gEventQueue;

std::vector<Event>& getEventQueue()
{
    return gEventQueue;
}

void addEvent(Event event)
{
    gEventQueue.push_back(event);
}