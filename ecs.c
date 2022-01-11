#include <stdarg.h>
#include <stdio.h>

#include "ecs.h"

VECTOR_SOURCE(Vu32, uint32_t)
VECTOR_SOURCE(Mu32, Vu32)
VECTOR_SOURCE(Vu64, uint64_t)
VECTOR_SOURCE(Mu64, Vu64)
HASHMAP_SOURCE(ArchIndexes, uint32_t, Vu32, hashI)
VECTOR_SOURCE(ArchetypeList, Archetype)
HASHMAP_SOURCE(EntityArchetypeMap, uint32_t, uint32_t, hashI)
QUEUE_SOURCE(Qu32, uint32_t)
VECTOR_SOURCE(Vector, void*)
VECTOR_SOURCE(Matrix, Vector)

static ECS ecs = {0};

Archetype ArchetypeInit(){
	Archetype a;
	a.cids = Vu32Init();
	a.data = ArchIndexesInit();
	return a;
}

void ArchetypeCopy(Archetype* ref, Archetype* copy){
	uint32_t i;
	for (i = 0;i<ref->cids.size;++i){
		Vu32PushBack(&(copy->cids), Vu32Get(&(ref->cids), i));
	}
}

uint8_t ArchetypeContainsEntity(Archetype* a, uint32_t eid){
	return ArchIndexesRef(&(a->data), eid) != NULL;
}

uint8_t ArchetypeContainsComponent(Archetype* a, uint32_t cid){
	uint32_t i;
	for (i = 0;i<a->cids.size;++i){
		if (Vu32Get(&(a->cids), i)==cid){
			return 1;
		}
	}
	return 0;
}

int32_t ArchetypeGetComponentIndex(Archetype* a, uint32_t cid){
	uint32_t i;
	for (i = 0;i<a->cids.size;++i){
		if (Vu32Get(&(a->cids), i)==cid){
			return i;
		}
	}
	return -1;
}

int32_t ArchetypeGetIndex(Archetype* a, uint32_t eid, uint32_t cid){
	if (!ArchetypeContainsEntity(a, eid)){
		return -1;
	}
	int32_t index = ArchetypeGetComponentIndex(a, cid);
	if (index == -1){
		return -1;
	}
	return Vu32Get(ArchIndexesRef(&(a->data), eid), index);
}

void ArchetypeRemoveCid(Archetype* newArchetype, uint32_t cid){
	uint32_t j;
	for (j = 0;j<newArchetype->cids.size;++j){
		if (Vu32Get(&(newArchetype->cids), j)==cid){
			Vu32RemoveInOrder(&(newArchetype->cids), j);
			return;
		}
	}
}

uint8_t maskCompare(Vu64* reference, Vu64* candidate){
	uint32_t i;
	if (reference->size > candidate->size){
		return 0;
	}
	uint64_t referenceChunk, candidateChunk;
	for (i = 0;i<reference->size;++i){
		referenceChunk = Vu64Get(reference, i);
		candidateChunk = Vu64Get(candidate, i);
		if ((referenceChunk & candidateChunk) != referenceChunk){
			return 0;
		}
	}
	return 1;
}

uint8_t maskEquals(Vu64* a, Vu64* b){
	uint32_t i;
	if (a->size != b->size){
		return 0;
	}
	uint64_t referenceChunk, candidateChunk;
	for (i = 0;i<a->size;++i){
		referenceChunk = Vu64Get(a, i);
		candidateChunk = Vu64Get(b, i);
		if (referenceChunk!=candidateChunk){
			return 0;
		}
	}
	return 1;
}

uint8_t maskContainsBit(Vu64* mask, uint32_t bit){
	uint32_t chunk;
	uint32_t pos = bit;
	reduceMaskBitPair(&chunk, &pos);
	uint64_t seg = Vu64Get(mask, chunk);
	return (seg & (1<<pos)) == seg;
}

void maskRemoveBit(Vu64* mask, uint32_t bit){
	uint32_t chunk;
	reduceMaskBitPair(&chunk, &bit);
	uint64_t* seg = Vu64Ref(mask, chunk);
	*seg &= ~(1<<bit);
}

void maskAddBit(Vu64* mask, uint32_t bit){
	uint32_t chunk;
	reduceMaskBitPair(&chunk, &bit);
	uint64_t* seg = Vu64Ref(mask, chunk);
	if (seg == NULL){printf("ah\n");}
	*seg |= (1<<bit);
}

void reduceMaskBitPair(uint32_t* index, uint32_t* pos){
	*index = (*pos)/64;
	*pos %= 64;
}

