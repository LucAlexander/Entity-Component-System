#ifndef ECS_H
#define ECS_H

#include "Type-Safe-Vector/vector.h"
#include "Type-Safe-Hash-Map/hashMap.h"
#include "Type-Safe-Queue/queue.h"

VECTOR(Vu32, uint32_t)
VECTOR(Mu32, Vu32)
VECTOR(Vu64, uint64_t)
VECTOR(Mu64, Vu64)

//                   eid     , list of data indexes
HASHMAP(ArchIndexes, uint32_t, Vu32)

typedef struct Archetype{
	Vu32 cids;
	ArchIndexes data;
}Archetype;

Archetype ArchetypeInit();
void ArchetypeCopy(Archetype* ref, Archetype* copy);

uint8_t ArchetypeContainsComponent(Archetype* a, uint32_t cid);
uint8_t ArchetypeContainsEntity(Archetype* a, uint32_t eid);
int32_t ArchetypeGetComponentIndex(Archetype* a, uint32_t cid);
int32_t ArchetypeGetIndex(Archetype* a, uint32_t eid, uint32_t cid);
void ArchetypeRemoveCid(Archetype* newArchetype, uint32_t cid);

VECTOR(ArchetypeList, Archetype)

//                          eid     , archetype id
HASHMAP(EntityArchetypeMap, uint32_t, uint32_t)

//          eids
QUEUE(Qu32, uint32_t)

VECTOR(Vector, void*)
VECTOR(Matrix, Vector)

uint8_t maskContainsBit(Vu64* mask, uint32_t bit);
uint8_t maskCompare(Vu64* reference, Vu64* candidate);
uint8_t maskEquals(Vu64* a, Vu64* b);
void maskRemoveBit(Vu64* mask, uint32_t bit);
void maskAddBit(Vu64* mask, uint32_t bit);
void reduceMaskBitPair(uint32_t* index, uint32_t* pos);
Vu64 createMask(uint32_t n, ...);
Vu64 maskCopy(Vu64* mask);
void maskDisplay(Vu64* mask);
void maskChunkDisplay(uint64_t chunk);

typedef struct ComponentQuery{
	Vu32 entities;
	Mu32 indexes;
	Matrix components;
}ComponentQuery;

ComponentQuery* ecsQuery(Vu64* mask, Vu32* bits);
ComponentQuery ComponentQueryInit();
void queryPullArchetypeCid(Archetype* arc, uint32_t relIndex, uint32_t index, uint32_t* eids);
void queryPullARchetypeEids(uint32_t* eids, uint32_t size);
void queryScrubArchetype(Archetype* arc, Vu32* bits);
void clearComponentQuery();
void freeComponentQuery(ComponentQuery* q);
void displayComponentQuery();

typedef struct ECS{
	Matrix componentData;
	Mu32 componentOwner;
	ArchetypeList archetypes;
	Mu64 masks;
	Qu32 idBacklog;
	uint32_t maxId;
	EntityArchetypeMap entityLocation;
	ComponentQuery query;
}ECS;

void ecsInit(uint32_t componentCount);

uint32_t summon();
void smite(uint32_t eid);

void addEntityToEmptyListing(uint32_t eid);
void removeComponent(uint32_t eid, uint32_t cid);
void addComponent(uint32_t eid, uint32_t cid, void* data);
void replaceComponentData(Archetype* oldArc, uint32_t eid, uint32_t cid, void* data);
void placeIndexInCidOrder(Vu32* cids, Vu32* vec, uint32_t cid, uint32_t val);
void moveEntityDataAdditive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, void* data, uint32_t i);
void moveEntityDataDeductive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, uint32_t i);
void removeComponentData(Archetype* oldArc, Vu32* listing, uint32_t cid);
void updateMovedComponentIndex(uint32_t eid, uint32_t cid, uint32_t updatedIndex);

void* getComponent(uint32_t eid, uint32_t cid);

uint8_t containsComponent(uint32_t eid, uint32_t cid);

void freeEcs();
void freeComponentData(Matrix* vec);
void freeComponentList(Vector* vec);
void freeComponent(void* cmp);
void freeArchetypeList(ArchetypeList* list);
void freeArchetype(Archetype* arc);
void freeArchIndexes(ArchIndexes* a);
void freeMatrixu32(Mu32* m);
void freeMatrixu64(Mu64* m);

void ecsDisplay();

#endif
