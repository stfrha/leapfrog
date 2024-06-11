# leapfrog
Sandbox space adventure game

Note that the SDL2 is a fork of the original libsdl-org/SDL. As of 2024-06-09, this is the latest SDL version. This should be updated when SDL is updated.
When building the leapfrog game solution, some libraries are also built (including SDL2). A post-build script is executed to copy the necessary dll-files to the data-folder of the game. The game must use the data folder as its working directory. I use a own oxygine project that uses the new custom built libjpeg and libpng as static libs (rather than dlls). That way they are built using the same VS version as oxygine and the game. zlib is built as a dll but it is also built using the same VS version. 

For windows:

The following Repositories are required:
https://github.com/stfrha/leapfrog
https://github.com/stfrha/oxygine-framework
https://github.com/stfrha/SDL.git using branch sdl2
https://github.com/stfrha/Box2D.git 
https://github.com/stfrha/lua.git
https://github.com/stfrha/libjpeg
https://github.com/stfrha/libpng
https://github.com/stfrha/zlib
https://github.com/stfrha/MultiSelectTreeView.git 
https://github.com/stfrha/AdvancedZoomAndPanSample.git 
https://github.com/stfrha/MicroMvvm.git 


For Android:

https://github.com/stfrha/leapfrog
https://github.com/stfrha/oxygine-framework
https://github.com/stfrha/SDL.git using branch sdl2
https://github.com/stfrha/Box2D.git 
https://github.com/stfrha/lua.git


For MacOsX:

https://github.com/stfrha/leapfrog
https://github.com/stfrha/oxygine-framework
https://github.com/stfrha/SDL.git using branch sdl2
https://github.com/stfrha/Box2D.git 
https://github.com/stfrha/lua.git


For iOS:


For Linux:
