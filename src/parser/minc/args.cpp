/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  Created by Douglas Scott on 7/31/16.

#include <map>
#include <string.h>
#include "minc_internal.h"

extern "C" {
	int check_new_arg(const char *argument);
	const char *lookup_token(const char *token, bool printWarning);
}

static std::map<int, const char *> sTokenMap;

#define MAX_TOKEN_LEN 32

int
check_new_arg(const char *argument)
{
	if (argument[0] == '-' && argument[1] == '-') {		// looking for --foobar=3, etc.
		char tokenbuf[MAX_TOKEN_LEN];
		const char *rawtoken = argument+2;
		const char *token = NULL;
		if (strlen(rawtoken) > 1) {
			// copy so we can add null after token string
			strncpy(tokenbuf, rawtoken, MAX_TOKEN_LEN);
			char *end = strchr(tokenbuf, '=');
			if (end) {
				*end = '\0';
				token = tokenbuf;
			}
			else {
				rtcmix_warn(NULL, "Argument '--%s' is malformed - ignoring", rawtoken);
				return 1;
			}
		}
		else {
			rtcmix_warn(NULL, "Argument '--%s' is malformed - ignoring", rawtoken);
			return 1;
		}
		const char *value = strchr(rawtoken, '=') + 1;
		if (value && strlen(value) > 0) {
			std::pair<std::map<int, const char *>::iterator, bool> ret;
			ret = sTokenMap.insert(std::pair<int, const char *>(hash(token), value));
			if (ret.second == false) {
				rtcmix_warn(NULL, "Argument '--%s=<value>' was passed to CMIX more than once -- ignoring", token);
			}
		}
		else {
			rtcmix_warn(NULL, "Argument '--%s' is malformed - ignoring", rawtoken);
		}
		return 1;
	}
	return 0;
}

const char *
lookup_token(const char *token, bool printWarning)
{
	std::map<int, const char *>::iterator it = sTokenMap.find(hash(token));
	if (it == sTokenMap.end()) {
		if (printWarning) {
			minc_warn("$%s was not passed to CMIX as an argument", token);
		}
		return NULL;
	}
	return it->second;
}

