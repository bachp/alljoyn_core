# Copyright 2010 - 2013, Qualcomm Innovation Center, Inc.
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

import os
Import('env')

# Indicate that this SConscript file has been loaded already
#
env['_ALLJOYNCORE_'] = True

# Dependent Projects
common_hdrs, common_objs = env.SConscript(['../common/SConscript'])

# The return value is the collection of files installed in the build destination.
returnValue = []

# Bullseye code coverage for 'debug' builds.
if env['VARIANT'] == 'debug':
    if(not(env.has_key('BULLSEYE_BIN'))):
        print('BULLSEYE_BIN not specified')
    else:
        env.PrependENVPath('PATH', env.get('BULLSEYE_BIN'))
        if (not(os.environ.has_key('COVFILE'))):
            print('Error: COVFILE environment variable must be set')
            if not GetOption('help'):
                Exit()
        else:
            env.PrependENVPath('COVFILE', os.environ['COVFILE'])

# manually add dependencies for xml to h, and for files included in the xml
env.Depends('$OBJDIR/Status.h', 'src/Status.xml')
env.Depends('$OBJDIR/Status.h', '../common/src/Status.xml')
env.Append(STATUS_FLAGS=['--base=%s' % os.getcwd()])

# Make alljoyn C++ dist a sub-directory of the alljoyn dist.
env['CPP_DISTDIR'] = env['DISTDIR'] + '/cpp'
env['CPP_TESTDIR'] = env['TESTDIR'] + '/cpp'
env['WINRT_DISTDIR'] = env['DISTDIR'] + '/winRT'

if env['OS_GROUP'] == 'winrt':
    env.Depends('$OBJDIR/Status_CPP0x.h', 'src/Status.xml')
    env.Depends('$OBJDIR/Status_CPP0x.h', '../common/src/Status.xml')
    env.AppendUnique(CFLAGS=['/D_WINRT_DLL'])
    env.AppendUnique(CXXFLAGS=['/D_WINRT_DLL'])
    env.Append(STATUS_FLAGS=['--cpp0xnamespace=AllJoyn'])

# Add support for multiple build targets in the same workset
env.VariantDir('$OBJDIR', 'src', duplicate = 0)
env.VariantDir('$OBJDIR/test', 'test', duplicate = 0)
env.VariantDir('$OBJDIR/daemon', 'daemon', duplicate = 0)
env.VariantDir('$OBJDIR/samples', 'samples', duplicate = 0)
env.VariantDir('$OBJDIR/alljoyn_android', 'alljoyn_android', duplicate = 0)

# AllJoyn Install
env.Install('$OBJDIR', env.File('src/Status.xml'))
env.Status('$OBJDIR/Status')
core_headers = env.Install('$CPP_DISTDIR/inc/alljoyn', '$OBJDIR/Status.h')
core_headers += env.Install('$CPP_DISTDIR/inc/alljoyn', env.Glob('inc/alljoyn/*.h'))
if env['OS_GROUP'] == 'winrt':
    core_headers += env.Install('$CPP_DISTDIR/inc/alljoyn', '$OBJDIR/Status_CPP0x.h')

core_headers += env.Install('$CPP_DISTDIR/inc/alljoyn', [ h for h in env.Glob('inc/alljoyn/*.h') if h not in env.Glob('inc/alljoyn/Status*.h') ])

for d,h in common_hdrs.items():
    core_headers += env.Install('$CPP_DISTDIR/inc/%s' % d, h)

returnValue += core_headers

# Header file includes
env.Append(CPPPATH = [env.Dir('$CPP_DISTDIR/inc'), env.Dir('$CPP_DISTDIR/inc/alljoyn')])

# Make private headers available
env.Append(CPPPATH = [env.Dir('src')])

# AllJoyn Libraries
(libs, alljoyn_core_objs) = env.SConscript('$OBJDIR/SConscript', exports = ['common_objs'])

ajlib = env.Install('$CPP_DISTDIR/lib', libs)
returnValue += ajlib

