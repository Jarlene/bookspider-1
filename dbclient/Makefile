ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 1
OUTFILE = libdbclient.so

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					$(ROOT)/include \
					./mysql-connector-c-6.1.1-linux-glibc2.5-i686/include

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))
SOURCE_FILES += $(ROOT)/source/systimeconfig.c
SOURCE_FILES += $(ROOT)/source/tools.c

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS =
ifdef RELEASE
# relase library path
LIBPATHS +=
else
LIBPATHS +=
endif

LIBS = rt dl

STATIC_LIBS = ./mysql-connector-c-6.1.1-linux-glibc2.5-i686/lib/libmysqlclient.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = DBCLIENT_EXPORTS

include $(ROOT)/gcc.mk
