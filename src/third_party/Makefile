#
# QMiner third party libraries
#

all:
	make -C libuv
	make -C node
	./v8-checkout.sh

clean:
	make -C libuv clean
	rm -rf ./v8/
	rm -rf ./node/
