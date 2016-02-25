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
import json
import pprint

from bstUtil import *

from BstRestService import *
import bstRest as rest

class configure_bst_feature_api_ct(object):

    def __init__(self,ip,port,params="",debug=False):
        self.obj = BstRestService(ip,port)
        self.debug = debug
        self.params = params

    def step1(self,jsonData):
        """Configure BST feature"""
        try:
            pprint.pprint(jsonData)
            resp = self.obj.postResponse(jsonData)
            if resp[0] == "INVALID":
                return "FAIL","Connection refused/Invalid JSON request... Please check the ip address provided in 'ini' file/BroadViewAgent is running or not/JSON data is valid or not ..."
        except Exception,e:
            return "FAIL","Unable to perform the rest call with given JSON data, Occured Exception ... "+str(e)

        try:
            self.obj.debugJsonPrint(self.debug,jsonData,resp)
        except:
            return "FAIL","Invalid JSON Response data received"

        self.nextStepCheckParams=jsonData
        if returnStatus(resp[0], 200)[0] == "FAIL": return "FAIL","Obtained {0}".format(resp[0])
        return returnStatus(resp[0], 200,"","Unable to get the 200 OK response, got reponse "+str(resp[0]))

    def step2(self,jsonData):
        """Get BST Feature Status"""
        try:
            resp = self.obj.postResponse(jsonData)
            if resp[0] == "INVALID":
                return "FAIL","Connection refused/Invalid JSON request... Please check the ip address provided in 'ini' file/BroadViewAgent is running or not/JSON data is valid or not ..."
        except Exception,e:
            return "FAIL","Unable to perform the rest call with given JSON data, Occured Exception ... "+str(e)

        try:
            self.obj.debugJsonPrint(self.debug,jsonData,resp)
        except:
            return "FAIL","Invalid JSON Response data received"

        if returnStatus(resp[0], 200)[0] == "FAIL": return "FAIL","Obtained {0}".format(resp[0])
        if not resp[1]: return "FAIL","Got null response"
        resp_ = resp[1].replace('Content-Type: text/json', '')
        data_dict = json.loads(resp_)
        if not "result" in data_dict: return "FAIL","No Result key in Response JSON Data"
        resultDict = data_dict['result']
        jsonDict = json.loads(self.nextStepCheckParams)
        paramsDict = jsonDict['params']
        valsCheck = True if cmp(resultDict, paramsDict) == 0 else False
        added,removed,modified,same=dict_compare(resultDict,paramsDict)
        if not added and not removed and not modified:
            msg = ""
        else:
            params_list=paramsDict.keys()
            diff_list=list(set(params_list) - same)
            msg="params "+" ".join(diff_list)+" contains wrong values in response."
        return returnStatus(valsCheck,True,"",msg)

    step3, step4 = step1, step2
    step5, step6 = step1, step2
    step7, step8 = step1, step2
    step9, step10 = step1, step2
    step11, step12 = step1, step2
    step13, step14 = step1, step2
    step15, step16 = step1, step2
    step17, step18 = step1, step2
    step19, step20 = step1, step2
    step21, step22 = step1, step2

    def getSteps(self):
        return sorted([ i for i in dir(self) if i.startswith('step') ], key=lambda item: int(item.replace('step','')))

def main(ip_address,port):
    jsonText = ConfigParser.ConfigParser()
    cwdir, f = os.path.split(__file__)
    jsonText.read(cwdir + '/testCaseJsonStrings.ini')
    json_dict = dict(jsonText.items('configure_bst_feature_api_ct'))
    params=json_dict.get("paramslist","")

    tcObj = configure_bst_feature_api_ct(ip_address,port,params,debug=True)

    stepResultMap = {}
    printStepHeader()
    for step in tcObj.getSteps():
        if step in json_dict:
            resp=getattr(tcObj,step)(json_dict[step])
            desc=getattr(tcObj,step).__doc__
            stepResultMap[step] = resp
            printStepResult(step,desc,resp[0], resp[1])
        else:
            resp=getattr(tcObj,step)()
            desc=""
            stepResultMap[step] = resp
            printStepResult(step,desc,resp[0], resp[1])
        if resp[0] == 'FAIL': break
    printStepFooter()
    statusMsgTuple = [ s for s in stepResultMap.values() if s[0] == "FAIL" ]
    if statusMsgTuple:
        return False, statusMsgTuple[0][1]
    return True, "Test Case Passed"

if __name__ == '__main__':
    main()
