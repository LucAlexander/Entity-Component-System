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
HASHMAP_SOURCE(EntityFlags, uint32_t, uint64_t, hashI)
QUEUE_SOURCE(Qu32, uint32_t)
VECTOR_SOURCE(Matrix, Cvector)
VECTOR_SOURCE(MatrixByPtr, Cvector*);

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
	if (chunk >= mask->size){
		return 0;
	}
	uint64_t seg = Vu64Get(mask, chunk);
	return (seg | (1<<pos)) == seg;
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
	while (chunk >= mask->size){
		Vu64PushBack(mask, 0);
	}
	uint64_t* seg = Vu64Ref(mask, chunk);
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
	if (chunk == 0){
		printf(" 0 ");
	}
	while (chunk > 0){
		printf("%lu",(chunk%2));
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

void ecsInit(uint32_t componentCount, ...){
	ecs.componentData = MatrixInit();
	ecs.componentOwner = Mu32Init();
	MatrixReserve(&(ecs.componentData), componentCount);
	Mu32Reserve(&(ecs.componentOwner), componentCount);
	uint32_t i;
	va_list vlist;
	va_start(vlist, componentCount);
	for (i = 0;i<componentCount;++i){
		size_t compSize = va_arg(vlist, size_t);
		Cvector newList = CvectorInit(compSize);
		MatrixPushBack(&(ecs.componentData), newList);
		Mu32PushBack(&(ecs.componentOwner), Vu32Init());
	}
	va_end(vlist);
	ecs.archetypes = ArchetypeListInit();
	ecs.masks = Mu64Init();
	ecs.idBacklog = Qu32Init();
	ecs.maxId = 0;
	ecs.flags = EntityFlagsInit();
	ecs.entityLocation = EntityArchetypeMapInit();
	Archetype empty = ArchetypeInit();
	ArchetypeListPushBack(&(ecs.archetypes), empty);
	Vu64 emptyMask = createMask(0);
	Mu64PushBack(&(ecs.masks), emptyMask);
	ecs.query = ComponentQueryInit();
}

void addEntityToEmptyListing(uint32_t eid){
	EntityArchetypeMapPush(&(ecs.entityLocation), eid, 0);
	EntityFlagsPush(&(ecs.flags), eid, 0);
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
}

void updateMovedComponentIndex(uint32_t eid, uint32_t cid, uint32_t updatedIndex){
	uint32_t archetypeIndex = EntityArchetypeMapGet(&(ecs.entityLocation), eid).val;
	if (archetypeIndex == 0){
		return;
	}
	Archetype* media = ArchetypeListRef(&(ecs.archetypes), archetypeIndex);	
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
	Vu32* indexes = ArchIndexesRef(&(arc->data), eid);
	uint32_t i, index;
	for (i = 0;i<arc->cids.size;++i){
		index = Vu32Get(indexes, i);
		uint32_t cid = Vu32Get(&(arc->cids), i);
		Cvector* components = MatrixRef(&(ecs.componentData), cid);
		CvectorRemove(components, index);
		Vu32* owners = Mu32Ref(&(ecs.componentOwner), cid);
		Vu32Remove(owners, index);
		uint32_t id = Vu32Get(owners, index);
		updateMovedComponentIndex(id, cid, index);
	}
	Vu32Free(indexes);
	ArchIndexesPop(&(arc->data), eid);
	EntityArchetypeMapPop(&(ecs.entityLocation), eid);
	EntityFlagsPop(&(ecs.flags), eid);
	Qu32Push(&(ecs.idBacklog), eid);
}

void removeComponentData(Archetype* oldArc, Vu32* listing, uint32_t cid){
	uint32_t k;
	for (k = 0;k<oldArc->cids.size;++k){
		if (Vu32Get(&(oldArc->cids), k)==cid){
			uint32_t index = Vu32Get(listing, k);
			Cvector* components = MatrixRef(&(ecs.componentData), cid);
			CvectorRemove(components, index);
			Vu32* owners = Mu32Ref(&(ecs.componentOwner), cid);
			Vu32Remove(owners, index);
			uint32_t id = Vu32Get(owners, index);
			if (components->size != index){
				updateMovedComponentIndex(id, cid, index);
			}
			Vu32RemoveInOrder(listing, k);
			return;
		}
	}
}

void moveEntityDataDeductive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, uint32_t i){
	ArchIndexesResult res = ArchIndexesPop(&(oldArc->data), eid);
	if (res.error!=0){
		return;
	}
	Vu32 listing = res.val;
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
		Vu64Free(&newMask);
		return;
	}
	maskRemoveBit(&newMask, cid);
	uint32_t i;
	for(i = 0;i<ecs.masks.size;++i){
		if (maskEquals(Mu64Ref(&(ecs.masks), i), &newMask)){
			Archetype* newArc = ArchetypeListRef(&(ecs.archetypes), i);
			moveEntityDataDeductive(oldArc, newArc, eid, cid, i);
			Vu64Free(&newMask);
			return;
		}
	}
	Archetype newArchetype = ArchetypeInit();
	ArchetypeCopy(oldArc, &newArchetype);
	ArchetypeRemoveCid(&newArchetype, cid);	
	Mu64PushBack(&(ecs.masks), newMask);
	moveEntityDataDeductive(oldArc, &newArchetype, eid, cid, i);
	ArchetypeListPushBack(&(ecs.archetypes), newArchetype);
}

