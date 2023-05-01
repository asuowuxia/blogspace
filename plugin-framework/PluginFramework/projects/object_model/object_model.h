#ifndef OBJECT_MODEL
#define OBJECT_MODEL

#include "c_object_model.h"

typedef C_ActorInfo ActorInfo;

struct IActorInfoIterator
{
  virtual void reset() = 0;
  virtual ActorInfo * next() = 0;
};

struct ITurn
{
  virtual ActorInfo * getSelfInfo() = 0;
  virtual IActorInfoIterator * getFriends() = 0;
  virtual IActorInfoIterator * getFoes() = 0;

  virtual void move(int x, int y) = 0;
  virtual void attack(int id) = 0;
};

struct IActor
{
  virtual void getInitialInfo(ActorInfo * info) = 0;
  virtual void play(ITurn * turnInfo) = 0;
};

#endif
