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

.PHONY: all clean

DAEMON_LIB = libajdaemon.a

all: $(DAEMON_LIB) daemon bundled_obj test_progs

BT_SRCS = BTController.cc BTTransport.cc BTNodeDB.cc
DAEMON_SRCS = $(wildcard *.cc)
DAEMON_SRCS += $(wildcard *.c)
DAEMON_SRCS += $(wildcard ice/*.cc)
DAEMON_SRCS += $(wildcard JSON/*.cc)
DAEMON_SRCS += $(wildcard ns/*.cc)
DAEMON_SRCS += \
	$(OS_GROUP)/ProximityScanner.cc \
	$(OS_GROUP)/Socket.cc 

INCLUDE += -I$(PWD)/ice -I$(PWD)/$(OS_GROUP) -I$(PWD)/JSON -I$(PWD)/ns

ifeq "$(OS)" "darwin"
    # Darwin has its own version of the daemon transport
    DAEMON_SRCS += $(OS)/DaemonTransport.cc
else
    DAEMON_SRCS += $(OS_GROUP)/DaemonTransport.cc
endif

# Select BlueZ for bluetooth support or not
ifeq "$(OS)" "android_donut"
    # Skip bluetooth
else
    # Use BlueZ
    INCLUDE += -I$(PWD)/bt_bluez
    DAEMON_SRCS += $(wildcard bt_bluez/*.cc)
endif

DAEMON_OBJS = $(patsubst %.cc,%.o,$(DAEMON_SRCS))
$(DAEMON_OBJS) : $(DAEMON_SRC)
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) -c -o $@ $(patsubst %.o,%.cc,$@)


CONFIG_SRCS = ConfigDB.cc ServiceDB.cc PropertyDB.cc PolicyDB.cc
CONFIG_OBJS = $(patsubst %.cc,%.o,$(CONFIG_SRCS))

DAEMON_MAIN = $(OS_GROUP)/daemon-main.cc
DAEMON_MAIN_OBJ = $(patsubst %.cc,%.o,$(DAEMON_MAIN))

TESTDIR = test

$(TESTDIR)/advtunnel.o : $(TESTDIR)/advtunnel.cc
$(TESTDIR)/bbdaemon.o : $(TESTDIR)/bbdaemon.cc
$(TESTDIR)/mcmd.o : $(TESTDIR)/mcmd.cc

BUNDLED_SRCS = bundled/BundledDaemon.cc
BUNDLED_OBJ = $(patsubst %.cc,%.o,$(BUNDLED_SRCS))

%.o:%.cc
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) -c -o $@ $<

$(DAEMON_LIB): $(DAEMON_OBJS)
	$(AR) r $(DAEMON_LIB) $(DAEMON_OBJS)
	cp $(DAEMON_LIB) $(INSTALLDIR)/dist/lib

daemon : $(DAEMON_OBJS) $(DAEMON_MAIN_OBJ)
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o alljoyn-daemon $(DAEMON_OBJS) $(DAEMON_MAIN_OBJ) $(LIBS)
	cp alljoyn-daemon $(INSTALLDIR)/dist/bin
 
bundled_obj : $(BUNDLED_OBJ)
	cp $(BUNDLED_OBJ) $(INSTALLDIR)/dist/lib

test_progs: advtunnel bbdaemon mcmd 

advtunnel : $(DAEMON_OBJS) $(TESTDIR)/advtunnel.o
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o advtunnel $(DAEMON_OBJS) $(TESTDIR)/advtunnel.o $(LIBS)
	cp advtunnel $(INSTALLDIR)/dist/bin

bbdaemon : $(DAEMON_OBJS) $(TESTDIR)/bbdaemon.o
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o bbdaemon $(DAEMON_OBJS) $(TESTDIR)/bbdaemon.o $(LIBS)
	cp bbdaemon $(INSTALLDIR)/dist/bin

DaemonTest : $(DAEMON_OBJS) $(TESTDIR)/DaemonTest.o
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o DaemonTest $(DAEMON_OBJS) $(TESTDIR)/DaemonTest.o $(LIBS)
	cp DaemonTest $(INSTALLDIR)/dist/bin

mcmd : $(DAEMON_OBJS) $(TESTDIR)/mcmd.o
	$(CC) $(CXXFLAGS) $(CPPDEFINES) $(INCLUDE) $(LINKFLAGS) -o mcmd $(DAEMON_OBJS) $(TESTDIR)/mcmd.o $(LIBS)
	cp mcmd $(INSTALLDIR)/dist/bin

clean:
	@rm -f *.o *~ $(OS_GROUP)/*.o $(TESTDIR)/*.o bt_bluez/*.o ice/*.o bundled/*.o JSON/*.o ns/*.o alljoyn-daemon $(DAEMON_LIB) advtunnel bbdaemon DaemonTest mcmd


