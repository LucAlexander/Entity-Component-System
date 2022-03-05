<h1 align="left">ECS Data Management Architecture</h1>
  <center><img align="center" width=55% height=50% src="https://github.com/LucAlexander/Entity-Component-System/blob/main/ECS.png"></img></center>
  <p>
    A data component system, or entity component system (ECS), is a data management system which aims to optimize for high program locality. The concept comes from the game development community, and has seen many different implementation standards, but its use cases reach far beyond this context. 
    This data component system is a general purpose implementation built for compatibility and speed. It is relatively lightweight and can handle a large data load, and should modularly fit into most C or C++ projects. 
  </p>
  <h2 align="left">Dependancies</h2>
  <p>
    This code base uses the Data Containers library listed on my github at <a href = "https://Github.com/LucAlexander/DataContainers/">this repository</a>, which includes a make target to build the static library. All code is compiled and tested on GNU-C 11.
  </p>
  <h2 align="left">Usage</h2>
    <h3 align="left">Build library from source</h3>
      <p>
        Use the build target <code>make build</code> to build the source code into a static library, you may then replace the <code>ecsmain.c</code> token within the make file with any source files you wish to link to the library, and run <code>make linkedCompile</code>
      </p>
    <h3 align="left">Buliding your own library</h3>
      <p>
        The provided Makefile contains reference to a sourcefile <code>ecsmain.c</code>, replace this with any code you want to be included in the build of your library using this project.
      </p>
