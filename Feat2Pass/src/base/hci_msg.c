
/* ====================================================================
 * Copyright (c) 2007 HCI LAB. 
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are prohibited provided that permissions by HCI LAB
 * are not given.
 *
 * ====================================================================
 *
 */

/**
 *	@file	hci_msg.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Package for checking and catching common errors, printing out errors nicely.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32_WCE
#if ((! WIN32) && (! _SGI_SOURCE))
#include <sys/errno.h>
#else
#include <errno.h>
#endif
#endif	// #ifndef _WIN32_WCE
#if (!defined(_WIN32) && !defined(_WIN32_WCE))
#include <unistd.h> /* dup2() */
#endif

#include "base/hci_msg.h"

static FILE *logfp = 0;

#if defined(_WIN32_WCE) || defined(GNUWINCE)

#include <windows.h>
#include "base/hci_malloc.h"

HCILAB_PRIVATE int
cst_verrmsg(const char *fmt,
			va_list args)
{
    WCHAR wmsg[256];
    WCHAR *wfmt;
    size_t len;

    len = mbstowcs(NULL,fmt,0) + 1;
    wfmt = hci_calloc(len,sizeof(*wfmt));
    mbstowcs(wfmt,fmt,len);

    va_start(args,fmt);
    _vsnwprintf(wmsg,256,wfmt,args);
    va_end(args);

    wmsg[255]=L'\0';
    hci_free(wfmt);
    return 0;
}

HCILAB_PRIVATE int
cst_errmsg(const char *fmt, ...)
{
    va_list args;

    va_start(args,fmt);
    cst_verrmsg(fmt, args);
    va_end(args);
    return 0;
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info_header_wofn(char const *msg)
{
    cst_errmsg("%s:\t", msg);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_header(char const *f,
			  long ln,
			  char const *msg)
{
    cst_errmsg("%s: \"%s\", line %ld: ", msg, f, ln);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info_header(char const *f,
				   long ln,
				   char const *msg)
{
    cst_errmsg("%s: %s(%ld): ", msg, f, ln);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_warn(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__die_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);
    exit(-1);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__fatal_sys_error(char const *fmt, ...)
{
    LPVOID msg_buf;
    DWORD error;
    va_list pvar;

    error = GetLastError();
    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);

    OutputDebugStringW(L"; ");
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error,
                  0, // Default language
                  (LPTSTR) &msg_buf,
                  0,
                  NULL);
    OutputDebugString(msg_buf);
    LocalFree(msg_buf);

    exit(error);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__sys_error(char const *fmt, ...)
{
    LPVOID msg_buf;
    DWORD error;
    va_list pvar;

    error = GetLastError();
    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);

    OutputDebugStringW(L"; ");
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error,
                  0, // Default language
                  (LPTSTR) &msg_buf,
                  0,
                  NULL);
    OutputDebugString(msg_buf);
    LocalFree(msg_buf);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__abort_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    cst_verrmsg(fmt, pvar);
    va_end(pvar);

}

#else

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info_header_wofn(char const *msg)
{
    (void) fflush(stderr);

    /* make different format so as not to be parsed by emacs compile */
    (void) fprintf(stderr, "%s:\t", msg);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_header(char const *f,
			  long ln,
			  char const *msg)
{
    (void) fflush(stderr);
    (void) fprintf(stderr, "%s: \"%s\", line %ld: ", msg, f, ln);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info_header(char const *f,
				   long ln,
				   char const *msg)
{
    (void) fflush(stderr);

    /* make different format so as not to be parsed by emacs compile */
    (void) fprintf(stderr, "%s: %s(%ld): ", msg, f, ln);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_warn(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stderr);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__pr_info(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stderr);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__die_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);

    (void) vfprintf(stderr, fmt, pvar);
    (void) fflush(stderr);

    va_end(pvar);

    (void) fflush(stderr);

    exit(-1);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__fatal_sys_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    putc(';', stderr);
    putc(' ', stderr);

    perror("");

    (void) fflush(stderr);

    exit(errno);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__sys_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    putc(';', stderr);
    putc(' ', stderr);

    perror("");

    (void) fflush(stderr);
}

HCILAB_PUBLIC HCI_BASE_API void
_E__abort_error(char const *fmt, ...)
{
    va_list pvar;

    va_start(pvar, fmt);
    (void) vfprintf(stderr, fmt, pvar);
    va_end(pvar);

    (void) fflush(stderr);

    abort();
}

#endif	// #if defined(_WIN32_WCE) || defined(GNUWINCE)

/** variables that allow redirecting all files to a log file */
#ifndef _WIN32_WCE /* FIXME: this is all BOGUS BOGUS BOGUS */
static FILE orig_stdout, orig_stderr;
#endif

/**
 * set logging file & open log-file
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_setLogFile(char const *file)
{
    FILE *fp = 0;

    if ((fp = fopen(file, "w")) == NULL) {
        HCIMSG_ERROR("fopen(%s,w) failed; logfile unchanged\n", file);
        return -1;
    }
    else {
        if (logfp)
            fclose(logfp);

        logfp = fp;
        /* 
         * Rolled back the dup2() bug fix for windows only. In
         * Microsoft Visual C, dup2 seems to cause problems in some
         * applications: the files are opened, but nothing is written
         * to it.
         */
#if defined(_WIN32) || defined(GNUWINCE)
#ifndef _WIN32_WCE /* FIXME: Possible? */
		orig_stdout = *stdout;      /* Hack!! To avoid hanging problem under Linux */
		orig_stderr = *stderr;      /* Hack!! To avoid hanging problem under Linux */
        *stdout = *logfp;
        *stderr = *logfp;
#endif
#else
        dup2(fileno(logfp), 1);
        dup2(fileno(logfp), 2);
#endif

    }

    return 0;
}

/**
 * close log-file
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_closeLogFile()
{
    if (logfp) {
        fclose(logfp);
#ifndef _WIN32_WCE
        *stdout = orig_stdout;
        *stderr = orig_stderr;
#endif
    }
}

// end of file
