#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <globals.h>
#include <ugens.h>
#include <Option.h>

typedef void (*ProfileFun)();

/* Assemble path to the shared library, and pass back as <dsoPath>. */
static int
get_dso_path(double pfield, char dsoPath[])
{
    char *str;
#ifdef SHAREDLIBDIR
    const char *directory = SHAREDLIBDIR;
#else
    const char *directory = "/musr/lib";
#endif

    /* cast double to string pointer */
    str = (char *) ((int) pfield);

    if (!str || strlen(str) == 0) {
		die("load", "Bad argument for p[0]!");
    }
    /* if name contains a '/', assume it is a full or relative path */
    if (strchr(str, '/'))
		strcpy(dsoPath, str);
    /* if name does not start with "lib", add prefix and suffix */
    else if (strncmp(str, "lib", 3))
		sprintf(dsoPath, "%s/lib%s.so", directory, str);
    /* otherwise just prepend directory and use as is */
    else
		sprintf(dsoPath, "%s/%s", directory, str);

    return 0;
}

#ifdef MACOSX

/* As of Mac OS X 10.1, the standard dlopen API is not supported officially
   (though there is a compatibility library floating around that works, from
   which this code was pilfered).  We roll our own loader here using the
   native OS X API for this reason, and also because it avoids a symbol
   collision with the Perl extension mechanism when using the dlopen
   compatibility library.   -JGG, 8/10/01
*/
#include <mach-o/dyld.h>

double m_load(float *p, int n_args, double *pp)
{
    char dsoPath[1024];
    NSObjectFileImage objectFileImage;
    NSObjectFileImageReturnCode result;
    NSModule module;
    NSSymbol symbol;
    ProfileFun profileFun;
    unsigned long options;
    int profileLoaded;

    get_dso_path(pp[0], dsoPath);

    result = NSCreateObjectFileImageFromFile(dsoPath, &objectFileImage);
    if (result != NSObjectFileImageSuccess)
	goto broken;

    options = NSLINKMODULE_OPTION_PRIVATE | NSLINKMODULE_OPTION_BINDNOW;
    module = NSLinkModule(objectFileImage, dsoPath, options);
    NSDestroyObjectFileImage(objectFileImage);
    if (module == NULL)
	goto broken;

    /*  Load & call the shared library's profile and/or rtprofile functions
	to load its symbols.
    */
    profileLoaded = 0;
    symbol = NSLookupSymbolInModule(module, "_profile");
    if (symbol != NULL) {
	profileFun = NSAddressOfSymbol(symbol);
	(*profileFun)();
	profileLoaded++;
    }
    symbol = NSLookupSymbolInModule(module, "_rtprofile");
    if (symbol != NULL) {
	profileFun = NSAddressOfSymbol(symbol);
 	(*profileFun)(); 
 	profileLoaded += 2; 
    } 
    if (!profileLoaded) {
	fprintf(stderr, "Unable to find a profile routine in DSO\n");
	NSUnLinkModule(module, 0);
	return 1.0;
    }
    if (get_bool_option(PRINT_STR)) {
	printf("Loaded %s functions from shared library '%s'.\n",
	    (profileLoaded == 3) ? "standard and RT" :
			(profileLoaded == 2) ? "RT" : "standard",
		dsoPath);
    }

    return 0.0;
broken:
    /* We could give more detailed error reporting here, but... */
    fprintf(stderr, "Unable to dynamically load '%s'\n", dsoPath);
    return 1.0;
}

#else /* !MACOSX */

#include <dlfcn.h>

double m_load(float *p, int n_args, double *pp)
{
    char dsoPath[1024];
    int profileLoaded;
    void *handle;
    ProfileFun profileFun;

    get_dso_path(pp[0], dsoPath);

    handle = dlopen(dsoPath, RTLD_NOW);

    if (!handle) {
	fprintf(stderr, "Unable to dynamically load '%s': %s\n",
		dsoPath, dlerror());
	return 0;
    }

    /* load & call the shared library's profile function to load its symbols */

    profileLoaded = 0;

    profileFun = (ProfileFun) dlsym(handle, "profile");
    if (profileFun) {
	profileLoaded++;
	(*profileFun)();
#ifdef DBUG
	printf("Loaded standard profile\n");
#endif
    }

    /* if present, load & call the shared library's rtprofile function to 
     * load its symbols.  Note that we access the function via its 
     * unmangled symbol name due to its extern "C" decl in rt.h.
     */

     profileFun = (ProfileFun) dlsym(handle, "rtprofile");
     if (profileFun) {
 	profileLoaded += 2; 
 	(*profileFun)(); 
#ifdef DBUG
 	printf("Loaded RT profile\n"); 
#endif
     } 

    if (!profileLoaded) {
		fprintf(stderr, "Unable to find a profile routine in DSO '%s'\n", 
				dsoPath);
		dlclose(handle);
		return 0;
    }

    if (get_bool_option(PRINT_STR)) {
		printf("Loaded %s functions from shared library '%s'.\n",
			  (profileLoaded == 3) ? "standard and RT" :
							   (profileLoaded == 2) ? "RT" : "standard",
			  dsoPath);
    }

    return 1;
}

#endif /* !MACOSX */
