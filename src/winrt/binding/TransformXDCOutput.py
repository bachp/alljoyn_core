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

#
# This script massages the XDCMake output xml into something the Visual Studio object browser can view. 
#
import sys, codecs, os;
import re;


# dictionary of strings to replace 
d = { 'Platform.String': 'System.String',
      'Platform.Object' : 'System.Object',
      '#ER_': 'QStatus::ER_',
    }

p = re.compile("|".join(re.escape(k) for k in d))
def repl(m):
    return d[m.group(0)]

filename = sys.argv[1]
filenameOut = sys.argv[2]
inFile = codecs.open(filename, "r", "utf-8")
outFile = codecs.open(filenameOut, "w", "utf-8")
for line in inFile:
    newline = p.sub(repl, line)
    
    #Replace some of the more complicated expressions directly
    newline = re.sub('Platform\.Array\&lt\;(.*)\&gt\;', '\\1[]', newline)
    newline = re.sub('Platform\.WriteOnlyArray\&lt\;(.*)\&gt\;', '\\1[]', newline)
    newline = re.sub('\!System\.Runtime\.CompilerServices\.IsConst', '', newline)
    outFile.write(newline)
print sys.argv[0] + ": closing " + filename
inFile.close()

print sys.argv[0] + ": closing " + filenameOut
outFile.close()
