# Third-party dependencies, pinned to exact versions so every machine and CI
# runner builds against identical code.

include(FetchContent)

# JUCE: plugin framework (AGPLv3 for open-source projects).
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE)

# NeuralAmpModelerCore: the official engine that runs .nam models (MIT).
# Its Eigen submodule is fetched separately below, and its other submodule
# (AudioDSPTools) is only used by NAM's own command-line tools.
FetchContent_Declare(nam
    GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG        v0.6.0
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES ""
    SOURCE_SUBDIR  no-cmake)   # download only: we compile NAM's sources ourselves below

# Eigen: linear algebra library NAM is built on. This is the exact commit
# NeuralAmpModelerCore v0.6.0 pins as its submodule (Eigen 5.0.1).
FetchContent_Declare(eigen
    URL      https://gitlab.com/libeigen/eigen/-/archive/bc3b39870ecb690a623a3f49149a358b95c5781d/eigen-bc3b39870ecb690a623a3f49149a358b95c5781d.tar.gz
    URL_HASH SHA256=5f6557d1aca6e959a4b955c55f2935261fd30de59c0c79e35bc6d5cbe1de22d1
    SOURCE_SUBDIR no-cmake)    # header-only for our purposes: download, don't configure

# SOURCE_SUBDIR points at a folder that doesn't exist, which tells CMake to
# download NAM and Eigen without running their own CMake projects (NAM's
# builds command-line benchmark tools we don't need).
FetchContent_MakeAvailable(juce nam eigen)

file(GLOB NAM_SOURCES CONFIGURE_DEPENDS
    "${nam_SOURCE_DIR}/NAM/*.cpp"
    "${nam_SOURCE_DIR}/NAM/*/*.cpp")

add_library(nam_core STATIC ${NAM_SOURCES})
target_compile_features(nam_core PUBLIC cxx_std_20)
target_include_directories(nam_core
    PUBLIC
        "${nam_SOURCE_DIR}"
        "${nam_SOURCE_DIR}/NAM"
    SYSTEM PUBLIC
        "${eigen_SOURCE_DIR}"
        "${nam_SOURCE_DIR}/Dependencies/nlohmann")
target_compile_definitions(nam_core
    PUBLIC
        NAM_SAMPLE_FLOAT        # JUCE buffers are float, so NAM runs on float with no conversion
        NAM_ENABLE_A2_FAST      # matches NAM's own default build: optimised path for A2 models
        $<$<PLATFORM_ID:Windows>:NOMINMAX WIN32_LEAN_AND_MEAN>)
# The VST3 is a shared library, so code linked into it must be position independent on Linux.
set_target_properties(nam_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(nam_core PRIVATE /bigobj)
endif()
