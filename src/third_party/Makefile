#
# QMiner third party libraries
#

all:
	make -C libuv
	make -C Snap
	./v8-checkout.sh

clean:
	make -C libuv clean
	make -C Snap clean
	rm -rf ./v8/
