/**
 * @file suppress_gui.h
 * @brief Utilities to suppress interactive GUI popups on Windows / MSVC / Wine.
 */

#ifndef C_CDD_SUPPRESS_GUI_H
#define C_CDD_SUPPRESS_GUI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "cdd_c_error.h"
#if defined(_WIN32)
#include <winsock2.h>
#endif
#if defined(_MSC_VER)
#include <crtdbg.h>
#include <stdlib.h>
#endif
/* clang-format on */

/**
 * @brief Suppress interactive GUI popups and modal dialogs on Windows/MSVC.
 * @return CDD_C_SUCCESS on success.
 */
static __inline cdd_c_error_t cdd_suppress_gui_popups(void) {
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
               SEM_NOOPENFILEERRORBOX);
#endif
#if defined(_MSC_VER)
  if (!GetProcAddress(GetModuleHandleA("ntdll.dll"), "wine_get_version")) {
#if defined(_DEBUG)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
  }
#endif
  return CDD_C_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_SUPPRESS_GUI_H */
