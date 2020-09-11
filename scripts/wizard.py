#! /usr/bin/env python
#coding=utf-8

#
# File      : wizard.py
# This file is part of CMCC IOT OS
# COPYRIGHT (C) 2012-2020, CMCC IOT
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#

"""
wizard.py - a script to generate SConscript in CMCC IOT OS. 

`wizard --component name' to generate SConscript for name component.
`wizard --bridge' to generate SConscript as a bridge to connect each 
SConscript script file of sub-directory. 
"""

import sys

SConscript_com = '''# CMCC IOT building script for component

from build_tools import *

pwd = PresentDir()
src = Glob('*.c') + Glob('*.cpp')
CPPPATH = [pwd]

group = AddCodeGroup('COMPONENT_NAME', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
'''

SConscript_bridge = '''# CMCC IOT building script for bridge

import os
from build_tools import *

pwd = PresentDir()
objs = []
list = os.listdir(pwd)

for d in list:
    path = os.path.join(pwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        objs = objs + SConscript(os.path.join(d, 'SConscript'))

Return('objs')
'''

def usage():
    print('wizard --component name')
    print('wizard --bridge')

def gen_component(name):
    print('generate SConscript for ' + name)
    text = SConscript_com.replace('COMPONENT_NAME', name)
    f = open('SConscript', 'w')
    f.write(text)
    f.close()

def gen_bridge():
    print('generate SConscript for bridge')
    f = open('SConscript', 'w')
    f.write(SConscript_bridge)
    f.close()

if __name__ == '__main__':
    if len(sys.argv) == 1:
        usage()
        sys.exit(2)
    
    if sys.argv[1] == '--component':
        gen_component(sys.argv[2])
    elif sys.argv[1] == '--bridge':
        gen_bridge()
    else:
        usage()
