'''
  *
  * (C) Copyright Broadcom Corporation 2015
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  *
  * You may obtain a copy of the License at
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
'''

#!/usr/bin/env python

import os
import sys
import ConfigParser

def returnStatus(actual, expected, passmsg="", failmsg=""):
    '''Returns a tuple with result and message after comapring two values.'''
    return ("PASS",passmsg) if actual == expected else ("FAIL",failmsg)

def dict_compare(d1, d2):
    '''Compares the provided two python dictionaries.'''
    d1_keys = set(d1.keys())
    d2_keys = set(d2.keys())
    intersect_keys = d1_keys.intersection(d2_keys)
    added = d1_keys - d2_keys
    removed = d2_keys - d1_keys
    modified = {o : (d1[o], d2[o]) for o in intersect_keys if d1[o] != d2[o]}
    same = set(o for o in intersect_keys if d1[o] == d2[o])
    return added, removed, modified, same

def get_ini_details(filename,section):
    '''returns a dictionary by reading the given ini file and the given section.'''

    config = ConfigParser.ConfigParser()
    config.read(filename)
    config_dict = dict(config.items(section))
    return config_dict
