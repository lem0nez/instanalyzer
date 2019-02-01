JSON = json-3.5.0
CURLPP = curlpp-0.8.1
LIBZIPPP = libzippp-2.1-1.5.1
BOOST = boost-1.69.0

BOOST_LIBS = regex

LDFLAGS = -L$(LIB)/$(CURLPP)/lib \
	-L$(LIB)/$(LIBZIPPP)/lib \
	-L$(LIB)/$(BOOST)/lib
LDLIBS = -lcurl -lcurlpp -lzippp -lzip -lz -lboost_regex

CXXFLAGS = -I$(LIB)/$(JSON)/include \
	-I$(LIB)/$(CURLPP)/include \
	-I$(LIB)/$(LIBZIPPP)/include \
	-I$(LIB)/$(BOOST)/include

define download_tar
	@rm -rf $(1) && mkdir -p $(1)
	curl -sL $(2) | tar -xzC $(1) --strip-components 1
endef

.PHONY: libs
libs: json curlpp libzippp boost

json:
	$(call download_tar,/tmp/$(JSON),'https://api.github.com/repos/nlohmann/json/tarball/v3.5.0')
	@rm -rf $(LIB)/$(JSON) && mkdir -p $(LIB)/$(JSON)/include
	@mv /tmp/$(JSON)/single_include/* $(LIB)/$(JSON)/include
	@rm -rf /tmp/$(JSON)

curlpp:
	$(call download_tar,/tmp/$(CURLPP),'https://api.github.com/repos/jpbarrette/curlpp/tarball/v0.8.1')
	@mkdir /tmp/$(CURLPP)/build
	cd /tmp/$(CURLPP)/build && cmake .. && make -j$(THREADS)
	@rm -rf $(LIB)/$(CURLPP) && mkdir -p $(LIB)/$(CURLPP)/lib
	@mv /tmp/$(CURLPP)/include $(LIB)/$(CURLPP)
	@mv /tmp/$(CURLPP)/build/libcurlpp.a $(LIB)/$(CURLPP)/lib
	@rm -rf /tmp/$(CURLPP)

libzippp:
	$(call download_tar,/tmp/$(LIBZIPPP),$\
		'https://api.github.com/repos/ctabin/libzippp/tarball/libzippp-v2.1-1.5.1')
	cd /tmp/$(LIBZIPPP) && make -j$(THREADS)
	@rm -rf $(LIB)/$(LIBZIPPP) && mkdir -p $(LIB)/$(LIBZIPPP)/lib $(LIB)/$(LIBZIPPP)/include
	@mv /tmp/$(LIBZIPPP)/libzippp.a $(LIB)/$(LIBZIPPP)/lib
	@mv /tmp/$(LIBZIPPP)/src/libzippp.h $(LIB)/$(LIBZIPPP)/include
	@rm -rf /tmp/$(LIBZIPPP)

boost:
	$(call download_tar,/tmp/$(BOOST),$\
		'https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz')
	@rm -rf $(LIB)/$(BOOST) && mkdir -p $(LIB)/$(BOOST)/include
	cd /tmp/$(BOOST) && ./bootstrap.sh --with-libraries=$(BOOST_LIBS) && ./b2 \
		--stagedir=$(PROJECT_PATH)/$(LIB)/$(BOOST) link=static threading=multi runtime-link=static
	mv /tmp/$(BOOST)/boost $(LIB)/$(BOOST)/include
	@rm -rf /tmp/$(BOOST)
