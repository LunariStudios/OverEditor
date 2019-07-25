# OverEditor
## An open source map editor for the game Overwatch by Blizzard Entertainment
**Beware, this is a W.I.P., experimental project. Nothing is stable for now, so expect bugs**

The editor uses Vulkan for rendering *(VulkanHPP)*, and the following libraries
* [CascLib](https://github.com/ladislav-zezula/CascLib): Handling Blizzard's binary format
* [entityx](https://github.com/alecthomas/entityx): ECS framework
* [glm](https://github.com/g-truc/glm): Math operations
* [glfw](https://github.com/glfw/glfw): Window creation
* [plog](https://github.com/SergiusTheBest/plog): Logging  

Officially supported Operating Systems are:
* Windows
* Linux
* Mac
## Building
Running the following command should download everything you need and compile the project. 
```
git clone https://github.com/LunariStudios/OverEditor && cd OverEditor && git submodule update --init && cmake -H. -Bcmake-build && cmake --build cmake-build
```
