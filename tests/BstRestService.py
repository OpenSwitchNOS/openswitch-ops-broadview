'''
@copyright: Hewlett Packard
This are simple libraries which will send the following REST calls:
POST
PUT
DELETE
GET
They all have the signature of  "RestCall(url,data,XAuthToken,debug=False,errorList=[])
where url is the url the call will be sent to. dat is the data contained in the request, and xAuthToken is the string
which will be used for the "X-Auth-Token" part of the header.
debug is a boolean which can be used to print to the system console the data which will be sent.
errorList is a string which will be used to return any error found while making the call.

If the call is succesful the return json object will be returned as a dictionary.
If an error occur the return value will be None (null) and the error text will be copied to the errorString variable
'''

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

import json
import bstRest as rest
import urllib2
import sys


class BstRestService(object):

    def __init__(self, switch_ip='localhost',port='8080'):
        self.url = 'http://'+switch_ip+':'+port+'/broadview/bst/'
        self.verbose = False

    def setVerbose(self):
        '''
        setVerbose:
        API to set verbosity level for the session.
        No Arguments, sets self.verbose.
        '''
        self.verbose = True

    def clearVerbose(self):
        '''
        clearVerbose:
        API to clear verbosity level for the session.
        No Arguments, clears self.verbose.
        '''
        self.verbose = False

    def setURL(self, url):
        '''
        setURL:
        API to set base URL for the controller.
        No Arguments, sets self.url.
        '''
        self.url = url

    def getURL(self):
        '''
        getURL:
        API to get base URL for the controller.
        No Arguments, gets self.url.
        '''
        return self.url

    def getResponse(self):
        '''
        getResponse:
        API to make REST GET request.
        Takes URL, returns (code, value) pair.
            code: HTTP response code or 555 (for other errors) or Invalid when server not reachable
            value: JSON response object of error string.
        '''
        
        methodUrl = self.url

        if self.verbose:
            print 'Req URL: ', self.url
        res = rest.get(methodUrl, self.verbose)
        if isinstance(res["response"], urllib2.URLError):
            return (555, res.reason)
        else:
            try:
                return (res["response"].status, res["data"])
            except Exception as e:
                if hasattr(res["response"],"status") and hasattr(res["response"],"reason"):
                    return (res["response"].status, res["response"].reason)
                else:
                    return "INVALID",str(e)

    def postResponse(self, data):
        '''
        postResponse:
        API to make REST POST request.
        Takes URL, and data (generally a json string).
        Returns (code, value) pair.
            code: HTTP response code or 555 (for other errors) or Invalid when server not reachable
            value: JSON response object of error string.
        '''
        
        broadViewMethod = json.loads(data)[u'method']
        methodUrl = self.url+broadViewMethod

        if self.verbose:
            print 'Req URL: ', methodUrl
        res = rest.post(methodUrl, data, self.verbose)
        if isinstance(res, urllib2.URLError):
            return (555, res.reason)
        else:
            try:
                return (res["response"].status, res["data"])
            except Exception as e:
                if hasattr(res["response"],"status") and hasattr(res["response"],"reason"):
                    return (res["response"].status, res["response"].reason)
                else:
                    return "INVALID",str(e)

    def putResponse(self, data):
        '''
        putResponse:
        API to make REST PUT request.
        Takes URL, and data (generally a json string).
        Returns (code, value) pair.
            code: HTTP response code or 555 (for other errors) or Invalid when server not reachable
            value: JSON response object of error string.
        '''
        
        broadViewMethod = json.loads(data)[u'method']
        methodUrl = self.url+broadViewMethod

        if self.verbose:
            print 'Req URL: ', methodUrl
        res = rest.put(methodUrl, data, self.verbose)
        if isinstance(res, urllib2.URLError):
            return (555, res.reason)
        else:
            try:
                return (res["response"].status, res["data"])
            except Exception as e:
                if hasattr(res["response"],"status") and hasattr(res["response"],"reason"):
                    return (res["response"].status, res["response"].reason)
                else:
                    return "INVALID",str(e)

    def deleteResponse(self, data):
        '''
        daleteResponse:
        API to make REST DELETE request.
        Takes URL, and data (generally a json string on None).
        Returns (code, value) pair.
            code: HTTP response code or 555 (for other errors) or Invalid when server not reachable
            value: JSON response object of error string.
        '''
        
        broadViewMethod = json.loads(data)[u'method']
        methodUrl = self.url+broadViewMethod

        if self.verbose:
            print 'Req URL: ', methodUrl
        res = rest.delete(methodUrl, data,self.verbose)
        if isinstance(res, urllib2.URLError):
            return (555, res.reason)
        else:
            try:
                return (res["response"].status, res["data"])
            except Exception as e:
                if hasattr(res["response"],"status") and hasattr(res["response"],"reason"):
                    return (res["response"].status, res["response"].reason)
                else:
                    return "INVALID",str(e)

    @staticmethod
    def jsonData(data,padding=0):
        """ prints json response in human readable format """
        jstr = data
        for ele in jstr.keys():
            val = str(jstr[ele])
            if isinstance(jstr[ele],dict) or isinstance(jstr[ele], list):
                val = ""
    
            print(" " * padding + "   %-35s       :    %-20s"%(ele, val))
            if isinstance(jstr[ele],dict):
                subDict = jstr[ele]
                BstRestService.jsonData(subDict,padding+5)
    
            if isinstance(jstr[ele],list):
                for li in jstr[ele]:
                    if isinstance(li,dict):
                        BstRestService.jsonData(li,padding+5)
                    else:
                        print(" " * (padding+5) + "   %-35s"%(str(li)))

    @staticmethod
    def flattenJson(d, result=None):
        """ flattens the given json into single key value pair """
        if result is None:
            result = {}
        for key in d:
            value = d[key]
            if isinstance(value, dict):
                value1 = {}
                for keyIn in value:
                    value1[".".join([key,keyIn])]=value[keyIn]
                BstRestService.flattenJson(value1, result)
            elif isinstance(value, (list, tuple)):
                for indexB, element in enumerate(value):
                    if isinstance(element, dict):
                        value1 = {}
                        index = 0
                        for keyIn in element:
                            newkey = ".".join([key,keyIn])
                            value1[".".join([key,keyIn])]=value[indexB][keyIn]
                            index += 1
                        for keyA in value1:
                            BstRestService.flattenJson(value1, result)
            else:
                result[key]=value
        return result 

    @staticmethod
    def pretty_print(d, delimiter="."):
        """ pretty prints the json into key value pair """
        max_len = max([len(l) for l in d.keys()])
        max_len = 56 if max_len < 56 else max_len
        print ""
        for name, value in d.items():
            actual_len = len(name)
            delim_len = max_len - actual_len
            delim_string = delimiter * delim_len
            sys.stdout.write("{0} {1} {2}\n".format(name, delim_string, value))
        print ""

    @staticmethod
    def debugJsonPrint(debug,jsonData,resp):
        if debug == True:
            print ""
            print "-"*130
            print "Request :"
            print "-"*130
            BstRestService.jsonData(json.loads(jsonData))
            print "-"*130
            print "Response :"
            print "-"*130
            if resp[1]:
                resp_ = resp[1].replace('Content-Type: text/json', '')
                BstRestService.jsonData(json.loads(resp_))
            else:
                print "''"
            print "-"*130

def printStepHeader():
    print ""
    print "="*130
    print "%-7s | %-75s | %-8s | %-40s" % ("STEP", "DESCRIPTION", "OUTCOME", "REMARK")
    print "="*130

def printStepFooter():
    print "="*130
    print ""

def printStepResult(step,desc,result,remark=""):
    tmp_desc = desc[:72]+"..." if len(desc) > 75 else desc
    print "%-7s | %-75s | %-8s | %-40s" % (step,tmp_desc,result, remark)
    print "-" * 130