Vu64 createMask(uint32_t n, ...){
	Vu32 bits = Vu32Init();
	Vu32Reserve(&bits, n);
	va_list v;
	va_start(v, n);
	uint32_t i;
	uint32_t maxBit = 0;
	for(i = 0;i<n;++i){
		uint32_t bit = va_arg(v, uint32_t);
		Vu32PushBack(&bits, bit);
		if (bit > maxBit){
			maxBit = bit;
		}
	}
	va_end(v);
	maxBit /= 64;
	uint64_t* tempMask = calloc(maxBit+1, sizeof(uint64_t));
	for (i = 0;i<bits.size;++i){
		uint32_t pos = Vu32Get(&bits, i);
		uint32_t index = 0;
		reduceMaskBitPair(&index, &pos);
		tempMask[index] |= 1<<pos;
	}
	Vu64 mask = Vu64Init();
	for(i = 0;i<maxBit+1;++i){
		Vu64PushBack(&mask, tempMask[i]);
	}
	Vu32Free(&bits);
	free(tempMask);
	tempMask = NULL;
	return mask;
}

Vu64 maskCopy(Vu64* mask){
	uint32_t i;
	Vu64 newMask = Vu64Init();
	for (i = 0;i<mask->size;++i){
		Vu64PushBack(&newMask, Vu64Get(mask, i));
	}
	return newMask;
}

void maskChunkDisplay(uint64_t chunk){
	uint32_t i;
	while (chunk > 0){
		printf("%u",(chunk%2));
		chunk /= 2;
	}
}

void maskDisplay(Vu64* mask){
	uint32_t i;
	for (i = 0;i<mask->size;++i){
		uint64_t chunk = Vu64Get(mask, i);
		maskChunkDisplay(chunk);
	}	
	printf("\n");
}

void ecsInit(uint32_t componentCount){
	ecs.componentData = MatrixInit();
	ecs.componentOwner = Mu32Init();
	MatrixReserve(&(ecs.componentData), componentCount);
	Mu32Reserve(&(ecs.componentOwner), componentCount);
	uint32_t i;
	for (i = 0;i<componentCount;++i){
		Vector newList = VectorInit();
		MatrixPushBack(&(ecs.componentData), newList);
		Mu32PushBack(&(ecs.componentOwner), Vu32Init());
	}
	ecs.archetypes = ArchetypeListInit();
	ecs.masks = Mu64Init();
	ecs.idBacklog = Qu32Init();
	ecs.maxId = 0;
	ecs.entityLocation = EntityArchetypeMapInit();
	Archetype empty = ArchetypeInit();
	ArchetypeListPushBack(&(ecs.archetypes), empty);
	Vu64 emptyMask = createMask(0);
	Mu64PushBack(&(ecs.masks), emptyMask);
}

void addEntityToEmptyListing(uint32_t eid){
	EntityArchetypeMapPush(&(ecs.entityLocation), eid, 0);
	Archetype* arc = ArchetypeListRef(&(ecs.archetypes), 0);
	ArchIndexesPush(&(arc->data), eid, Vu32Init());
}

uint32_t summon(){
	uint32_t id;
	if (ecs.idBacklog.size==0){
		id = ecs.maxId++;
		addEntityToEmptyListing(id);
		return id;
	}
	id = Qu32Pop(&(ecs.idBacklog));
	addEntityToEmptyListing(id);
	return id;
	//TODO  we may need a more efficient entity creation system
}

void updateMovedComponentIndex(uint32_t eid, uint32_t cid, uint32_t updatedIndex){
	uint32_t archetypeIndex = EntityArchetypeMapGet(&(ecs.entityLocation), eid).val;
	if (archetypeIndex == 0){
		return;
	}
	Archetype* media =  ArchetypeListRef(&(ecs.archetypes), archetypeIndex);	
	Vu32* indexes = ArchIndexesRef(&(media->data), eid);
	uint32_t i;
	for (i=0;i<indexes->size;++i){
		if (Vu32Get(&(media->cids), i)==cid){
			*Vu32Ref(indexes, i) = updatedIndex;
			return;
		}
	}
}

void smite(uint32_t eid){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		return;
	}
	Archetype* arc = ArchetypeListRef(&(ecs.archetypes), res.val);
	Vu32 indexes = ArchIndexesGet(&(arc->data), eid).val;
	uint32_t i, index;
	for (i = 0;i<arc->cids.size;++i){
		index = Vu32Get(&indexes, i);
		uint32_t cid = Vu32Get(&(arc->cids), i);
		Vector* components = MatrixRef(&(ecs.componentData), cid);
		void* component = VectorGet(components, index);
		VectorRemove(components, index);
		Vu32* owners = Mu32Ref(&(ecs.componentOwner), cid);
		Vu32Remove(owners, index);
		uint32_t id = Vu32Get(owners, index);
		updateMovedComponentIndex(id, cid, index);
		freeComponent(component);
	}
	ArchIndexesPop(&(arc->data), eid);
	Qu32Push(&(ecs.idBacklog), eid);
}

