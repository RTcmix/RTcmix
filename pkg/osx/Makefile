include ../../makefile.conf

namevers="RTcmix-4.0.1"

all: intel

intel:
	./prep-package $(CMIXDIR) "Intel"
	PackageMaker -build -proj RTcmix-Intel.pmproj -p ~/Desktop/${namevers}-Intel.pkg

ppc:
	./prep-package $(CMIXDIR) "PPC"
	PackageMaker -build -proj RTcmix-PPC.pmproj -p ~/Desktop/${namevers}-PPC.pkg

clean:
	$(RM) -r rtcmix
