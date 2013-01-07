# Copyright 2010-2013, Qualcomm Innovation Center, Inc.
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

ifneq ($(TARGET_CC),)
CC:=$(TARGET_CC)
endif

ifneq ($(TARGET_CXX),)
CXX:=$(TARGET_CXX)
else
endif

CPPFLAGS:=$(TARGET_CPPFLAGS) -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX
CXXFLAGS:=$(TARGET_CXXFLAGS) -Wall
LDLIBS:=$(TARGET_LDFLAGS) -lalljoyn -lcrypto -lssl -lpthread -lrt

all: chatresponder

clean:
	$(RM) chatresponder chatresponder.o

chatresponder: chatresponder.cc


