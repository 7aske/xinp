build_dir=build
install_dir=/usr/bin

default_recipe: build

build:
	mkdir -p $(build_dir) && \
	cd $(build_dir) && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && \
	make

clean:
	rm -rf $(build_dir)

install: build
	sudo cp $(build_dir)/xinp $(install_dir)/
