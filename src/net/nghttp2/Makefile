#
# Rules for building library
#

# ----------------------------------------------------------------------------
# common rules
# ----------------------------------------------------------------------------
ROOT_PATH := ../../..

include $(ROOT_PATH)/gcc.mk

# ----------------------------------------------------------------------------
# library and objects
# ----------------------------------------------------------------------------
LIBS := libnghttp2.a

DIRS := ./nghttp2-1.43.0/lib
DIRS += ./nghttp2-1.43.0/third-party/url-parser
DIRS += .

SRCS := $(sort $(basename $(foreach dir,$(DIRS),$(wildcard $(dir)/*.[csS]))))

OBJS := $(addsuffix .o,$(SRCS))

# extra include path
INCLUDE_PATHS += -I$(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/lib \
                 -I$(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/lib/includes \
                 -I$(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/port \
                 -I$(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/third-party \
                 -I$(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/third-party/url-parser \
                 -I$(ROOT_PATH)/include/net/nghttp2

CC_FLAGS += -include $(ROOT_PATH)/include/net/nghttp2/nghttp2-1.43.0/port/config.h
CC_FLAGS += -fno-strict-aliasing

# library make rules
include $(LIB_MAKE_RULES)
