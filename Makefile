TOP = .
include $(TOP)/Make.misc

all: 
	chmod +x install-sh
	chmod +x config/host.type
	chmod +x 3rd-lib/util/d2u
	dos2unix install-sh
	dos2unix config/*
	dos2unix scripts/*
	dos2unix 3rd-lib/util/d2u
	cd jhtools; gmake all
	cd lsf/intlib; gmake all
	cd lsf/lib; gmake all
	cd lsbatch/lib; gmake all
	cd chkpnt; gmake all
	cd eauth; gmake all
	cd lsbatch/cmd; gmake all
	cd lsbatch/bhist; gmake all
	cd lsbatch/daemons; gmake all
	cd lsf/lim; gmake all
	cd lsf/lsadm; gmake all
	cd lsf/lstools; gmake all
	cd lsf/pim; gmake all
	cd lsf/res; gmake all

package:
	$(INSTALL) -m 755 -d $(DISTDIR)/log
	$(INSTALL) -m 755 -d $(DISTDIR)/work/logdir
	cd chkpnt; gmake package
	cd eauth; gmake package
	cd jhtools; gmake package
	cd lsbatch/bhist; gmake package
	cd lsbatch/cmd; gmake package
	cd lsbatch/daemons; gmake package
	cd lsf/lim; gmake package
	cd lsf/lsadm; gmake package
	cd lsf/lstools; gmake package
	cd lsf/pim; gmake package
	cd lsf/res; gmake package
	cd config; gmake package
	cd scripts; gmake package
	$(INSTALL) -m 755 -d $(DISTDIR)/lib/$(BINARY_TYPE)
	$(INSTALL) -m 755 3rd-lib/$(BINARY_TYPE)/*.so $(DISTDIR)/lib/$(BINARY_TYPE)
	cd $(DISTDIR)/lib/$(BINARY_TYPE); ln -s libglib-2.0.so libglib-2.0.so.0; ln -s libcrypto.so libcrypto.so.4;
	pwd
	cd $(DISTDIR)/bin/$(BINARY_TYPE); ln -s bkill bchkpnt; ln -s bkill bresume; ln -s bkill bstop;  ln -s bmgroup bugroup; ln -s lsrun lsgrun; \
	cd ../../../;tar zcvf ../$(TNAME).tar.gz $(PNAME);
clean:
	rm -rf $(DISTDIR)
	cd chkpnt; gmake clean
	cd eauth; gmake clean
	cd jhtools; gmake clean
	cd lsbatch/bhist; gmake clean
	cd lsbatch/cmd; gmake clean
	cd lsbatch/lib; gmake clean
	cd lsbatch/daemons; gmake clean
	cd lsf/intlib; gmake clean
	cd lsf/lib; gmake clean
	cd lsf/lim; gmake clean
	cd lsf/lsadm; gmake clean
	cd lsf/lstools; gmake clean
	cd lsf/pim; gmake clean
	cd lsf/res; gmake clean


