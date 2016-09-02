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

#!/usr/bin/python
#
# Copyright (C) 2015 Hewlett-Packard Development Company, L.P.
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#

import os
import sys
import pytest
import subprocess
import time

import ConfigParser
import json
import pprint

from bstUtil import *

from BstRestService import *
import bstRest as rest

import get_bst_feature_api_ct
import get_bst_tracking_api_ct
import get_bst_thresholds_api_ct
import get_bst_report_api_ct
import clear_bst_statistics_api_ct
import clear_bst_thresholds_api_ct
import configure_bst_feature_api_ct
import configure_bst_tracking_api_ct
import configure_bst_thresholds_api_ct

#cwdir = os.path.abspath(os.path.dirname(__file__))
cwdir, f = os.path.split(__file__)

config_dict = get_ini_details(cwdir + "/serverDetails.ini","server_details")
sw_type = config_dict.get('switch_type',"").strip()
if sw_type in ['',"genericx86-64"]:
    from opsvsi.docker import *
    from opsvsi.opsvsitest import *
elif sw_type == "as5712":
    OpsVsiTest = object
else:
    assert False,"Unknown platform is provided in the serverDetails.ini file..."

class bstTest( OpsVsiTest ):

    def setupNet(self):
        # if you override this function, make sure to
        # either pass getNodeOpts() into hopts/sopts of the topology that
        # you build or into addHost/addSwitch calls

        config_dict = get_ini_details(cwdir + "/serverDetails.ini","server_details")
        sw_type = config_dict.get('switch_type',"").strip()
        if sw_type in ['',"genericx86-64"]:
            topo=SingleSwitchTopo(
                k=1,
                hopts=self.getHostOpts(),
                sopts=self.getSwitchOpts())
            self.net = Mininet(topo=topo,
                switch=VsiOpenSwitch,
                host=Host,
                link=OpsVsiLink, controller=None,
                build=True)
            self.s1 = self.net.switches[0]

    def get_bst_feature(self):
        result,message = get_bst_feature_api_ct.main(self.ip_address,self.port)
        assert result,message

    def get_bst_tracking(self):
        result,message = get_bst_tracking_api_ct.main(self.ip_address,self.port)
        assert result,message

    def get_bst_thresholds(self):
        result,message = get_bst_thresholds_api_ct.main(self.ip_address,self.port)
        assert result,message

    def get_bst_report(self):
        result,message = get_bst_report_api_ct.main(self.ip_address,self.port)
        assert result,message

    def configure_bst_feature(self):
        result,message = configure_bst_feature_api_ct.main(self.ip_address,self.port)
        assert result,message

    def configure_bst_tracking(self):
        result,message = configure_bst_tracking_api_ct.main(self.ip_address,self.port)
        assert result,message

    def configure_bst_thresholds(self):
        result,message = configure_bst_thresholds_api_ct.main(self.ip_address,self.port)
        assert result,message

    def clear_bst_statistics(self):
        result,message = clear_bst_statistics_api_ct.main(self.ip_address,self.port)
        assert result,message

    def clear_bst_thresholds(self):
        result,message = clear_bst_thresholds_api_ct.main(self.ip_address,self.port)
        assert result,message

    def start_agent(self):
        sw_type = self.config_dict.get('switch_type',"").strip()
        if sw_type in ['',"genericx86-64"]:
            print "***** Starting ops-broadview"
            self.s1.popen("/usr/bin/ops-broadview")
            time.sleep(60)
            startBroadView = self.s1.cmd("echo")
            print "***** ops-bradview status :" + startBroadView + "\n"

            out = self.s1.cmd("pgrep ops-broadview")
            out = out.strip().split("\n")
            if len(out) == 1:
                self.broadview_pid = out[0]
            else:
                self.broadview_pid = None
            print "ops-brodview pid : " + self.broadview_pid

    def stop_agent(self):
        pass

    def getSwitchIp(self):
        sw_type = self.config_dict.get('switch_type',"").strip()
        if sw_type in ['',"genericx86-64"]:
            out = self.s1.cmd("ifconfig eth0")
            self.ip_address = out.split("\n")[1].split()[1][5:]
            self.port = "8080"
        else:
            self.ip_address = self.config_dict.get('agent_server_ip',"127.0.0.1")
            self.port = self.config_dict.get('agent_server_port',"8080")
        print "\n"
        print "sw_type : " + sw_type
        print "ip_address : " + self.ip_address
        print "port : " + self.port

    def getConfigDetails(self,filename,section):
        self.config_dict = get_ini_details(filename,section)


class Test_bstd:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        # Create the Mininet topology based on mininet.
        pw, f = os.path.split(__file__)
        switchmounts=[pw + "/bufmondsim.yaml:/etc/openswitch/platform/Generic-x86/X86-64/bufmond.yaml"]
        if sw_type in ['',"genericx86-64"]:
            Test_bstd.test = bstTest(switchmounts=switchmounts)
        else:
            Test_bstd.test = bstTest()
        Test_bstd.test.getConfigDetails(pw + "/serverDetails.ini","server_details")
        Test_bstd.test.getSwitchIp()
        Test_bstd.test.start_agent()

    def teardown_class(cls):
        Test_bstd.test.stop_agent()
        if hasattr(Test_bstd.test,"net"):
            Test_bstd.test.net.stop()

    def setup_method(self, method):
        pass

    def teardown_method(self, method):
        pass

    def __del__(self):
        pass

    def test_get_bst_feature(self):
        self.test.get_bst_feature()

    def test_get_bst_tracking(self):
        self.test.get_bst_tracking()

    def test_get_bst_thresholds(self):
        self.test.get_bst_thresholds()

    def test_get_bst_report(self):
        self.test.get_bst_report()

    def test_configure_bst_feature(self):
        self.test.configure_bst_feature()

    def test_configure_bst_tracking(self):
        self.test.configure_bst_tracking()

    def test_configure_bst_thresholds(self):
        self.test.configure_bst_thresholds()

    def test_clear_bst_thresholds(self):
        self.test.clear_bst_thresholds()

    def test_clear_bst_thresholds(self):
        self.test.clear_bst_thresholds()

