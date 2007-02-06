
# NOTE: This should match the name in uninstall-rtcmix.
pkgname="RTcmix-4.0.1.pkg"

all:
	./prep-package
	PackageMaker -build -proj RTcmix.pmproj -p ~/Desktop/${pkgname}

clean:
	$(RM) -r rtcmix
