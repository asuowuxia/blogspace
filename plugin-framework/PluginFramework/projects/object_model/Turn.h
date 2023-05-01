#ifndef TURN_H
#define TURN_H

#include "object_model.h"
#include "ActorInfoContainer.h"

struct Turn : ITurn, C_Turn
{
  Turn();
  ~Turn();
  virtual ActorInfo * getSelfInfo();
  virtual IActorInfoIterator * getFriends();
  virtual IActorInfoIterator * getFoes();
  virtual void move(int x, int y);
  virtual void attack(int id);

  // All data members are owned and populated by BattleManager
  ActorInfo * self;
  ActorInfoContainer friends;
  ActorInfoContainer foes;
};

#endif