void removeComponentData(Archetype* oldArc, Vu32* listing, uint32_t cid){
	uint32_t k;
	for (k = 0;k<oldArc->cids.size;++k){
		if (Vu32Get(&(oldArc->cids), k)==cid){
			uint32_t index = Vu32Get(listing, k);
			Vector* components = MatrixRef(&(ecs.componentData), cid);
			void* component = VectorGet(components, index);
			VectorRemove(components, index);
			Vu32* owners = Mu32Ref(&(ecs.componentOwner), cid);
			Vu32Remove(owners, index);
			uint32_t id = Vu32Get(owners, index);
			updateMovedComponentIndex(id, cid, index);
			freeComponent(component);
			Vu32RemoveInOrder(listing, k);
			return;
		}
	}
}

void moveEntityDataDeductive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, uint32_t i){
	Vu32 listing = ArchIndexesPop(&(oldArc->data), eid).val;
	removeComponentData(oldArc, &listing, cid);
	ArchIndexesPush(&(newArc->data), eid, listing);
	*EntityArchetypeMapRef(&(ecs.entityLocation), eid) = i; 
}

void removeComponent(uint32_t eid, uint32_t cid){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		return;
	}
	Archetype* oldArc = ArchetypeListRef(&(ecs.archetypes), res.val);
	Vu64 newMask = maskCopy(Mu64Ref(&(ecs.masks), res.val));
	if (!maskContainsBit(&newMask, cid)){
		return;
	}
	maskRemoveBit(&newMask, cid);
	uint32_t i;
	for(i = 0;i<ecs.masks.size;++i){
		if (maskEquals(Mu64Ref(&(ecs.masks), i), &newMask)){
			Archetype* newArc = ArchetypeListRef(&(ecs.archetypes), i);
			moveEntityDataDeductive(oldArc, newArc, eid, cid, i);
			return;
		}
	}
	Archetype newArchetype = ArchetypeInit();
	ArchetypeCopy(oldArc, &newArchetype);
	ArchetypeRemoveCid(&newArchetype, cid);	
	Mu64PushBack(&(ecs.masks), newMask);
	ArchetypeListPushBack(&(ecs.archetypes), newArchetype);
	moveEntityDataDeductive(oldArc, &newArchetype, eid, cid, i);
}

void moveEntityDataAdditive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, void* data, uint32_t i){
	ArchIndexesPush(&(newArc->data), eid, ArchIndexesPop(&(oldArc->data), eid).val);
	*(EntityArchetypeMapRef(&(ecs.entityLocation), eid)) = i;
	if (cid > ecs.componentData.size){
		Vector cmpList = VectorInit();
		VectorPushBack(&(cmpList), data);
		Vu32 entList = Vu32Init();
		Vu32PushBack(&entList, eid);
		Mu32PushBack(&(ecs.componentOwner), entList);
		MatrixPushBack(&(ecs.componentData), cmpList);
		Vu32PushBack(ArchIndexesRef(&(newArc->data), eid), 0);
		return;
	}
	Vector* reference = MatrixRef(&(ecs.componentData), cid);
	Vu32PushBack(ArchIndexesRef(&(newArc->data), eid), reference->size);
	VectorPushBack(reference, data);
	Vu32PushBack(Mu32Ref(&(ecs.componentOwner), cid), eid);
}

void addComponent(uint32_t eid, uint32_t cid, void* data){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		printf("error: %u\n", res.error);
		return;
	}
	Archetype* oldArc = ArchetypeListRef(&(ecs.archetypes), res.val);
	Vu64 newMask = maskCopy(Mu64Ref(&(ecs.masks), res.val));
	maskAddBit(&newMask, cid);
	uint32_t i;
	for (i = 0;i<ecs.masks.size;++i){
		if (maskEquals(Mu64Ref(&(ecs.masks), i), &newMask)){
			Archetype* newArc = ArchetypeListRef(&(ecs.archetypes), i);
			moveEntityDataAdditive(oldArc, newArc, eid, cid, data, i);
			return;
		}
	}
	Archetype newArchetype = ArchetypeInit();
	ArchetypeCopy(oldArc, &newArchetype);
	Vu32PushBack(&(newArchetype.cids), cid);
	Mu64PushBack(&(ecs.masks), newMask);
	ArchetypeListPushBack(&(ecs.archetypes), newArchetype);
	moveEntityDataAdditive(oldArc, &newArchetype, eid, cid, data, i);
}

