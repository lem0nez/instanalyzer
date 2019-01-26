RAPIDJSON = rapidjson-1.1.0
LIBGIT2 = libgit2-0.27.7

LDFLAGS = -L$(LIB)/$(LIBGIT2)/build -lgit2
CXXFLAGS = -I$(LIB)/$(RAPIDJSON)/include \
	-I$(LIB)/$(LIBGIT2)/include

.PHONY: libs
libs: rapidjson git2

rapidjson:
	@rm -rf $(LIB)/$(RAPIDJSON) && mkdir -p $(LIB)/$(RAPIDJSON)
	curl -sL 'https://api.github.com/repos/Tencent/rapidjson/tarball/v1.1.0' | \
		tar -xzC $(LIB)/$(RAPIDJSON) --strip-components 1

git2:
	@rm -rf $(LIB)/$(LIBGIT2) && mkdir -p $(LIB)/$(LIBGIT2)
	curl -sL 'https://api.github.com/repos/libgit2/libgit2/tarball/v0.27.7' | \
		tar -xzC $(LIB)/$(LIBGIT2) --strip-components 1
	@mkdir $(LIB)/$(LIBGIT2)/build
	cd $(LIB)/$(LIBGIT2)/build && cmake -DTHREADSAFE=OFF -DUSE_SSH=OFF -DUSE_HTTPS=OFF -DBUILD_CLAR=OFF \
		-DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && cmake --build . -j $(THREADS)
