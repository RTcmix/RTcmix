#include <stdio.h>
#include <dlfcn.h>

typedef double (*UG_FUN)(float *, int, double *);
typedef void (*ProfileFun)();

double m_load(float *p, int n_args, double *pp)
{
    char *dsoName;
    char dsoPath[1024];
#ifdef SHAREDLIBDIR
    const char *directory = SHAREDLIBDIR;
#else
    const char *directory = "/musr/lib";
#endif
    int   i;
    int profileLoaded;
    void *handle;
    ProfileFun profileFun;

    /* assemble path to the shared library */

    i = (int) pp[0];
    dsoName = (char *) i;

    if (!dsoName || strlen(dsoName) == 0) {
	fprintf(stderr, "Bad argument for p[0]!\n");
	return 0;
    }

    /* if name contains a '/', assume it is a full or relative path */
    if (strchr(dsoName, '/'))
	strcpy(dsoPath, dsoName);
    /* if name does not start with "lib", add prefix and suffix */
    else if (strncmp(dsoName, "lib", 3))
	sprintf(dsoPath, "%s/lib%s.so", directory, dsoName);
    /* otherwise just prepend directory and use as is */
    else
	sprintf(dsoPath, "%s/%s", directory, dsoName);

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
	printf("Loaded standard profile\n");
    }

    /* if present, load & call the shared library's rtprofile function to 
     * load its symbols.  Note that we access the function via its 
     * unmangled symbol name due to its extern "C" decl in rt.h.
     */

     if ((profileFun = (ProfileFun) dlsym(handle, "rtprofile")) != NULL) { 
 	profileLoaded += 2; 
 	(*profileFun)(); 
 	printf("Loaded RT profile\n"); 
     } 

    if (!profileLoaded) {
	fprintf(stderr, "Unable to find a profile routine in DSO\n");
	dlclose(handle);
	return 0;
    }

    printf("Loaded %s functions from shared library.\n",
	   (profileLoaded == 3) ? "standard and RT" :
	   (profileLoaded == 2) ? "RT" :
	   "standard");

    return 1;
}
