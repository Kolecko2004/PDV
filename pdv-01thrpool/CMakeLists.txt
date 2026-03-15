cmake_minimum_required(VERSION 3.16)

# 1. Vynucení správného kompilátoru z Homebrew
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")

# 2. Definice projektu
project(pdv_threadpool LANGUAGES CXX)

# 3. Standard C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Cesty k hlavičkám a knihovnám z Homebrew
include_directories("/opt/homebrew/opt/llvm/include")
link_directories("/opt/homebrew/opt/llvm/lib")

# 5. Vynucení moderní knihovny libc++ (klíčové pro osyncstream)
add_compile_options(
        -stdlib=libc++
        -nostdinc++
        -I/opt/homebrew/opt/llvm/include/c++/v1
)

# 6. Nastavení linkeru, aby našel knihovny při spuštění
add_link_options(-Wl,-rpath,/opt/homebrew/opt/llvm/lib)

# 7. Najdi OpenMP (potřebuješ pro paralelní výpočty)
find_package(OpenMP REQUIRED)

# 8. Definice tvého spustitelného souboru (uprav název podle tvého projektu)
# Pokud máš více souborů, přidej je sem (např. src/utils.cpp)
add_executable(pdv_threadpool src/main.cpp)

# 9. Propojení s knihovnami
target_link_libraries(pdv_threadpool PRIVATE OpenMP::OpenMP_CXX)