void placeIndexInCidOrder(Vu32* cids, Vu32* vec, uint32_t cid, uint32_t val){
	Vu32Iterator it = Vu32IteratorInit(cids);
	uint32_t i = 0;
	while(Vu32IteratorHasNext(&it)){
		if (cid <= Vu32IteratorNext(&it)){
			Vu32Insert(vec, i, val);
			return;
		}
		i++;
	}
	Vu32Insert(vec, i, val);
}

void moveEntityDataAdditive(Archetype* oldArc, Archetype* newArc, uint32_t eid, uint32_t cid, void* data, uint32_t i){
	ArchIndexesPush(&(newArc->data), eid, ArchIndexesPop(&(oldArc->data), eid).val);
	*(EntityArchetypeMapRef(&(ecs.entityLocation), eid)) = i;
	Cvector* reference = MatrixRef(&(ecs.componentData), cid);
	placeIndexInCidOrder(&(newArc->cids), ArchIndexesRef(&(newArc->data), eid), cid, reference->size);
	CvectorPushBack(reference, data);
	Vu32PushBack(Mu32Ref(&(ecs.componentOwner), cid), eid);
}

void replaceComponentData(Archetype* oldArc, uint32_t eid, uint32_t cid, void* data){
	uint32_t index = ArchetypeGetIndex(oldArc, eid, cid);
	Cvector* componentList = MatrixRef(&(ecs.componentData), cid);
	CvectorSet(componentList, index, data);
}

void addComponent(uint32_t eid, uint32_t cid, void* data){
	EntityArchetypeMapResult res = EntityArchetypeMapGet(&(ecs.entityLocation), eid);
	if (res.error != 0){
		return;
	}
	Archetype* oldArc = ArchetypeListRef(&(ecs.archetypes), res.val);
	Vu64 newMask = maskCopy(Mu64Ref(&(ecs.masks), res.val));
	if (maskContainsBit(&newMask, cid)){
		replaceComponentData(oldArc, eid, cid, data);
		Vu64Free(&newMask);
		return;
	}
	maskAddBit(&newMask, cid);
	uint32_t i;
	for (i = 0;i<ecs.masks.size;++i){
		if (maskEquals(Mu64Ref(&(ecs.masks), i), &newMask)){
			Archetype* newArc = ArchetypeListRef(&(ecs.archetypes), i);
			moveEntityDataAdditive(oldArc, newArc, eid, cid, data, i);
			Vu64Free(&newMask);
			return;
		}
	}
	Archetype newArchetype = ArchetypeInit();
	ArchetypeCopy(oldArc, &newArchetype);
	Vu32PushInOrder(&(newArchetype.cids), cid, &u32Compare);
	Mu64PushBack(&(ecs.masks), newMask);
	moveEntityDataAdditive(oldArc, &newArchetype, eid, cid, data, i);
	ArchetypeListPushBack(&(ecs.archetypes), newArchetype);
}

