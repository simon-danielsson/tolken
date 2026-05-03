
// libraries
#include "./libs/ana.h"

// standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// diagnostics
#pragma GCC diagnostic ignored "-Wunused-variable"

// define environment variables with fallbacks
// (these are overridden and expanded at compile-time)
#ifndef ENV_NAME // project name
#define ENV_NAME "UNDEFINED"
#endif
#ifndef ENV_AUTHOR // author of this project
#define ENV_AUTHOR "UNDEFINED"
#endif
#ifndef ENV_CONTACT // contact info to author
#define ENV_CONTACT "UNDEFINED"
#endif
#ifndef ENV_GITHASH // git hash
#define ENV_GITHASH "UNDEFINED"
#endif
#ifndef ENV_GITTAG // git release version
#define ENV_GITTAG "UNDEFINED"
#endif
#ifndef ENV_REPO // link to git repo
#define ENV_REPO "UNDEFINED"
#endif

