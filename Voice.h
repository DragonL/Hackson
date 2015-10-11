#ifndef VOICE_H
#define VOICE_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/select.h>
#endif

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include "pocketsphinx.h"

void init_voice_ctrl();

void start_void_ctrl();


#endif // VOICE_H