void markForPurge(uint32_t eid){
	addEntityFlag(eid, ENTITY_DEACTIVATE);
}

void purgeDeactivatedData(){
	uint32_t* entities = EntityFlagsGetKeySet(&(ecs.flags));
	uint32_t i;
	for (i = 0;i<ecs.flags.size;++i){
		uint32_t eid = entities[i];
		uint64_t flag = EntityFlagsGet(&(ecs.flags), entities[i]).val;
		uint8_t modFlag = (1<<ENTITY_DEACTIVATE);
		if ((flag & modFlag) == modFlag){
			smite(eid);
		}
	}
	free(entities);
}

void addEntityFlag(uint32_t eid, uint64_t flagBit){
	uint64_t* flag = (EntityFlagsRef(&(ecs.flags), eid));
	if (flag == NULL){
		return;
	}
	*flag |= (1<<flagBit);
}

void removeEntityFlag(uint32_t eid, uint64_t flagBit){
	uint64_t* flag = (EntityFlagsRef(&(ecs.flags), eid));
	if (flag == NULL){
		return;
	}
	*flag &= ~(1<<flagBit);
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
	return CvectorGet(MatrixRef(&(ecs.componentData), cid), index);
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
	EntityFlagsFree(&(ecs.flags));
	freeComponentQuery(&(ecs.query));
}

void freeComponentData(Matrix* vec){
	uint32_t i;
	for (i = 0;i<vec->size;++i){
		CvectorFree(MatrixRef(vec, i));
	}
	MatrixFree(vec);
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
		Vu32* list = ArchIndexesRef(a, ArchIndexesIteratorNext(&it).key);
		Vu32Free(list);
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
	printf("_______________________________________________________________________________________________________________________________________________________________________\033[1;4mENTITY/COMPONENT DATA\033[0m\n");
	for (i = 0;i<ecs.componentData.size;++i){
		Cvector* sub = MatrixRef(&(ecs.componentData), i);
		Vu32* entities = Mu32Ref(&(ecs.componentOwner), i);
		printf("\033[1;31m%p\033[0m\033[1;33m\tCOMPONENT TYPE %u, %u components\033[0m\n", sub, i, sub->size);
		for (k = 0;k<sub->size;++k){
			uint32_t ent = Vu32Get(entities, k);
			printf("\033[1;31m%p\033[0m\t\t%u is owned by entity %u in archetype %u\n", CvectorGet(sub, k), k, ent, EntityArchetypeMapGet(&(ecs.entityLocation), ent).val);
		}
	}
	printf("_______________________________________________________________________________________________________________________________________________________________________\033[1;4mARCHETYPE LIST\033[0m\n");
	for (i = 0;i<ecs.archetypes.size;++i){
		Archetype* arc = ArchetypeListRef(&(ecs.archetypes), i);
		Vu64* mask = Mu64Ref(&(ecs.masks), i);
		printf("\033[1;31m%p\033[0m\t\033[1;33mArchetype id %u, Bit Mask ", arc, i);
		maskDisplay(mask);
		printf("\033[0m");
		ArchIndexesIterator it = ArchIndexesIteratorInit(&(arc->data));
		if (i==0){
			printf("\t\t");
			k = 0;
			while (ArchIndexesIteratorHasNext(&it)){
				printf("%u\t", ArchIndexesIteratorNext(&it).key);
				if ((++k)%24==0){
					printf("\n\t\t");
				}
			}
			printf("\n");
			continue;
		}
		printf("\033[1;31m%p\033[0m\t\033[1;33meid : [cid:index] : size: %u\033[0m\n", &(arc->data), arc->data.size);
		while (ArchIndexesIteratorHasNext(&it)){
			ArchIndexesResult res = ArchIndexesIteratorNext(&it);
			uint32_t key = res.key;
			Vu32 val = res.val;
			printf("\033[1;31m%p\033[0m\t\t%u\t:\t", ArchIndexesRef(&(arc->data), key), key);
			for (k = 0;k<val.size;++k){
				printf("[%u:%u]\t", Vu32Get(&(arc->cids), k), Vu32Get(&val, k));
			}
			printf("flags: %lu\n", EntityFlagsGet(&(ecs.flags), key).val);
		}
	}
	printf("_______________________________________________________________________________________________________________________________________________________________________\033[1;4mMAX ID: %u\033[0m\n", ecs.maxId);
}

