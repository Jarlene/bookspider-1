ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 0
OUTFILE = webbooks

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					$(ROOT)/include \
					../bookspider/include \
					../dbclient/include \
					../webspider/source

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))
SOURCE_FILES += $(ROOT)/source/unicode.c
SOURCE_FILES += ../webspider/source/utf8.c

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = ../bookspider/$(BUILD).$(PLATFORM) ../dbclient/$(BUILD).$(PLATFORM)
ifdef RELEASE
# relase library path
LIBPATHS +=
else
LIBPATHS +=
endif

LIBS = pthread bookspider dbclient

STATIC_LIBS =

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES =

include $(ROOT)/gcc.mk
