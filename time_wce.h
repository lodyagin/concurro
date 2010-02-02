#pragma once

#include <windows.h>
#include <wchar.h>

/* tm <--> SYSTEMTIME */
SYSTEMTIME wce_tm2SYSTEMTIME(struct tm *t);
struct tm wce_SYSTEMTIME2tm(SYSTEMTIME *s);

