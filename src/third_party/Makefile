#
# QMiner third party libraries
#

all:
	make -C libuv
	make -C node
	make -C v8 dependencies
	make -C v8 x64.release

clean:
	make -C libuv clean
	rm -rf ./v8/
	rm -rf ./node/