<h1 align="left">Documentation</h1>
  <h2>Initialization</h2>
  <p>
    Initialize the data management system using <code>ecsInit(uint32_t componentCount, ...);</code>. The first argument is the number of component types you intend to use, followed by a variadic argument list of <code>size_t</code> component sizes.
  </p>
  <p>
    Example usage:<br>
    <code>
      ecsInit(4, <br>
      &emsp;sizeof(struct panel),<br>
      &emsp;sizeof(struct position),<br>
      &emsp;sizeof(struct pressable),<br>
      &emsp;sizeof(struct image)<br>
      );<br>
    </code>
  </p>
  <p>
    Note that the order of these is very important, as the order determines the id associated with every component type. It is reccommended that you keep track of these ids in some sort of enumerator or list structure.
  </p>
  <br>
  <h2>Data Entities</h2>
    <p>
      "Entities", or, the containers which store data components, are entirely conceptual. They exist simply as an unsigned 32 bit id. which is generated and registered by the data system. To generate one of these ids, call <code>uint32_t summon();</code>.
    </p>
    <p>
      Eample usage:<br>
      <code>
        uint32_t entity = summon();<br>
      </code>
    </p>
    <br>
    <p>
      To destroy the registered entity and all its components, as well as release the entity back to the management system so it acn be reused, call <code>void smite(uint32_t eid)</code>.
    </p>
    <br>
    <p>
      Example usage:<br>
      <code>
        uint32_t entity = summon();<br>
        smite(entity);<br>
      </code>
    </p>
    <br>
    <p>
      You can give entities flags, some flags are built in and are meant for specific purposes, but you can create your own flags. Currently the default enumerator which represents these flags exists as follows:<br>
      <code>
        enum ESC_DEFAULT_FLAGS{<br>
        &emsp;ENTITY_DEACTIVATE=0,<br>
        &emsp;RENDER_RELATIVE=1,<br>
        &emsp;RENDER_ABSOLUTE=2<br>
        };<br>
      </code>
      The first is for destroying entities. You'll find that destoying entities works best if they are marked and then destroye at the end of a frame, as it prevents confictions with referencing entities which were available for one sytem but not for another. The other two are placeholders for whatever graphical utility you choose to use. <code>RENDER_RELATIVE</code> implies that some image will be rendered relative to some in window view, while <code>RENDER_ABSOLUTE</code> implies that some image will be rendered with absolute positioning relative to the screen. You do not have to use these, but they are there for your conventience.
    </p>
    <p>
      Flags can be added and removed with <code>addEntityFlag(uint32_t eid, uint64_t flagBit);</code> and <code>removeEntitiyFlag(uint32_t eid, uint64_t flagBit);</code> respectively. <code>marForPurge(uint32_t eid);</code> is a short cut to apply the <code>ENTITY_DEACTIVATE</code> flag to an entity.
    </p>
    <p>
      Speaking of marking entities for destruction, <code>void purgeDeactivatedData();</code> will free and remove all entities marked with the <code>ENTITY_DEACTIVATE</code> flag.
    </p>
    <br>
  <h2>Data Components</h2>
    <p>
    Components much like entities are registered as an id. These ids are unique and it is reccommeded you keep track of which id corrosponds to which component, these ids are sequential integers starting at 0.
    </p>
    <p>
    A data component can be anything, from a simple integer type to a complex struct. It is reccommended that you keep these structures small though, so that as much relevant data can be stored contiguously.
    </p>
    <p>
    To register an instance of a component with an existing entity id, call <code>void addComponent(uint32_t eid, uint32_t cid, void\* data);</code>
    </p>
    <p>
    Example usage:<br>
    <code>
    ecsInit(1, sizeof(v2));
    uint32_t entity = summon():<br>
    v2 pos = {32, 48};<br>
    addComponent(entity, 0, &pos);<br>
    </code>
    </p>
    <p>
    Similarly, to remove an existing component from an existing entity id use <code>removeComponent(uint32_t eid, uint32_t cid);</code>
    </p>
    <p>
    Example usage:<br>
    <code>
    ecsInit(1, sizeof(v2));
    uint32_t entity = summon():<br>
    v2 pos = {32, 48};<br>
    addComponent(entity, 0, &pos);<br>
    removeComponent(entity, 0);
    </code>
    </p>
    <br>
    <p>
    You may find yourself needing to check whether or not some entity contains some other component. For this use <code>uint8_t containsComponent(uint32_t eid, uint32_t cid);</code> and subsequently you may find it useful to use <code>void\* getComponent(uint32_t eid, uint32_t cid);</code>.
    </p>
    <p>
    Example usage:<br>
    <code>
    if (containsComponent(save_button, pressable_component_id)){<br>
    &emsp;void\* pressable = getComponent(save_button, pressable_component_id);<br>
    }<br>
    </code>
    </p>
    <br>
  <h2>Logic Systems</h2>
    <p>
    Logic Systems are functions which operate on a subset of components. A system can be created with <code>System SystemInit(void sys(SysData\*), uint32_t n, ...);</code>. The first argument is a pointer to a function which takes a SysData pointer. The SysData struct is opaque, and is managed by the ECS, it contains all the relevant data for any given iteration over the subset of components for the logic system you are creating. The second argument is how many component types this system will operate on, followed by a variadic argument list of component ids.
    </p>
    <p>
    SysData provides two functions to query for information about the current iteration. <code>uint32_t entityArg(SysDate\* s);</code> gives the current entity id, and <code>void\* componentArg(SysDate\* s, uint32_t component);</code> which provides a pointer to one of the components querried for by the current iteration of the logic system. The second argument however is not a component id, but rather an indicator of which argument is being pulled, starting from 0, 1, 2, etc.
    </p>
    <p>
    Example usage:<br>
    <code>
    typedef enum COMPONENT_IDS{
    &emsp;POSITION_C,
    &emsp;FORCES_C
    }COMPONENT_IDS;
    void move(SysData\* sys){<br>
    &emsp;v2\* pos = componentArg(sys, 0);<br>
    &emsp;v2\* forces = componentArg(sys, 1);<br>
    &emsp;uint32_t entity = entityArg(sys);<br>
    &emsp;pos->x += forces->x;<br>
    &emsp;pos->y += forces->y;<br>
    }<br>
    <br>
    int main(void){<br>
    &emsp;System move_s = SystemInit(move, 2, POSITION_C, FORCES_C);<br>
    }<br>
    </code>
    Note that every system you create you must also manually free with <code>SystemFree(System\* sys)</code>
    </p>
    <br>
    <p>
    In order to querry the ESC for relevant data and run the internal logic function in a logic system, call <code>void SystemActivate(System\* sys);</code>
    </p>
    <br>
    <p>
    By nature, logic systems will collect all, and only all data relating to the list of component ids you pass it. You can also give them flags to filter by. passing an entity enumeration value like <code>ENTITY_DEACTIVATE</code> to <code>SystemAddFilter(System\* sys, uint64_t flag);</code> will tell the system to skip over any entity that has that flag.
    </p>
    <p>
    Similarly you can remove these flags by using <code>SystemRemoveFilter(System\* sys, uint64_t flag);</code>
    </p>
    <br>
   <h2>Freeing Memory</h2>
    <p>
    Systems need to be freed manually with <code>void SystemFree(System\* sys);</code>, and the ECS should be freed at program end as well; <code>freeEcs();</code>.
    </p>
    <p>
    If a component has heap allocated memory in it, that component must be given a custom function to free its memory. This is the responsibility of the user, as <code>void smite(uint32_t eid);</code>, <code>void purgeDeactivatedData();</code> etc will remove the reference to those pointers otherwise, causing an indirect memory leak.
    </p>
  <h2>Debug</h2>
    <p>
    <code>void ecsDisplay();</code> will show the current state of the data management system.
    </p>
    <p>
    <code>void displayComponentQuery();</code> will display the most recent data query as part of a <code>void SystemActivate(System\* sys);</code>
    </p>
