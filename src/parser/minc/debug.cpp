/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  Created by Douglas Scott on 12/30/19.
//

#include "debug.h"

#if defined(DEBUG_TRACE)

char Trace::sMsgbuf[256];
int Trace::sTraceDepth = 0;
char Trace::spaces[MAX_SPACES+1];

#endif

