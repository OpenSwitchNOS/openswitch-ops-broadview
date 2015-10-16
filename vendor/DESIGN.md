High level design of BroadView Daemon
============================

Networks have become business critical and Network Operators are demanding greater instrumentation and telemetry capabilities so that they can get better visibility into their networks. Increased visibility would allow them to identify problems leading to poor network performance proactively. It also helps network operator to better plan and fine tune their networks to meet strict SLAs and improve and maintain Application performance. Broadcom has introduced [BroadView Instrumentation](https://github.com/Broadcom-Switch/BroadView-Instrumentation), an industry first, software suite that provides unprecedented visibility into switch silicon. BroadView Instrumentation will expose the various instrumentation capabilities in Broadcom silicon and ease adoption of it by working with the ecosystem partners.

The suite consists of an Agent that runs on the switch and Applications that interface with the Agent over Open REST API. Applications visualize, analyze data exported by the Agent and provide ability to operator to fine tune the network. The Agent is Open and portable across different Network Operating Systems. BroadView Daemon is the implementation of the Agent functionality on OpenSwitch.

The BroadView Daemon is based on the modular BroadView Agent developed and open sourced by Broadcom. The BroadView Agent has been ported to OpenSwitch OS and adapted to support the Interface/API mechanism provided by OpenSwitch. The Daemon consists of a NB plugin which is responsible for marshalling and unmarshalling of REST messages and interfacing with the Core Agent module. The NB plugin interfaces with the REST service of OpenSwitch. The SB plugin for this implementation has been modified to interface with the OVSDB Server.

In this release the BroadView Daemon collects the BST Instrumentation Statistics from the Broadcom Silicon and exports those via the BroadView Instrumentation REST (JSON) API. Instrumentation Collectors and Apps interface with the BroadView Daemon to obtain these statistics and use them for visualization, analytics, etc. An example of Visualization App is Ganglia. 

Reponsibilities
---------------
BroadView Daemon's main responsiblity it to collect the BST Instrumentation Statistics from the Broadcom Silicon and exports those via the BroadView Instrumentation REST (JSON) API. It is also responsible to configure the silicon for configuration related to the Instrumentation feature.

Design choices
--------------
Discuss any design choices that were made.

Relationships to external OpenSwitch entities
---------------------------------------------

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

The BroadView Daemon interfaces with the OVSDB-Server so it is loosely coupled with the Driver. The Driver is the one responsible for obtaining the statistics from the Switch Silicon and populating the BST statistics counters defined via the OVSDB schema.The Driver is the Publisher of the data and the BroadView Daemon is the Subscriber (consumer) to the data. This allows the design to be very modular and keeping the interfaces simple and well-defined.

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

BroadView Daemon gets the configuration from the OVSDB schema, it exports statistics via its REST API using JSON messaging to a Collector or Controller. 

Internal structure
------------------
The picture below is a very high level simple description.

           ^  REST (JSON)
           |             
+----------+---------+   
|                    |   
|   NorthBound (NB)  <   
|       Plugin       |   
+--------------------+   
                         
+--------------------+   
|                    |   
|   Core Agent       |   
|                    |   
+--------------------+   
                         
 +-------------------+   
 |                   |   
 |  SouthBound (SB)  |   
 |      Plugin       |   
 +-------------------+   

The Daemon consists of a NB plugin which is responsible for marshalling and unmarshalling of REST messages and interfacing with the Core Agent module. The NB plugin interfaces with the REST service of OpenSwitch. The SB plugin for this implementation has been modified to interface with the OVSDB Server. 

Detailed Design Description can be found at the [BroadView Instrumentation Design Documentation](http://broadcom-switch.github.io/BroadView-Instrumentation/doc/html/dc/d12/DESIGN.html) as also [functional details](http://broadcom-switch.github.io/BroadView-Instrumentation/doc/html/modules.html) on BroadView Instrumentation GitHub.
Any other sections that are relevant for the module
---------------------------------------------------

References
----------
* [Reference 1](http://www.openswitch.net/docs/redest1)
* ...

Include references to any other modules that interact with this module directly or through the database model. For example, CLI, REST, etc.
ops-fand might provide reference to ops-sensord, etc.
