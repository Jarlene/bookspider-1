ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 2
OUTFILE = libsdk.a

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./json \
					$(ROOT)/include 
					
#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = json
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

SOURCE_FILES += $(ROOT)/source/http-server.cpp
SOURCE_FILES += $(ROOT)/source/HttpSocket.cpp
SOURCE_FILES += $(ROOT)/source/http-upload.c
SOURCE_FILES += $(ROOT)/source/cookie.cpp
SOURCE_FILES += $(ROOT)/source/dlog.cpp
SOURCE_FILES += $(ROOT)/source/log.cpp
SOURCE_FILES += $(ROOT)/source/sysnetconfig.c
SOURCE_FILES += $(ROOT)/source/systimeconfig.c
SOURCE_FILES += $(ROOT)/source/sysprocess.c
SOURCE_FILES += $(ROOT)/source/tcpserver.c
SOURCE_FILES += $(ROOT)/source/thread-pool.c
SOURCE_FILES += $(ROOT)/source/task-queue.c
SOURCE_FILES += $(ROOT)/source/time64.c
SOURCE_FILES += $(ROOT)/source/base64.c
SOURCE_FILES += $(ROOT)/source/url.c
SOURCE_FILES += $(ROOT)/source/urlcodec.c
SOURCE_FILES += $(ROOT)/source/tools.c
SOURCE_FILES += $(ROOT)/source/unicode.c
SOURCE_FILES += $(ROOT)/source/systimer.c
SOURCE_FILES += $(ROOT)/source/html-entities.c
SOURCE_FILES += $(ROOT)/encoding/nls_cp936.c

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS =

ifeq ($(RELEASE),1)
LIBPATHS += 
else
LIBPATHS += 
endif

LIBS =

STATIC_LIBS =

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = OS_LINUX

include $(ROOT)/gcc.mk
