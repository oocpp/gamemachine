﻿add_subdirectory(./3rdparty/zlib zlib)
add_subdirectory(./3rdparty/glew-2.1.0 glew)
add_subdirectory(./3rdparty/freetype-2.8.1 freetype)
add_subdirectory(./3rdparty/libpng libpng)
add_subdirectory(./3rdparty/libmad libmad)
add_subdirectory(./3rdparty/jpeg-6b libjpeg)
add_subdirectory(./3rdparty/lua lua)
add_subdirectory(./3rdparty/glm-0.9.9-a2 glm)
add_subdirectory(./3rdparty/openal-soft-1.18.2 openal)
add_subdirectory(./3rdparty/bullet3-2.87 bullet3)

add_subdirectory(gamemachine gamemachine)
add_subdirectory(gamemachinemedia gamemachinemedia)

if (GM_BUILD_DEMO)
	add_subdirectory(gamemachinedemo gamemachinedemo)
endif(GM_BUILD_DEMO)

if (GM_BUILD_UNITTEST)
	add_subdirectory(gamemachineunittest gamemachineunittest)
endif(GM_BUILD_UNITTEST)