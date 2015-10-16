BroadView Daemon
=======

What is BroadView Daemon?
----------------

Networks have become business critical and Network Operators are demanding greater instrumentation and telemetry capabilities so that they can get better visibility into their networks. Increased visibility enables them to proactively identify problems that may lead to poor network performance. It also helps network operators to better plan and fine tune their networks to meet strict SLAs and improve and maintain Application performance. Broadcom has introduced the [BroadView Instrumentation](https://github.com/Broadcom-Switch/BroadView-Instrumentation) software suite -- an industry first -- that provides unprecedented visibility into switch silicon. BroadView Instrumentation exposes the various instrumentation capabilities in Broadcom silicon and eases adoption of it by working with the ecosystem partners.

The suite consists of an Agent that runs on the switch and Applications that interface with the Agent over the Open REST API. Applications visualize, analyze data exported by the Agent, and provide the operator the ability to fine tune the network. The Agent is Open and portable across different Network Operating Systems. The BroadView Daemon is the implementation of the Agent functionality on OpenSwitch.

The BroadView Daemon is responsible for providing Instrumentation and visibility into the silicon in [OpenSwitch](http://www.openswitch.net) project. In this release, the BroadView Daemon supports Buffer Statistics Tracking (BST) which provides unprecedented visibility into MMU buffer utilization, enabling an operator to detect microbursts and fine tune the configuration to meet SLA for certain traffic, such as lossless traffic (e.g., storage).

The BroadView Daemon is heavily based on project [BroadView Instrumentation](https://github.com/Broadcom-Switch/BroadView-Instrumentation) and will be frequently upstreaming its changes to the parent project.

What is the structure of the repository?
----------------------------------------

* tests/ contains all the component tests of BroadView Daemon based on [ops-test-framework](http://git.openswitch.net/openswitch/ops-test-framework)
* configs/ contains all the example YAML files required by BroadView Daemon
* bar/ contains the modified parent project sources

What is the license?
--------------------
Being heavily based on project bar, ops-foo inherits its Apache 2.0 license. For more details refer to [COPYING](COPYING)

What other documents are available?
-----------------------------------
For the high level design of ops-foo, refer to [DESIGN.md](DESIGN.md)
For the current list of contributors and maintainers, refer to [AUTHORS.md](AUTHORS.md)

For general information about OpenSwitch project refer to http://www.openswitch.net