ComponentQuery ComponentQueryInit(){
	ComponentQuery q;
	q.entities = Vu32Init();
	q.indexes = Mu32Init();
	return q;
}

ComponentQuery* ecsQuery(Vu64* mask, Vu32* bits, uint64_t filter, uint64_t magnet){
	clearComponentQuery();
	uint32_t i;
	for (i = 0;i<bits->size;++i){
		Vu32 placeholder = Vu32Init();
		Mu32PushBack(&(ecs.query.indexes), placeholder);
	}
	for (i = 0;i<ecs.masks.size;++i){
		Vu64 candidate = Mu64Get(&(ecs.masks), i);
		if(maskCompare(mask, &candidate)){
			queryScrubArchetype(ArchetypeListRef(&(ecs.archetypes), i), bits, filter, magnet);
		}
	}
	return &(ecs.query);
}

uint8_t queryFlagsLineUp(uint64_t targetFlag, uint64_t filter, uint64_t magnet){
	return ((((targetFlag & filter) != filter) || (filter==0)) && ((targetFlag & magnet ==magnet) || (magnet==0)));
}

void queryPullArchetypeCid(Archetype* arc, uint32_t relIndex, uint32_t index, uint32_t* eids, uint64_t filter, uint64_t magnet){
	Vu32* list = Mu32Ref(&(ecs.query.indexes), relIndex);
	uint32_t i;
	for (i = 0;i<arc->data.size;++i){
		uint32_t eid = eids[i];
		uint64_t targetFlag = EntityFlagsGet(&(ecs.flags), eid).val;
		if (queryFlagsLineUp(targetFlag, filter, magnet)){
			Vu32 indexes = ArchIndexesGet(&(arc->data), eid).val;
			Vu32PushBack(list, Vu32Get(&indexes, index));
		}
	}
}

void queryPullArchetypeEids(uint32_t* eids, uint32_t size, uint64_t filter, uint64_t magnet){
	uint32_t i;
	for (i = 0;i<size;++i){
		uint32_t eid = eids[i];
		uint64_t targetFlag = EntityFlagsGet(&(ecs.flags), eid).val;
		if (queryFlagsLineUp(targetFlag, filter, magnet)){
			Vu32PushBack(&(ecs.query.entities), eid);
		}
	}
}

void queryScrubArchetype(Archetype* arc, Vu32* bits, uint64_t filter, uint64_t magnet){
	uint32_t i;
	uint32_t k = 0;
	uint32_t currentBit = Vu32Get(bits, k++);
	uint32_t* eids = ArchIndexesGetKeySet(&(arc->data));
	queryPullArchetypeEids(eids, arc->data.size, filter, magnet);
	for (i = 0;i<arc->cids.size&&k<=bits->size;++i){
		if (currentBit == Vu32Get(&(arc->cids), i)){
			queryPullArchetypeCid(arc, k-1, i, eids, filter, magnet);
			currentBit = Vu32Get(bits, k++);
		}
	}
	free(eids);
	eids = NULL;
}

