include ../../makefile.conf

namevers="RTcmix-4.0.1"
packagemaker="/Developer/usr/bin/packagemaker"

all: intel-usersshared

intel-applications:
	./prep-package $(CMIXDIR) "/Applications/rtcmix" "Intel"
	${packagemaker} --doc RTcmix-Intel-ApplicationsDir.pmdoc \
		--out ~/Desktop/${namevers}-Intel.pkg

intel-usersshared:
	./prep-package $(CMIXDIR) "/Users/Shared/rtcmix" "Intel"
	${packagemaker} --doc RTcmix-Intel-UsersSharedDir.pmdoc \
		--out ~/Desktop/${namevers}-Intel.pkg

clean:
	$(RM) -r rtcmix
