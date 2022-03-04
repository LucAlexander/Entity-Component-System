<style>
code {
  font-family: Consolas,"courier new";
  color: mintcream;
  background-color: #1b1c19;
  padding: 2px;
  font-size: 105%;
}
</style>
<h1 align="center">ECS Data Management Architecture</h1>
	<p>
		A data component system, or entity component system (ECS), is a data management system which aims to optimize for high program locality. The concept comes from the game development community, and has seen many different implementation standards, but its use cases reach far beyond this context. 
		This data component system is a general purpose implementation built for compatibility and speed. It is relatively lightweight and can handle a large data load, and should modularly fit into most C or C++ projects. 
	</p>
	<h2 align="left">Dependancies</h2>
	<p>
		This code base uses the Data Containers library listed on my github at <a href = "https://Github.com/LucAlexander/DataContainers/">this repository</a>, which includes a make target to build the static library. All code is compiled and tested on GNU-C 11.
	</p>
	<h2 align="left">Usage</h2>
		<h3 align="center">Build library from source</h3>
		<p>
			Use the build target <code>make build</code> to build the source code into a static library, you may then replace the <code>ecsmain.c</code> token within the make file with any source files you wish to link to the library, and run <code>make linkedCompile</code>
		</p>
		<h3 align="center">Buliding your own library</h3>
		<p>
			The provided Makefile contains reference to a sourcefile <code>ecsmain.c</code>, replace this with any code you want to be included in the build of your library using this project.
		</p>
	<h2 align="left">Documentation</h2>
 		<p>pending</p>
