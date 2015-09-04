include ../../makefile.conf

version="4.1.2"

all: intel-usrlocal

intel-applications:
	mkdir pkgtmp
	./prep-package $(CMIXDIR) "/Applications/rtcmix" "Intel"
	pkgbuild    --root rtcmix \
	            --identifier org.rtcmix.rtcmix \
	            --version $(version) \
	            --ownership recommended \
	            --install-location "/Applications/rtcmix" \
	            --scripts scripts \
	            pkgtmp/rtcmix.pkg
	productbuild \
	            --distribution distribution.dist \
	            --resources resources \
	            --package-path pkgtmp \
	            --version $(version) \
	            RTcmix-v$(version).pkg

intel-usrlocal:
	mkdir pkgtmp
	./prep-package $(CMIXDIR) "/usr/local/rtcmix" "Intel"
	pkgbuild    --root rtcmix \
	            --identifier org.rtcmix.rtcmix \
	            --version $(version) \
	            --ownership recommended \
	            --install-location "/usr/local/rtcmix" \
	            --scripts scripts \
	            pkgtmp/rtcmix.pkg
	productbuild \
	            --distribution distribution.dist \
	            --resources resources \
	            --package-path pkgtmp \
	            --version $(version) \
	            RTcmix-v$(version).pkg

clean:
	$(RM) -r rtcmix pkgtmp
