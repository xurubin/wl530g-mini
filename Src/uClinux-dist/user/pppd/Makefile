
.EXPORT_ALL_VARIABLES:

DIRS = pppd

all:
	#make -C ../../user.asus/openl2tp-0.9 wx54g
	make -C pppd/plugins
	for i in $(DIRS) ; do make -C $$i || exit $? ; done

romfs:
	for i in $(DIRS) ; do make -C $$i romfs || exit $? ; done

clean:
	-for i in $(DIRS) ; do make -C $$i clean ; done


