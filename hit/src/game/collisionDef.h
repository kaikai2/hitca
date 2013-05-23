#ifndef TANK_GAME_COLLISION_DEF_H
#define TANK_GAME_COLLISION_DEF_H

enum CollisionCategory
{
    CC_Player           = 0x00000001,
    CC_Enemy            = 0x00000002,
};

enum CollisionBits
{
    CB_Player   = CC_Player | CC_Enemy,
    CB_Enemy    = CC_Player | CC_Enemy,
};

#endif