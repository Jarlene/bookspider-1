ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 1
OUTFILE = libhttp.so

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . ./include ./zlib \
					$(ROOT)/include \
					../dxt/include
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source zlib
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))


#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = ../dxt/$(BUILD).$(PLATFORM)

ifeq ($(RELEASE),1)
LIBPATHS += 
else
LIBPATHS += 
endif

LIBS = dxt rt dl pthread

STATIC_LIBS = $(ROOT)/xml/$(BUILD).$(PLATFORM)/libtinyxml.a
STATIC_LIBS += ../libsdk/$(BUILD).$(PLATFORM)/libsdk.a
STATIC_LIBS += ./zlib/libz.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = LIBHTTP_EXPORTS

NOVERSION := 1

include $(ROOT)/gcc.mk
