TOP = ..
include $(TOP)/Make.misc

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/bin
	if [ '$(OS_TYPE)' = 'Linux' ]; then \
		$(INSTALL) -m 755 lam-mpirun mpich2-mpiexec mpich-mpirun openmpi-mpirun esync $(DISTDIR)/bin/$(BINARY_TYPE); \
		$(INSTALL) -m 755 adoptHost reghost ssh_cmd $(DISTDIR)/sbin; \
	fi
