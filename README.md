<h1 align="center">ECS Data Management Architecture</h1>
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
<p>
To destroy the registered entity and all its components, as well as release the entity back to the management system so it acn be reused, call <code>void smite(uint32_t eid)</code>.
</p>
<p>
Example usage:<br>
<code>
uint32_t entity = summon();<br>
smite(entity);<br>
</code>
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
<h2>Systems</h2>
<h2>Freeing Memory</h2>
