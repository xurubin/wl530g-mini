O_TARGET := mvwlan.o

##EXTRA_LDFLAGS := mvwlan-mod.o

obj-y    := mvwlan-wrapper.o mvwlan-mod.o

include $(TOPDIR)/Rules.make

mvwlan-mod.o: ../../../../vendors/Marvell/Libertas/mvwlan-8M-160MHZ.o
	arm-elf-strip -R .modinfo -N cleanup_module $< -o $@
	perl -pi -e 's/__div0/__div1/g' $@
	perl -pi -e 's/init_module/init_mvwlan/g' $@
	perl -pi -e 's/__this_module/__this_mvwlan/g' $@

