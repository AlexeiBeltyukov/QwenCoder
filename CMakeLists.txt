cmake_minimum_required(VERSION 3.16)

project(SimpleCalculator LANGUAGES CXX)

# ----------------------------------------------------------------------------
# Стандарт C++17
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# Автоматическое скачивание зависимостей (GLFW + Dear ImGui) через FetchContent
# ----------------------------------------------------------------------------
include(FetchContent)

# --- GLFW ---
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4                      # стабильный релиз
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)   # только статическая библиотека

# --- Dear ImGui (с бэкендами GLFW + OpenGL3) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.9-docking          # содержит нужные бэкенды GLFW/OpenGL3
)

FetchContent_MakeAvailable(glfw imgui)

# В репозитории ImGui нет CMakeLists.txt, поэтому собираем его как
# отдельную статическую библиотеку вручную.
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC glfw)

# ----------------------------------------------------------------------------
# Приложение калькулятора
# ----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui_lib
    glfw
)

# ----------------------------------------------------------------------------
# Системная OpenGL: под Windows MinGW предоставляет opengl32.lib
# ----------------------------------------------------------------------------
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 gdi32)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()

# ----------------------------------------------------------------------------
# Флаги для Windows / MinGW: полностью статическая линковка,
# чтобы готовый .exe запускался автономно без поиска внешних DLL
# (libgcc, libstdc++, winpthread и т.д.)
# ----------------------------------------------------------------------------
if (WIN32)
    if (MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
        )
    endif()
    # GUI-приложение: без лишнего окна консоли в Release-сборке
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()
