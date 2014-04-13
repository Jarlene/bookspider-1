ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 0
OUTFILE = joke-server

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . ./include \
					$(ROOT)/include \
					$(ROOT)/xml/include \
					$(ROOT)/libsys/include \
					../dbclient/include \
					../libhttp/include \
					../libsdk/include \
					../libsdk/json \
					../dxt/include \
					../joke-spider/include \
					../joke-spider/website
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source ../joke-spider/website
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += ../libsdk/source/config.cpp
SOURCE_FILES += ../libsdk/source/web-translate.cpp
SOURCE_FILES += ../libhttp/zlib/deflate.cpp
SOURCE_FILES += ../joke-spider/source/joke-db.cpp
SOURCE_FILES += ../joke-spider/source/joke-list.cpp
SOURCE_FILES += ../joke-spider/source/joke-spider.cpp
SOURCE_FILES += ../joke-spider/source/joke-comment.cpp

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = $(ROOT)/libsys/$(BUILD).$(PLATFORM) ../dxt/$(BUILD).$(PLATFORM) ../libhttp/$(BUILD).$(PLATFORM) ../dbclient/$(BUILD).$(PLATFORM)

ifeq ($(RELEASE),1)
LIBPATHS += 
else
LIBPATHS += 
endif

LIBS = sys dxt http dbclient rt dl pthread

STATIC_LIBS = $(ROOT)/xml/$(BUILD).$(PLATFORM)/libtinyxml.a
STATIC_LIBS += ../libsdk/$(BUILD).$(PLATFORM)/libsdk.a
STATIC_LIBS += ../libhttp/zlib/libz.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = __gcc__builtin_

include $(ROOT)/gcc.mk
