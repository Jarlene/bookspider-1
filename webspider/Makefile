ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 0
OUTFILE = webspider

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					$(ROOT)/include \
					$(ROOT)/xml/include \
					../dbclient/include \
					../bookspider/include \
					../dxt/include \
					../net/include
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source json stream
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += $(ROOT)/source/base64.c
SOURCE_FILES += $(ROOT)/source/tools.c
SOURCE_FILES += $(ROOT)/source/url.c
SOURCE_FILES += $(ROOT)/source/unicode.c
SOURCE_FILES += $(ROOT)/source/urlcodec.c
SOURCE_FILES += $(ROOT)/source/cookie.cpp
SOURCE_FILES += $(ROOT)/source/HttpSocket.cpp


#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = ../dxt/$(BUILD).$(PLATFORM) ../net/$(BUILD).$(PLATFORM) ../dbclient/$(BUILD).$(PLATFORM) ../bookspider/$(BUILD).$(PLATFORM)

ifeq ($(RELEASE),1)
LIBPATHS += 
else
LIBPATHS += 
endif

LIBS = rt dl pthread dxt net dbclient bookspider

STATIC_LIBS = $(ROOT)/xml/$(BUILD).$(PLATFORM)/libtinyxml.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES =

include $(ROOT)/gcc.mk