void clearComponentQuery(){
	freeComponentQuery(&(ecs.query));
	ecs.query = ComponentQueryInit();
}

void freeComponentQuery(ComponentQuery* q){
	Vu32Free(&(q->entities));
	freeMatrixu32(&(q->indexes));
}

void displayComponentQuery(){
	uint32_t i, k;
	printf("entities: \n");
	for (i = 0;i<ecs.query.entities.size;++i){
		printf("%u\t",Vu32Get(&(ecs.query.entities), i));
	}
	printf("\nindexes: \n");
	for (i = 0;i<ecs.query.indexes.size;++i){
		Vu32 sub = Mu32Get(&(ecs.query.indexes), i);
		for (k = 0;k<sub.size;++k){
			printf("%u\t",Vu32Get(&sub, k));
		}
		printf("\n");
	}
	printf("\n");
}

System SystemInit(void f(SysData*), uint32_t n, ...){
	System sys;
	sys.function = f;
	sys.mask = createMask(0);
	sys.bits = Vu32Init();
	sys.filter = 0;
	sys.magnet = 0;
	sys.components = MatrixByPtrInit();
	va_list v;
	va_start(v, n);
	uint32_t i, bit;
	for (i=0;i<n;++i){
		bit = va_arg(v, uint32_t);
		maskAddBit(&(sys.mask), bit);
		Vu32PushInOrder(&(sys.bits), bit, u32Compare);
	}
	va_end(v);
	for (i=0;i<n;++i){
		uint32_t index = Vu32Get(&(sys.bits), i);
		MatrixByPtrPushBack(&(sys.components), MatrixRef(&(ecs.componentData), index));
	}
	return sys;
}

void SystemAddFilter(System* sys, uint64_t flag){
	sys->filter |= (1<<flag);
}

void SystemRemoveFilter(System* sys, uint64_t flag){
	sys->filter &= ~(1<<flag);
}

void SystemAddMagnet(System* sys, uint64_t flag){
	sys->magnet |= (1<<flag);
}

void SystemRemoveMagnet(System* sys, uint64_t flag){
	sys->magnet &= ~(1<<flag);
}

void SystemActivate(System* sys){
	ComponentQuery* q = ecsQuery(&(sys->mask), &(sys->bits), sys->filter, sys->magnet);
	SysData data = SysDataInit(q);
	data.components = &(sys->components);
	while (SysDataHasData(&data)){
		sys->function(&data);
		SysDataIterate(&data);
	}
	SysDataFree(&data);
}

void SystemFree(System* sys){
	Vu64Free(&(sys->mask));
	Vu32Free(&(sys->bits));
	MatrixByPtrFree(&(sys->components));
}

SysData SysDataInit(ComponentQuery* q){
	SysData s;
	s.index = 0;
	s.query = q;
	s.indexes = malloc(sizeof(uint32_t)*q->entities.size);
	s.components = NULL;
	SysDataPopulateIndexes(&s);
	return s;
}

void SysDataPopulateIndexes(SysData* s){
	s->entity = Vu32Get(&(s->query->entities), s->index);
	uint32_t i;
	for (i=0;i<s->query->indexes.size;++i){
		s->indexes[i] = Vu32Get(Mu32Ref(&(s->query->indexes), i), s->index);
	}
}

uint8_t SysDataHasData(SysData* s){
	return s->index < s->query->entities.size;
}

void SysDataIterate(SysData* s){
	s->index++;
	if (SysDataHasData(s)){
		SysDataPopulateIndexes(s);
	}
}

void SysDataFree(SysData* s){
	free(s->indexes);
	s->indexes = NULL;
	s->components = NULL;
}

uint32_t entityArg(SysData* s){
	return s->entity;
}

void* componentArg(SysData* s, uint32_t component){
	return CvectorGet(MatrixByPtrGet(s->components, component), s->indexes[component]);
}

