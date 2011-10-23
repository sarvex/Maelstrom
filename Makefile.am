

bin_PROGRAMS = Maelstrom Maelstrom-netd

Maelstrom_SOURCES =		\
	Maelstrom.h		\
	Maelstrom_Globals.h	\
	buttonlist.h		\
	checksum.cpp		\
	checksum.h		\
	colortable.h		\
	controls.cpp		\
	controls.h		\
	dialog.cpp		\
	dialog.h		\
	fastrand.cpp		\
	fastrand.h		\
	init.cpp		\
	load.cpp		\
	load.h			\
	logic.h			\
	main.cpp		\
	myerror.cpp		\
	myerror.h		\
	netscore.cpp		\
	netscore.h		\
	public_key.h		\
	rect.cpp		\
	rect.h			\
	scores.cpp		\
	scores.h		\
	UIElementTitle.cpp	\
	UIElementTitle.h	\
	UIElements.cpp		\
	UIElements.h

LOGIC = netlogic

Maelstrom_LDADD = 			\
	$(LOGIC)/liblogic.a		\
	screenlib/libSDLscreen.a	\
	maclib/libSDLmac.a		\
	utils/libutils.a		\
	@MATHLIB@ @INETLIB@ @ICONOBJ@

SUBDIRS = $(LOGIC) screenlib maclib utils

EXTRA_DIST =			\
	autogen.sh		\
	VisualC.zip		\
	PBProjects.tar.gz	\
	MPWmake.sea.bin		\
	CREDITS			\
	Changelog		\
	Docs			\
	README.joystick		\
	README.network		\
	README.options		\
	README.simple		\
	COPYING			\
	COPYING.GPL		\
	Maelstrom_Data.zip	\
	ico_o			\
	icon.xpm		\
	Maelstrom.spec

DIST_SUBDIRS = $(SUBDIRS)

install-exec-hook:
	make append-data BINARY="$(DESTDIR)$(bindir)/Maelstrom$(EXEEXT)"

append-data:
	cat Maelstrom_Data.zip >>$(if $(BINARY),$(BINARY),Maelstrom$(EXEEXT))

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule to build a Mac OS X application
Maelstrom.app: Maelstrom$(EXEEXT)
	unzip $@.zip
	cp $< $@/Contents/MacOS/$<
	strip $@/Contents/MacOS/$<
	install_name_tool -change `otool -L $< | fgrep libSDL-1.2.0.dylib | awk '{print $$1}'` @executable_path/../Frameworks/libSDL-1.2.0.dylib $@/Contents/MacOS/$<
	install_name_tool -change `otool -L $< | fgrep libSDL_net-1.2.0.dylib | awk '{print $$1}'` @executable_path/../Frameworks/libSDL_net-1.2.0.dylib $@/Contents/MacOS/$<
	make append-data BINARY="$@/Contents/MacOS/$<"

Maelstrom.dmg: Maelstrom.app
	-mkdir tmp
	mv $< tmp/
	cp README.simple tmp/README.txt
	cp COPYING tmp/COPYING.txt
	cp COPYING.GPL tmp/
	cp -r Docs tmp/
	hdiutil create -fs HFS+ -srcfolder tmp -volname "Maelstrom" $@
	$(RM) -r tmp
