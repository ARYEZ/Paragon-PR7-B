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

# AudioDSPTools: the official NAM plugin's resampler, used when the DAW's
# sample rate differs from the model's (MIT). Pinned to the commit
# NeuralAmpModelerCore v0.6.0 uses. Header-only for the parts we use.
FetchContent_Declare(audiodsptools
    GIT_REPOSITORY https://github.com/sdatkinson/AudioDSPTools.git
    GIT_TAG        0827c6c2fc0deced568536142ea86f189e0b98a1
    SOURCE_SUBDIR  no-cmake)

# SOURCE_SUBDIR points at a folder that doesn't exist, which tells CMake to
# download NAM, Eigen and AudioDSPTools without running their own CMake projects (NAM's
# builds command-line benchmark tools we don't need).
FetchContent_MakeAvailable(juce nam eigen audiodsptools)

file(GLOB NAM_SOURCES CONFIGURE_DEPENDS
    "${nam_SOURCE_DIR}/NAM/*.cpp"
    "${nam_SOURCE_DIR}/NAM/*/*.cpp")

# Headers and defines for code that uses NAM. Kept separate from nam_core:
# nam_core is linked with WHOLE_ARCHIVE (see the root CMakeLists.txt), which
# strips the SYSTEM flag from include paths and would flood the build with
# warnings from Eigen and the resampler.
add_library(nam_headers INTERFACE)
target_compile_features(nam_headers INTERFACE cxx_std_20)
target_include_directories(nam_headers SYSTEM INTERFACE
    "${nam_SOURCE_DIR}"
    "${nam_SOURCE_DIR}/NAM"
    "${eigen_SOURCE_DIR}"
    "${nam_SOURCE_DIR}/Dependencies/nlohmann"
    "${audiodsptools_SOURCE_DIR}/dsp")
target_compile_definitions(nam_headers INTERFACE
    NAM_SAMPLE_FLOAT        # JUCE buffers are float, so NAM runs on float with no conversion
    NAM_ENABLE_A2_FAST      # matches NAM's own default build: optimised path for A2 models
    $<$<PLATFORM_ID:Windows>:NOMINMAX WIN32_LEAN_AND_MEAN>)

add_library(nam_core STATIC ${NAM_SOURCES})
target_link_libraries(nam_core PRIVATE nam_headers)
# The VST3 is a shared library, so code linked into it must be position independent on Linux.
set_target_properties(nam_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(nam_core PRIVATE /bigobj)
endif()
