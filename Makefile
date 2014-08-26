#
# QMiner makefile
#
# Prerequisites for documentation:
#  - doxygen:  sudo apt-get install doxygen
#  - GraphViz: sudo apt-get install graphviz
#  - docco:    sudo npm install -g docco
#

include ./Makefile.config

# QMiner version
VERSION = 0.7.0

# dependencies
THIRD_PARTY = src/third_party
LIBUV = $(THIRD_PARTY)/libuv
ifeq ($(UNAME), Linux)
  LIBV8 = $(THIRD_PARTY)/v8/out/x64.release/obj.target/tools/gyp
else ifeq ($(UNAME), Darwin)
  LIBV8 = $(THIRD_PARTY)/v8/out/x64.release
endif
SNAP = $(THIRD_PARTY)/Snap
LIBSNAP = $(SNAP)/snap-core
GLIB = src/glib
QMINER = src/qminer
BUILD = build

# lib includes
STATIC_LIBS = $(LIBSNAP)/libsnap.a $(GLIB)/glib.a $(LIBUV)/libuv.a \
	$(LIBV8)/libv8_base.x64.a $(LIBV8)/libv8_snapshot.a

# QMiner code
QMOBJS = $(QMINER)/qminer_core.o $(QMINER)/qminer_ftr.o $(QMINER)/qminer_aggr.o \
	$(QMINER)/qminer_op.o $(QMINER)/qminer_gs.o $(QMINER)/qminer_js.o \
	$(QMINER)/qminer_srv.o $(QMINER)/qminer_snap.o
MAINOBJ = $(QMINER)/main.o


# default make target
all: release

# release target turns on compiler optimizations and disables debugging asserts
release: CXXFLAGS += -O3 -DNDEBUG
release: TARGET=release
release: qm

# debug target turns on crash debugging, get symbols with <prog> 2>&1 | c++filt
debug: CXXFLAGS += -g
debug: LDFLAGS += -rdynamic
debug: TARGET=debug
debug: qm

qm:
	# compile glib
	make -C $(GLIB) $(TARGET)
	# compile SNAP
	make -C $(SNAP)	
	# compile qminer
	make -C $(QMINER) $(TARGET)
	# create qm commandline tool
	$(CC) -o qm $(QMOBJS) $(MAINOBJ) $(STATIC_LIBS) $(CXXFLAGS) $(LDFLAGS) $(LIBS) 
	# create qminer static library
	rm -f qm.a
	ar -cvq qm.a $(QMOBJS)
	# prepare instalation directory
	mkdir -p $(BUILD)
	# move in qm commandline tool
	mv ./qm ./$(BUILD)/
	# copy qminer javascript environment
	cp ./$(QMINER)/*.js ./$(BUILD)/
	# copy in unicode definiton files
	cp ./$(GLIB)/bin/UnicodeDef.Bin ./$(BUILD)/
	# copy in javascript libraries
	mkdir -p ./$(BUILD)/lib
	cp -r ./$(QMINER)/js/* ./build/lib
	# copy in admin GUI
	mkdir -p ./$(BUILD)/gui
	cp -r ./$(QMINER)/gui/* ./$(BUILD)/gui
	# copy resources
	mkdir -p ./$(BUILD)/resources
	cp -r ./$(QMINER)/resources/* ./$(BUILD)/resources
	cat ./$(QMINER)/src/qminer/gui/js/Highcharts/js/highcharts.js <(echo) ./$(QMINER)/src/qminer/gui/js/Highcharts/js/modules/exporting.js <(echo) ./$(QMINER)/src/qminer/js/visualization.js > ./$(BUILD)/gui/js/visualization.js
	
cleanall: clean cleandoc
	make -C $(THIRD_PARTY) clean

clean:
	make -C $(GLIB) clean
	make -C $(SNAP) clean
	make -C $(QMINER) clean
	rm -f *.o *.gch *.a qm
	rm -rf ./$(BUILD)/

lib:
	make -C $(THIRD_PARTY)

install: 
	# prepare installation directory
	mkdir /usr/local/qm-$(VERSION)
	# copy build to installation dir
	cp -r ./$(BUILD)/* /usr/local/qm-$(VERSION)
	# create link for qm commandline tool
	ln /usr/local/qm-$(VERSION)/qm /usr/local/bin/qm
	# set QMINER_HOME environment variable
	echo "QMINER_HOME=/usr/local/qm-$(VERSION)/\nexport QMINER_HOME" > qm.sh
	mv ./qm.sh /etc/profile.d/
	
uninstall:
	# delete installation
	rm -rf /usr/local/qm-$(VERSION)
	# delete link
	rm /usr/local/bin/qm
	# delete environment
	rm /etc/profile.d/qm.sh

doc: cleandoc
	./genJSdoc.sh
	docco -o ./docjs/ examples/movies/src/movies.js
	docco -o ./docjs/ examples/timeseries/src/timeseries.js
	docco -o ./docjs/ examples/linalg/src/linalg.js
	docco -o ./docjs/ examples/twitter/src/twitter.js
	docco -o ./docjs/ examples/hoeffdingtree/src/ht.js
	docco -o ./docjs/ examples/nnet/src/nnet.js
	docco -o ./docjs/ examples/kalman/src/kalman.js
	sed "s/00000000/$(DOXYGEN_STIME)/" Doxyfile | sed "s/11111111/$(DOXYGEN_SLVER)/" > Doxyfile-tmp
	$(DOXYGEN) Doxyfile-tmp
	
cleandoc:
	rm -rf doc
	rm -rf Doxyfile-tmp
	rm -rf log-doxygen.txt

installdoc: doc
	scp -r doc blazf@agava:www/qminer-$(DOXYGEN_TIME)
	ssh blazf@agava ln -fsn qminer-$(DOXYGEN_TIME) www/qminer

