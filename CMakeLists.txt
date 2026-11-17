cmake_minimum_required(VERSION 3.20)
project(DetectionMouvement LANGUAGES CXX C)

# Standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Inclusion du module FetchContent
include(FetchContent)

# -----------------------------------------------------------------------------
# Configuration minimale et statique d'OpenCV
# -----------------------------------------------------------------------------
# Compilation statique
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Désactivation des tests, exemples et applications annexes
set(BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(BUILD_PERF_TESTS OFF CACHE BOOL "Build perf tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_opencv_apps OFF CACHE BOOL "Build OpenCV applications" FORCE)

# Désactivation des liaisons vers d'autres langages
set(BUILD_opencv_python2 OFF CACHE BOOL "Build Python 2 bindings" FORCE)
set(BUILD_opencv_python3 OFF CACHE BOOL "Build Python 3 bindings" FORCE)
set(BUILD_opencv_python_bindings_generator OFF CACHE BOOL "Build Python bindings generator" FORCE)
set(BUILD_opencv_java OFF CACHE BOOL "Build Java bindings" FORCE)
set(BUILD_opencv_js OFF CACHE BOOL "Build JavaScript bindings" FORCE)

# Désactivation des accélérations matérielles lourdes
set(WITH_CUDA OFF CACHE BOOL "Disable CUDA" FORCE)
set(WITH_OPENCL OFF CACHE BOOL "Disable OpenCL" FORCE)
set(WITH_IPP OFF CACHE BOOL "Disable IPP" FORCE)

# Réduction des dépendances multimédias superflues
set(BUILD_DOCS OFF CACHE BOOL "Disable documentation" FORCE)
set(WITH_FFMPEG OFF CACHE BOOL "Disable FFmpeg" FORCE)
set(WITH_GSTREAMER OFF CACHE BOOL "Disable GStreamer" FORCE)
set(WITH_1394 OFF CACHE BOOL "Disable IEEE 1394" FORCE)
set(WITH_TIFF OFF CACHE BOOL "Disable TIFF" FORCE)
set(WITH_WEBP OFF CACHE BOOL "Disable WebP" FORCE)
set(WITH_OPENEXR OFF CACHE BOOL "Disable OpenEXR" FORCE)
set(WITH_OBSENSOR OFF CACHE BOOL "Disable Obsensor" FORCE)

# Intégration de Protobuf en interne pour le module dnn sans paquet système requis
set(BUILD_PROTOBUF ON CACHE BOOL "Build bundled Protobuf for dnn module" FORCE)

# Limitation stricte aux modules nécessaires
set(BUILD_LIST "core,imgproc,highgui,videoio,dnn" CACHE STRING "OpenCV modules to build" FORCE)

# -----------------------------------------------------------------------------
# Téléchargement d'OpenCV via FetchContent
# -----------------------------------------------------------------------------
FetchContent_Declare(
    opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG        4.8.0
    GIT_SHALLOW    TRUE
)

# Rendre OpenCV disponible dans le projet
FetchContent_MakeAvailable(opencv)

# -----------------------------------------------------------------------------
# Définition de l'exécutable DetectionMouvement
# -----------------------------------------------------------------------------
add_executable(DetectionMouvement main.cpp)

# Découverte automatique des dossiers d'en-tête des modules OpenCV
file(GLOB OPENCV_MODULE_INCLUDES "${opencv_SOURCE_DIR}/modules/*/include")

target_include_directories(DetectionMouvement PRIVATE
    ${CMAKE_BINARY_DIR}
    ${opencv_BINARY_DIR}
    ${opencv_SOURCE_DIR}/include
    ${OPENCV_MODULE_INCLUDES}
)

# Édition des liens avec les modules OpenCV requis
target_link_libraries(DetectionMouvement PRIVATE
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    opencv_highgui
    opencv_videoio
    opencv_dnn
)
