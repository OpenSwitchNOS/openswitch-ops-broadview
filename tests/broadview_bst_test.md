# ops-broadview Test Cases

## Contents

- [Test get_bst_feature API](#get-bst-feature)
- [Test get_bst_tracking API](#get-bst-tracking)
- [Test get_bst_thresholds API](#get-bst-threshold)
- [Test get_bst_report API](#get-bst-report)
- [Test clear_bst_statistics API](#clear-bst-statisrics)
- [Test clear_bst_thresholds API](#clear-bst-threshold)
- [Test configure_bst_feature API](#configure-bst-feature)
- [Test configure_bst_tracking API](#configure-bst-tracking)
- [Test configure_bst_thresholds API](#configure-bst-thresholds)

##  Test get_bst_feature API ##
### Objective ###
Verify that the get_bst_feature API call yields 200 OK and the JSON response yields an expected parameter list defined in the associated testCaseJsonStrings.ini file.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.

 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call get_bst_feature API through REST with the following JSON data to POST to the ops-broadview.

 -      {"jsonrpc": "2.0", "method": "get-bst-feature", "params": { }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the following parameters present in the JSON response.
 -      stat-units-in-cells,collection-interval,async-full-reports,send-async-reports,send-snapshot-on-trigger,trigger-rate-limit,trigger-rate-limit-interval,stats-in-percentage,bst-enable
 
### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.
 

## Test get_bst_tracking API  ##
### Objective ###
Verify that the get_bst_tracking API call yields 200 OK and the JSON response yields expected parameter list defined in the associated testCaseJsonStrings.ini file.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call get_bst_tracking API through REST with the following JSON data to POST to the ops-broadview.
 -     {"jsonrpc": "2.0", "method": "get-bst-tracking", "params": { }, "id": 1, "asic-id":"1"}
 - Verify 200 OK is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the following parameters present in the JSON response.
 -     track-peak-stats,track-ingress-port-priority-group,track-ingress-port-service-pool,track-ingress-service-pool,track-egress-port-service-pool,track-egress-service-pool,track-egress-uc-queue,track-egress-uc-queue-group,track-egress-mc-queue,track-egress-cpu-queue,track-egress-rqe-queue,track-device
  
### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

 
# Test get_bst_thresholds API  ##
### Objective ###
Verify that the get_bst_threshold REST API call with the user configurable parameters yields 200 OK and receives an associated realm and it's data in the JSON response.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview. Set include-ingress-port-priority-group to 1 in the params section.
 -     {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 1, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the parameter set in the input JSON request received a realm and data in the JSON response. 
2. Repeat step no 1 by resetting the param set in step 1 to 0 and setting the next parameter in the params list to 1 and posting the request. The verification criteria is same as step 1.
 
### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

# Test get_bst_report API  ##
### Objective ###
Verify that the get_bst_report REST API call with the user configurable parameters yields 200 OK and receives an associated realm and it's data in the JSON response.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call get_bst_report API through REST with the following JSON data to POST to the ops-broadview. Set include-ingress-port-priority-group to 1 in the params section.
 -     {"jsonrpc": "2.0", "method": "get-bst-report", "params": { "include-ingress-port-priority-group": 1, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the parameter set in the input JSON request received a realm and data in the JSON response. 
2. Repeat step no 1 by resetting the param set in step 1 to 0 and setting the next parameter in the params list to 1 and posting the request. The verification criteria is same as step 1.
 
### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.
 
## Test clear_bst_statistics API  ##
### Objective ###
Verify that the clear_bst_statistics REST API call yields 200 OK.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call clear_bst_statistrics API through REST with the following JSON data to POST to the ops-broadview. 
 -     {"jsonrpc": "2.0", "method": "clear-bst-statistics", "params": { }, "id": 1, "asic-id":"1"} 
 - Verify 200 OK is received from the agent.

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.


# Test clear_bst_thresholds API  ##
### Objective ###
Verify that the clear_bst_thresholds REST API call yields 200 OK.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call clear_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview.
 -     {"jsonrpc": "2.0", "method": "clear-bst-statistics", "params": { }, "id": 1, "asic-id":"1"} 
 - Verify 200 OK is received from the agent.

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.
 
# Test configure_bst_feature API  ##
### Objective ###
Verify that the configure_bst_feature REST API call with the user configurable parameters yields 200 OK and reflects the associated configuration in the JSON response.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call configure_bst_feature API through REST with the following JSON data to POST to the ops-broadview. Keep all parameters to 0 values, except trigger-rate-limit and trigger-rate-limit-interval.
      {"jsonrpc": "2.0", "method": "configure-bst-feature", "id": 1, "asic-id": "1", "params": {"bst-enable": 0, "collection-interval": 0, "send-async-reports": 0, "stat-units-in-cells": 0, "trigger-rate-limit": 1, "send-snapshot-on-trigger": 0, "trigger-rate-limit-interval": 1, "async-full-reports": 0, "stats-in-percentage": 0}}
 - Verify 200 OK is received from the agent.

2. Call get_bst_feature API through REST with the following JSON data
 -      {"jsonrpc": "2.0", "method": "get-bst-feature", "params": { }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the following parameters present in the JSON response.
 -      stat-units-in-cells,collection-interval,async-full-reports,send-async-reports,send-snapshot-on-trigger,trigger-rate-limit,trigger-rate-limit-interval,stats-in-percentage,bst-enable
 - Verify that the JSON response has the correct configuration reflected as per step 1.
3. Repeat step 1 and step 2 for configuring other parameters from the params section. The verification crieteria is same.


### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

# Test configure_bst_tracking API  ##
### Objective ###
Verify that the configure_bst_tracking REST API call with the user configurable parameters yields 200 OK and reflects the associated configuration in the JSON response.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call configure_bst_tracking API through REST with the following JSON data to POST to the ops-broadview. Keep all parameters to 0 values.
 -     {"jsonrpc": "2.0", "method": "configure-bst-tracking", "asic-id": "1", "params": {"track-peak-stats" : 0, "track-ingress-port-priority-group" : 0, "track-ingress-port-service-pool" : 0, "track-ingress-service-pool" : 0, "track-egress-port-service-pool" : 0, "track-egress-service-pool" : 0, "track-egress-uc-queue" : 0, "track-egress-uc-queue-group" : 0, "track-egress-mc-queue" : 0, "track-egress-cpu-queue" : 0, "track-egress-rqe-queue" : 0, "track-device" : 0}, "id": 1}
 - Verify 200 OK is received from the agent.

2. Call get_bst_tracking API through REST with the following JSON data
 -      {"jsonrpc": "2.0", "method": "get-bst-tracking", "params": { }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the following parameters present in the JSON response.
 -     track-peak-stats,track-ingress-port-priority-group,track-ingress-port-service-pool,track-ingress-service-pool,track-egress-port-service-pool,track-egress-service-pool,track-egress-uc-queue,track-egress-uc-queue-group,track-egress-mc-queue,track-egress-cpu-queue,track-egress-rqe-queue,track-device 
 - Verify that the JSON response has the correct configuration reflected as per step 1.
3. Repeat step 1 and step 2 for configuring other parameters from the params section. The verification crieteria is same.


### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

# Test configure_bst_thresholds API  ##
### Objective ###
Verify that the configure_bst_thresholds REST API call with the user configurable parameters yields 200 OK and reflects the associated configuration in the JSON response.
### Requirements ###
 - Virtual Mininet Test Setup
 - serverSetupDetails.ini -- specify if the target switch_type is genericx86-64 or as5712 (default is genericx86-64). 
 - If target switch type is as5712, user needs to specify the IP of the management interface of the switch and the port on which the ops-broadview service is running.
 - If test is executed on the target=as5712, user needs to manually start the ops-broadview service on the switch.
 - testCaseJsonStrings.ini -- Contains the JSON strings need to be posted to the ops-broadview through REST API for each step
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "ingress-port-priority-group".
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "ingress-port-priority-group", "port": "1", "priority-group": 1, "um-share-threshold": 10, "um-headroom-threshold": 10  }, "id": 1, "asic-id":"1"} 
  - Verify 200 OK is received from the agent.
 
2. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-ingress-port-priority-group"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 1, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

3. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "ingress-port-service-pool".
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "ingress-port-service-pool", "port": "1", "service-pool": 1, "um-share-threshold": 10 }, "id": 1, "asic-id":"1"
  - Verify 200 OK is received from the agent.

4. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-ingress-port-service-pool"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 1, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

5. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "ingress-service-pool".
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "ingress-service-pool", "service-pool": 1, "um-share-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

6. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-ingress-service-pool"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 1, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.
 
7. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-port-service-pool"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-port-service-pool", "port": "1", "service-pool": 1, "uc-share-threshold": 10, "um-share-threshold": 10, "mc-share-threshold": 10, "mc-share-queue-entries-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

8. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-port-service-pool"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 1, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

9. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-service-pool"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-service-pool", "service-pool": 1, "um-share-threshold": 10, "mc-share-threshold": 10, "mc-share-queue-entries-threshold": 10  }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

10. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-service-pool"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 1, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

11. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-uc-queue"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-uc-queue", "queue": 1, "uc-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

12. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-uc-queue"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 1, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

13. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-uc-queue-group"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-uc-queue-group", "queue-group": 1, "uc-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

14. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-uc-queue-group"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 1, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

15. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-mc-queue"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-mc-queue", "queue": 1, "mc-threshold": 10, "mc-queue-entries-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

16. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-mc-queue"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 1, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

17. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-cpu-queue"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-cpu-queue", "queue": 1, "cpu-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

18. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-cpu-queue"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 1, "include-egress-rqe-queue": 0, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.
 
19. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "egress-rqe-queue"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "egress-rqe-queue", "queue": 1, "rqe-threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

20. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-egress-rqe-queue"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 1, "include-device": 0 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.

21. Call configure_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param realm "device"
  -     {"jsonrpc": "2.0", "method": "configure-bst-thresholds", "params": { "realm": "device", "threshold": 10 }, "id": 1, "asic-id":"1"}
  - Verify 200 OK is received from the agent.

22. Call get_bst_thresholds API through REST with the following JSON data to POST to the ops-broadview for the param "include-device"
 -      {"jsonrpc": "2.0", "method": "get-bst-thresholds", "params": { "include-ingress-port-priority-group": 0, "include-ingress-port-service-pool": 0, "include-ingress-service-pool": 0, "include-egress-port-service-pool": 0, "include-egress-service-pool": 0, "include-egress-uc-queue": 0, "include-egress-uc-queue-group": 0, "include-egress-mc-queue": 0, "include-egress-cpu-queue": 0, "include-egress-rqe-queue": 0, "include-device": 1 }, "id": 1, "asic-id":"1"}
 - Verify 200 OK status code is received from the agent.
 - Verify the response JSON is received with out any errors.
 - Verify the realm and its associated data is present on the JSON response.
 
### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.