env.Append(LIBPATH = [env.Dir('$CPP_DISTDIR/lib')])

# Set the alljoyn library 
env.Prepend(LIBS = ajlib)

# AllJoyn Daemon, daemon library, and bundled daemon object file
daemon_progs, bdlib, bdobj = env.SConscript('$OBJDIR/daemon/SConscript', exports = ['common_objs', 'alljoyn_core_objs'])
if env['OS_GROUP'] == 'winrt':
    returnValue += env.Install('$WINRT_DISTDIR/bin', daemon_progs)
    daemon_lib = [bdlib]
    daemon_obj = [bdobj]
else:
    returnValue += env.Install('$CPP_DISTDIR/bin', daemon_progs)
    daemon_lib = env.Install('$CPP_DISTDIR/lib', bdlib)
    daemon_obj = env.Install('$CPP_DISTDIR/lib', bdobj)

returnValue += daemon_lib
returnValue += daemon_obj

# Test programs to have built-in bundled daemon or not
if env['BD'] == 'on':
    env.Prepend(LIBS = daemon_lib)
    env.Prepend(LIBS = daemon_obj)
    env['bdlib'] = ""
    env['bdobj'] = ""
else:
    env['bdlib'] = daemon_lib
    env['bdobj'] = daemon_obj

# only include command line samples, unit test, test programs if we are not 
# building for iOS. No support on iOS for command line applications.
if env['OS'] == 'darwin' and (env['CPU'] == 'arm' or env['CPU'] == 'armv7' or env['CPU'] == 'armv7s'):
    progs = []
else:
    # Test programs
    progs = env.SConscript('$OBJDIR/test/SConscript')
    returnValue += env.Install('$CPP_DISTDIR/bin', progs)

    # Build unit Tests
    env.SConscript('unit_test/SConscript', variant_dir='$OBJDIR/unittest', duplicate=0)

    # Sample programs
    returnValue = env.SConscript('$OBJDIR/samples/SConscript')

# Android daemon runner
returnValue += env.SConscript('$OBJDIR/alljoyn_android/SConscript')

# Release notes and misc. legals
if env['OS_CONF'] == 'darwin':
    if env['CPU'] == 'x86':
        returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.darwin.txt')
        returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
        returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
        returnValue += env.Install('$DISTDIR', 'README.md')
        returnValue += env.Install('$DISTDIR', 'NOTICE.txt')
elif env['OS_CONF'] == 'winrt':
    returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.winrt.txt')
    returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
    returnValue += env.Install('$DISTDIR', 'README.md')
    returnValue += env.Install('$DISTDIR', 'NOTICE.txt')
elif env['OS_CONF'] == 'windows':
    returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.windows.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_java.txt')
    returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
    returnValue += env.Install('$DISTDIR', 'README.md')
    returnValue += env.Install('$DISTDIR', 'NOTICE.txt')
elif env['OS_CONF'] == 'linux':
    returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.linux.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_java.txt')
    returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
    returnValue += env.Install('$DISTDIR', 'README.md')
    returnValue += env.Install('$DISTDIR', 'NOTICE.txt')
elif env['OS_CONF'] == 'android':
    returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.android.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_java.txt')
    returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
    returnValue += env.Install('$DISTDIR', 'README.md')
    returnValue += env.Install('$DISTDIR', 'NOTICE.txt')
else:
    returnValue += env.InstallAs('$DISTDIR/README.txt', 'docs/README.linux.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_cpp.txt')
    returnValue += env.Install('$DISTDIR', 'docs/AllJoyn_API_Changes_java.txt')
    returnValue += env.Install('$DISTDIR', 'docs/ReleaseNotes.txt')
    returnValue += env.Install('$DISTDIR', 'README.md')
    returnValue += env.Install('$DISTDIR', 'NOTICE.txt')

# Build docs
installDocs = env.SConscript('docs/SConscript')
env.Depends(installDocs, core_headers);
returnValue += installDocs

#Build Win8 SDK installer
env.SConscript('win8_sdk/SConscript')

Return('returnValue')
