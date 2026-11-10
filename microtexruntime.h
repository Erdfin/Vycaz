#pragma once

/// Loads MicroTeX's fonts/XML tables once, using the resource directory
/// copied next to the executable by CMake.
bool ensureMicroTeXInitialized();
