# Copyright 2012, Qualcomm Innovation Center, Inc.
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
# 

TARGET = liballjoyn.a

all:corelib

ifeq "$(OS)" "darwin"
   OS_SRC_DIR = $(OS)
else
   OS_SRC_DIR = $(OS_GROUP)
endif

ALLJOYN_SRC_SRCS = $(wildcard *.cc) 
ALLJOYN_SRC_SRCS += $(wildcard $(OS_SRC_DIR)/*.cc)
ALLJOYN_SRC_OBJS = $(patsubst %.cc,%.o,$(ALLJOYN_SRC_SRCS))

$(ALLJOYN_SRC_OBJS) : $(ALLJOYN_SRC_SRCS)
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) -c -o $@ $(patsubst %.o,%.cc,$@)

%.o:%.cc
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) -c -o $@ $<
    
corelib: $(ALLJOYN_SRC_OBJS) $(ALLJOYN_OS_OBJS) version.o
	$(AR) r $(TARGET) $(ALLJOYN_SRC_OBJS) $(VERSION_OBJ) $(wildcard $(COMMONDIR)/src/*.o) $(wildcard $(COMMONDIR)/os/$(OS_GROUP)/*.o) $(wildcard $(COMMONDIR)/crypto/$(CRYPTO)/*.o)
	cp $(TARGET) $(INSTALLDIR)/dist/lib

clean:
	-rm *.o *.~ $(TARGET) $(OS_SRC_DIR)/*.o
