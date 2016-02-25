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


import urllib
import httplib
import json
import re

def parseUrl(url):
    '''Parses the given URL and sends back the IP address and the server port details.'''
    patt = r'https?://(.*?):?(\d*)/broadview/bst.*'
    match = re.search(patt, url)
    if match:
        return match.group(1), match.group(2) if match.group(2) else 8080
    else:
        return None, None

def genericREST(method,url,data,debug=True):
    '''sends a json request using the selected method: method and using X-auth-Token: xAuthToken, and data as the body
    if the post attempt fails "None" (null) will be returned and the text of the error
    will be appended to errorLit[].
    If the post attempt is successful the json response will be returned as a dictionary
    If debug=True, then the url and data will be printed to the system screen.
    '''

    headers = {"Content-type": "application/json", "Accept": "application/json"}

    if debug == True:
        debugLabel = "_"+method+"-REST: "
        print ""
        print debugLabel+ "url:          " + str(url)
        print debugLabel+ "data:         " + str(data)
        print ""

    ip,server_port=parseUrl(url)

    try:
        conn = httplib.HTTPConnection(ip, int(server_port))
    except Exception as e:
        return {"response":e, "data":None}

    try:
        conn.request(method, url, data, headers)
        response = conn.getresponse()
        res_data = response.read()
        conn.close()
        return {"response":response, "data":res_data}
    except Exception as e:
        conn.close()
        return {"response":e, "data":None}


def get(url, debug=False):
    return genericREST("GET", url, None, debug)

def post(url, data, debug=False):
    return genericREST("POST", url, data, debug)

def put(url, data, debug=False):
    return genericREST("PUT", url, data, debug)

def delete(url, data, debug=False):
    return genericREST("DELETE", url, data, debug)

def head(url, debug=False):
    return genericREST("HEAD", url,None, debug)
