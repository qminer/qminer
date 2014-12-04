all: release

release: TARGET=release
release: node

debug: TARGET=debug
debug: node

node:
	make -C ./qm $(TARGET)
	
clean:
	make -C ./qm clean