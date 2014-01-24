TOP = ..
include $(TOP)/Make.misc

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/conf
	$(INSTALL) -m 755 -d $(DISTDIR)/etc
	$(INSTALL) -m 755 -d $(DISTDIR)/bin
	$(INSTALL) -m 644 lsb.* lsf.* *.conf $(DISTDIR)/conf
	if [ '$(OS_TYPE)' = 'Linux' ]; then \
		$(INSTALL) -m 644 cshrc.jhlava profile.jhlava $(DISTDIR)/conf; \
		$(INSTALL) -m 755 jhlava $(DISTDIR)/bin; \
		$(INSTALL) -m 755 host.setup host.type pinstall $(DISTDIR)/etc; \
		$(TOP)/3rd-lib/util/d2u $(DISTDIR)/conf/*; \
		$(TOP)/3rd-lib/util/d2u $(DISTDIR)/etc/*; \
		$(TOP)/3rd-lib/util/d2u $(DISTDIR)/bin/jhlava; \
		$(TOP)/3rd-lib/util/d2u install.sh; \
		$(INSTALL) -m 755 install.sh $(DISTDIR); \
	fi
