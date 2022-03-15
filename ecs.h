#ifndef ECS_H
#define ECS_H

#include "../DataContainers/src/vector/vector.h"
#include "../DataContainers/src/hashMap/hashMap.h"
#include "../DataContainers/src/queue/queue.h"

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
HASHMAP(EntityFlags, uint32_t, uint64_t)

//          eids
QUEUE(Qu32, uint32_t)

VECTOR(Matrix, Cvector)
VECTOR(MatrixByPtr, Cvector*)

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
}ComponentQuery;

ComponentQuery* ecsQuery(Vu64* mask, Vu32* bits, uint64_t filter, uint64_t magnet);
ComponentQuery ComponentQueryInit();
void queryPullArchetypeCid(Archetype* arc, uint32_t relIndex, uint32_t index, uint32_t* eids, uint64_t filter, uint64_t magnet);
void queryPullArchetypeEids(uint32_t* eids, uint32_t size, uint64_t filter, uint64_t magnet);
void queryScrubArchetype(Archetype* arc, Vu32* bits, uint64_t filter, uint64_t magnet);
void clearComponentQuery();
void freeComponentQuery(ComponentQuery* q);
void displayComponentQuery();

uint8_t queryFlagsLineUp(uint64_t target, uint64_t filter, uint64_t magnet);

typedef struct SysData{
	uint32_t entity;
	uint32_t index;
	uint32_t* indexes;
	ComponentQuery* query;
	MatrixByPtr* components;
}SysData;

SysData SysDataInit(ComponentQuery* q);
uint8_t SysDataHasData(SysData* s);
void SysDataPopulateIndexes(SysData* s);
void SysDataIterate(SysData* s);
void SysDataFree(SysData* s);

uint32_t entityArg(SysData* s);
void* componentArg(SysData* s, uint32_t component);

typedef struct ECS{
	Matrix componentData;
	Mu32 componentOwner;
	ArchetypeList archetypes;
	Mu64 masks;
	Qu32 idBacklog;
	uint32_t maxId;
	EntityArchetypeMap entityLocation;
	EntityFlags flags;
	ComponentQuery query;
}ECS;

void ecsInit(uint32_t componentCount, ...);

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

void markForPurge(uint32_t eid);
void purgeDeactivatedData();

void addEntityFlag(uint32_t eid, uint64_t flagBit);
void removeEntityFlag(uint32_t eid, uint64_t flagBit);

void* getComponent(uint32_t eid, uint32_t cid);

uint8_t containsComponent(uint32_t eid, uint32_t cid);

void freeEcs();
void freeComponentData(Matrix* vec);
void freeArchetypeList(ArchetypeList* list);
void freeArchetype(Archetype* arc);
void freeArchIndexes(ArchIndexes* a);
void freeMatrixu32(Mu32* m);
void freeMatrixu64(Mu64* m);

void ecsDisplay();

enum ESC_DEFAULT_FLAGS{
	ENTITY_DEACTIVATE=0,
	RENDER_RELATIVE=1,
	RENDER_ABSOLUTE=2
};

typedef struct System{
	Vu64 mask;
	Vu32 bits;
	uint64_t filter;
	uint64_t magnet;
	void (*function)(SysData* sys);
	MatrixByPtr components;
}System;

System SystemInit(void sys(SysData*), uint32_t n, ...);
void SystemAddMagnet(System* sys, uint64_t flag);
void SystemRemoveMagnet(System* sys, uint64_t flag);
void SystemAddFilter(System* sys, uint64_t flag);
void SystemRemoveFilter(System* sys, uint64_t flag);
void SystemActivate(System* sys);
void SystemFree(System* sys);

#endif
