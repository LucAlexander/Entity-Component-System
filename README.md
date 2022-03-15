# ECS Data Management Architecture
![](https://github.com/LucAlexander/Entity-Component-System/blob/main/ECS.png)

A data component system, or entity component system (ECS), is a data management system which aims to optimize for high program locality. The concept comes from the game development community, and has seen many different implementation standards, but its use cases reach far beyond this context. 
This data component system is a general purpose implementation built for compatibility and speed. It is relatively lightweight and can handle a large data load, and should modularly fit into most C or C++ projects. 

## Dependancies
This code base uses the Data Containers library listed on my github at [this repository](https://Github.com/LucAlexander/DataContainers/), which includes a make target to build the static library. All code is compiled and tested on GNU-C 11.

## Usage
### Build the library from source
Use the build target `make build` to build the source code into a static library, you may then replace the `ecsmain.c` token within the make file with any source files you wish to link to the library, and run `make linkedCompile`.

### Building your own library
The provided Makefile contains reference to a sourcefile `ecsmain.c`, replace this with any code you want to be included in the build of your library using this project.

# Documentation
## Initialization
Initialize the data management system using `ecsInit(uint32_t componentCount, ...);`. The first argument is the number of component types you intend to use, followed by a variadic argument list of `size_t` component sizes.
Example usage:
```
ecsInit(4,
	sizeof(struct panel),
	sizeof(struct position),
	sizeof(struct pressable),
	sizeof(struct image)
);
```

Note that the order of these is very important, as the order determines the id associated with every component type. It is reccommended that you keep track of these ids in some sort of enumerator or list structure.

## Date Entities
"Entities", or, the containers which store data components, are entirely conceptual. They exist simply as an unsigned 32 bit id. which is generated and registered by the data system. To generate one of these ids, call `uint32_t summon();`.

Eample usage:
```
uint32_t entity = summon();
```

To destroy the registered entity and all its components, as well as release the entity back to the management system so it acn be reused, call `void smite(uint32_t eid);`.

Example usage:
```
uint32_t entity = summon();
smite(entity);
```

You can give entities flags, some flags are built in and are meant for specific purposes, but you can create your own flags. Currently the default enumerator which represents these flags exists as follows:
```
enum ESC_DEFAULT_FLAGS{
	&emsp;ENTITY_DEACTIVATE=0,
	&emsp;RENDER_RELATIVE=1,
	&emsp;RENDER_ABSOLUTE=2
};
```

The first is for destroying entities. You'll find that destoying entities works best if they are marked and then destroye at the end of a frame, as it prevents confictions with referencing entities which were available for one sytem but not for another. The other two are placeholders for whatever graphical utility you choose to use. `RENDER_RELATIVE` implies that some image will be rendered relative to some in window view, while `RENDER_ABSOLUTE` implies that some image will be rendered with absolute positioning relative to the screen. You do not have to use these, but they are there for your conventience.

Flags can be added and removed with `addEntityFlag(uint32_t eid, uint64_t flagBit);` and `removeEntitiyFlag(uint32_t eid, uint64_t flagBit);` respectively. `markForPurge(uint32_t eid);` is a short cut to apply the `ENTITY_DEACTIVATE` flag to an entity.

Speaking of marking entities for destruction, `void purgeDeactivatedData();` will free and remove all entities marked with the `ENTITY_DEACTIVATE` flag.

## Data Components
Components much like entities are registered as an id. These ids are unique and it is reccommeded you keep track of which id corrosponds to which component, these ids are sequential integers starting at 0.

A data component can be anything, from a simple integer type to a complex struct. It is reccommended that you keep these structures small though, so that as much relevant data can be stored contiguously.

To register an instance of a component with an existing entity id, call `void addComponent(uint32_t eid, uint32_t cid, void* data);`

Example usage:
```
ecsInit(1, sizeof(v2));
uint32_t entity = summon();
v2 pos = {32, 48};
addComponent(entity, 0, &pos);
```

Similarly, to remove an existing component from an existing entity id use `removeComponent(uint32_t eid, uint32_t cid);`

Example usage:
```
ecsInit(1, sizeof(v2));
uint32_t entity = summon();
v2 pos = {32, 48};
addComponent(entity, 0, &pos);
removeComponent(entity, 0);
```

You may find yourself needing to check whether or not some entity contains some other component. For this use `uint8_t containsComponent(uint32_t eid, uint32_t cid);` and subsequently you may find it useful to use `void* getComponent(uint32_t eid, uint32_t cid);`.

Example usage:
```
if (containsComponent(save_button, pressable_component_id)){
	void* pressable = getComponent(save_button, pressable_component_id);
}
```

## Logic Systems
Logic Systems are functions which operate on a subset of components. A system can be created with `System SystemInit(void sys(SysData*), uint32_t n, ...);`. The first argument is a pointer to a function which takes a SysData pointer. The SysData struct is opaque, and is managed by the ECS, it contains all the relevant data for any given iteration over the subset of components for the logic system you are creating. The second argument is how many component types this system will operate on, followed by a variadic argument list of component ids.

SysData provides two functions to query for information about the current iteration. `uint32_t entityArg(SysDate* s);` gives the current entity id, and `void* componentArg(SysDate* s, uint32_t component);` which provides a pointer to one of the components querried for by the current iteration of the logic system. The second argument however is not a component id, but rather an indicator of which argument is being pulled, starting from 0, 1, 2

Example usage:
```
typedef enum COMPONENT_IDS{
	POSITION_C,
	FORCES_C
}COMPONENT_IDS;

void move(SysData* sys){
	v2* pos = componentArg(sys, 0);
	v2* forces = componentArg(sys, 1);
	uint32_t entity = entityArg(sys);
	pos->x += forces->x;
	pos->y += forces->y;
}

int main(void){
	System move_s = SystemInit(move, 2, POSITION_C, FORCES_C);
}
```

Note that every system you create you must also manually free with `SystemFree(System* sys);`

In order to querry the ESC for relevant data and run the internal logic function in a logic system, call `void SystemActivate(System* sys);`

By nature, logic systems will collect all, and only all data relating to the list of component ids you pass it. You can also give them flags to filter by. passing an entity enumeration value like `ENTITY_DEACTIVATE` to `void SystemAddFilter(System* sys, uint64_t flag);` will tell the system to skip over any entity that has that flag.
Similarly you can remove these flags by using `void SystemRemoveFilter(System* sys, uint64_t flag);`

In complement to system filters you can add a system magnet, which will make the logic system only operate of entities which have a certain flag. This is accomplished with `void SystemAddMagnet(System* sys, uint64_t flag);`, they can also be removed with `void SystemRemoveMagnet(System* sys, uint64_t flag);`

## Freeing Memory
Systems need to be freed manually with `void SystemFree(System* sys);`, and the ECS should be freed at program end as well; `freeEcs();`.

If a component has heap allocated memory in it, that component must be given a custom function to free its memory. This is the responsibility of the user, as `void smite(uint32_t eid);`, `void purgeDeactivatedData();` etc will remove the reference to those pointers otherwise, causing an indirect memory leak.

## Debug
`void ecsDisplay();` will show the current state of the data management system.

`void displayComponentQuery();` will display the most recent data query as part of a `void SystemActivate(System* sys);`
