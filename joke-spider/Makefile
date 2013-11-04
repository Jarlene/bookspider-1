ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 0
OUTFILE = joke-spider

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . ./include ./website \
					$(ROOT)/include \
					$(ROOT)/xml/include \
					../dbclient/include \
					../libhttp/include \
					../libsdk/include
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source website
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += ../libsdk/source/config.cpp

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

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES =

include $(ROOT)/gcc.mk
