#ifndef C_OBJECT_MODEL
#define C_OBJECT_MODEL

#define MAX_STR 64 /* max string length of string fields */

typedef struct C_ActorInfo_
{
	int id;
	char   name[MAX_STR];
	int location_x;
	int location_y;
	int health;
	int attack;
	int defense;
	int damage;
	int movement;
} C_ActorInfo;

typedef struct C_ActorInfoIteratorHandle_
{
	char c;
} *C_ActorInfoIteratorHandle;

typedef struct C_ActorInfoIterator_
{
	void (*reset)(C_ActorInfoIteratorHandle handle);
	C_ActorInfo* (*next)(C_ActorInfoIteratorHandle handle);

	C_ActorInfoIteratorHandle handle;
} C_ActorInfoIterator;

typedef struct C_TurnHandle_
{
	char c;
} *C_TurnHandle;

typedef struct C_Turn_
{
	C_ActorInfo* (*getSelfInfo)(C_TurnHandle handle);
	C_ActorInfoIterator* (*getFriends)(C_TurnHandle handle);
	C_ActorInfoIterator* (*getFoes)(C_TurnHandle handle);

	void (*move)(C_TurnHandle handle, int x, int y);
	void (*attack)(C_TurnHandle handle, int id);

	C_TurnHandle handle;
} C_Turn;

typedef struct C_ActorHandle_
{
	char c;
} *C_ActorHandle;

typedef struct C_Actor_
{
	void (*getInitialInfo)(C_ActorHandle handle, C_ActorInfo* info);
	void (*play)(C_ActorHandle handle, C_Turn* turn);

	C_ActorHandle handle;
} C_Actor;

typedef struct LogParmas
{
	const char* filename;
	int       line;
	const char* message;
} LogParmas;

typedef struct ReportErrorParmas
{
	const char* filename;
	int       line;
	const char* message;
} ReportErrorParmas;

#endif
