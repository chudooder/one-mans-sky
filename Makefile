default:
	mkdir -p build
	cd build && cmake ..
	cd build && make -j8

clean:
	rm -rf build/
