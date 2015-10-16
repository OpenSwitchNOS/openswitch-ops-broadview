High level design of FEATURE
============================

High level description of a FEATURE design. 

Please also refer to DESIGN.md document of the repo that contains BroadView daemon. 

Design choices
--------------
The BroadView Daemon was chosen, as it would export the BST Instrumentation Statistics in the Open BroadView Instrumentation defined REST API (JSON). This allows OpenSwitch to have the BroadView Instrumentation REST API support and allows Collectors and Instrumentation Apps to support OpenSwitch based switches. The same statistics are also available from the OVSDB Schema and would serve Collectors and Management systems that support OVSDB.

Participating modules
---------------------

                          +----+                           
+--------------------+    | O  |                           
|                    |    | V  |                           
| BroadView Daemon   <--->+ S  |                           
|                    |    | D  |                           
+--------------------+    | B  |                           
                          | |  |                           
                          | S  |                           
                          | e  |     +--------------------+
                          | r  |     |                    |
                          | v  <---->+                    |
                          | e  |     |     Driver         |
                          | r  |     +--------------------+
                          |    |                           
                          |    |                           
                          |    |                           
                          +----+                           

The BroadView Daemon interfaces with the OVSDB-Server, so it is loosely coupled with the Driver. The Driver is responsible for obtaining the statistics from the Switch Silicon and populating the BST statistics counters defined via the OVSDB schema. The Driver is the Publisher of the data and the BroadView Daemon is the Subscriber (consumer) of the data. This allows the design to be modular, with simple, well-defined interfaces.

OVSDB-Schema
------------
"bufmon": {
            "columns": {
                "counter_value": {
                    "ephemeral": true,
                    "type": {
                        "key": {
                            "minInteger": 0,
                            "type": "integer"
                        },
                        "max": 1,
                        "min": 0
                    }
                },
                "counter_vendor_specific_info": {
                    "type": {
                        "key": "string",
                        "max": "unlimited",
                        "min": 0,
                        "value": "string"
                    }
                },
                "enabled": {
                    "type": "boolean"
                },
                "hw_unit_id": {
                    "type": "integer"
                },
                "name": {
                    "type": "string"
                },
                "status": {
                    "type": {
                        "key": {
                            "enum": [
                                "set",
                                [
                                    "ok",
                                    "not-properly-configured",
                                    "triggered"
                                ]
                            ],
                            "type": "string"
                        },
                        "max": 1,
                        "min": 0
                    }
                },
                "trigger_threshold": {
                    "type": {
                        "key": {
                            "minInteger": 0,
                            "type": "integer"
                        },
                        "max": 1,
                        "min": 0
                    }
                }
            },
            "indexes": [
                [
                    "hw_unit_id",
                    "name"
                ]
            ],
            "isRoot": true
        }

The BroadView Daemon gets the configuration from the OVSDB schema. It exports statistics via its REST API using JSON messaging to a Collector or Controller. 


Any other sections that are relevant for the module
---------------------------------------------------

References
----------
* [Reference 1](http://www.openswitch.net/docs/redest1)
* ...

Include references to DESIGN.md of any module that participates in the feature.
Include reference to user guide of the feature.
