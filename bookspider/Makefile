ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 1
OUTFILE = bookspider

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					../webspider/source \
					$(ROOT)/include \
					$(ROOT)/xml/include
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source json stream
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += $(ROOT)/source/unicode.c
SOURCE_FILES += $(ROOT)/source/urlcodec.c
SOURCE_FILES += ../webspider/source/utf8.c

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = ../tpm/$(BUILD).$(PLATFORM)

ifeq ($(RELEASE),1)
LIBPATHS += 
else
LIBPATHS += 
endif

LIBS = pthread

STATIC_LIBS = $(ROOT)/xml/$(BUILD).$(PLATFORM)/libtinyxml.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = BOOKSPIDER_EXPORTS

include $(ROOT)/gcc.mk