void* getComponent(uint32_t eid, uint32_t cid){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		return 0;
	}
	uint32_t arcIndex = res.val;
	Archetype* arc = ArchetypeListRef(&(ecs.archetypes), arcIndex);
	int32_t index = ArchetypeGetIndex(arc, eid, cid);
	if (index == -1){
		return NULL;
	}
	return VectorGet(MatrixRef(&(ecs.componentData), cid), index);
}

uint8_t containsComponent(uint32_t eid, uint32_t cid){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		return 0;
	}
	uint32_t arcIndex = res.val;
	return maskContainsBit(Mu64Ref(&(ecs.masks), arcIndex), cid);
}

void freeEcs(){
	freeComponentData(&(ecs.componentData));
	freeMatrixu32(&(ecs.componentOwner));
	freeMatrixu64(&(ecs.masks));
	freeArchetypeList(&(ecs.archetypes));
	Qu32Free(&(ecs.idBacklog));
	EntityArchetypeMapFree(&(ecs.entityLocation));
}

void freeComponentData(Matrix* vec){
	uint32_t i;
	for (i = 0;i<vec->size;++i){
		freeComponentList(MatrixRef(vec, i));
	}
	MatrixFree(vec);
}

void freeComponentList(Vector* vec){
	uint32_t i;
	for (i = 0;i<vec->size;++i){
		freeComponent(VectorGet(vec, i));
	}
	VectorFree(vec);
}

void freeComponent(void* cmp){
	free(cmp);
	cmp = NULL;
}

void freeArchetypeList(ArchetypeList* list){
	uint32_t i;
	for(i = 0;i<list->size;++i){
		freeArchetype(ArchetypeListRef(list, i));
	}
	ArchetypeListFree(list);
}

void freeArchIndexes(ArchIndexes* a){
	ArchIndexesIterator it = ArchIndexesIteratorInit(a);
	while (ArchIndexesIteratorHasNext(&it)){
		Vu32 list = ArchIndexesIteratorNext(&it).val;
		Vu32Free(&list);
	}
	ArchIndexesFree(a);
}

void freeArchetype(Archetype* arc){
	Vu32Free(&(arc->cids));
	freeArchIndexes(&(arc->data));
}

void freeMatrixu32(Mu32* m){
	uint32_t i;
	for (i = 0;i<m->size;++i){
		Vu32Free(Mu32Ref(m, i));
	}
	Mu32Free(m);
}

void freeMatrixu64(Mu64* m){
	uint32_t i;
	for (i = 0;i<m->size;++i){
		Vu64Free(Mu64Ref(m, i));
	}
	Mu64Free(m);
}

void ecsDisplay(){
	uint32_t i, k;
	printf("ENTITY/COMPONENT DATA\n");
	for (i = 0;i<ecs.componentData.size;++i){
		Vector* sub = MatrixRef(&(ecs.componentData), i);
		Vu32* entities = Mu32Ref(&(ecs.componentOwner), i);
		printf("\tCOMPONENT TYPE %u, %u components\n",i, sub->size);
		for (k = 0;k<sub->size;++k){
			uint32_t ent = Vu32Get(entities, k);
			printf("\t\t%p is owned by entity %u in archetype %u\n", VectorGet(sub, k), ent, EntityArchetypeMapGet(&(ecs.entityLocation), ent).val);
		}
	}
	printf("ARCHETYPE LIST\n");
	for (i = 0;i<ecs.archetypes.size;++i){
		Archetype* arc = ArchetypeListRef(&(ecs.archetypes), i);
		Vu64* mask = Mu64Ref(&(ecs.masks), i);
		printf("\tArchetype  id %u, bitmask ", i);
		maskDisplay(mask);
		ArchIndexesIterator it = ArchIndexesIteratorInit(&(arc->data));
		printf("\t\teid : [cid:index]\n");
		while (ArchIndexesIteratorHasNext(&it)){
			ArchIndexesResult res = ArchIndexesIteratorNext(&it);
			uint32_t key = res.key;
			Vu32 val = res.val;
			printf("\t\t%u : ", key);
			for (k = 0;k<val.size;++k){
				printf("[%u:%u] ", Vu32Get(&(arc->cids), k), Vu32Get(&val, k));
			}
			printf("\n");
		}
	}
	printf("MAX ID: %u\n", ecs.maxId);
}
