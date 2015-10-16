<!--  See the https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet for additional information about markdown text.
Here are a few suggestions in regards to style and grammar:
* Use active voice. With active voice, the subject is the doer of the action. Tell the reader what
to do by using the imperative mood, for example, Press Enter to view the next screen. See https://en.wikipedia.org/wiki/Active_voice for more information about the active voice. 
* Use present tense. See https://en.wikipedia.org/wiki/Present_tense for more information about using the present tense. 
* Avoid the use of I or third person. Address your instructions to the user. In text, refer to the reader as you (second person) rather than as the user (third person). The exception to not using the third-person is when the documentation is for an administrator. In that case, *the user* is someone the reader interacts with, for example, teach your users how to back up their laptop. 
* See https://en.wikipedia.org/wiki/Wikipedia%3aManual_of_Style for an online style guide.
Note regarding anchors:
--StackEdit automatically creates an anchor tag based off of each heading.  Spaces and other nonconforming characters are substituted by other characters in the anchor when the file is converted to HTML. 
 --> 
 
BroadView Daemon
=======
<!--Provide the title of the feature-->

 [TOC]
 
## Overview ##
Networks have become business critical and Network Operators are demanding greater instrumentation and telemetry capabilities so that they can get better visibility into their networks. Increased visibility enables them to proactively identify problems that may lead to poor network performance. It also helps network operators to better plan and fine tune their networks to meet strict SLAs and improve and maintain Application performance. Broadcom has introduced the [BroadView Instrumentation](https://github.com/Broadcom-Switch/BroadView-Instrumentation)software suite -- an industry first -- that provides unprecedented visibility into switch silicon. BroadView Instrumentation exposes the various instrumentation capabilities in Broadcom silicon and eases adoption of it by working with the ecosystem partners.

The suite consists of an Agent that runs on the switch and Applications that interface with the Agent over Open REST API. Applications visualize, analyze data exported by the Agent, and provide the operator the ability to fine tune the network. The Agent is Open and portable across different Network Operating Systems. The BroadView Daemon is the implementation of the Agent functionality on OpenSwitch.

The BroadView Daemon provides instrumentation capability for OpenSwitch. In the current release, it obtains MMU Buffer Statistics from Broadcom silicon and exports them via the REST API. This allows Instrumentation collectors or Apps to obtain the MMU Buffer stats and visualize buffer utilization patterns and detect microbursts. These help an operator to get visibility into the network and switch performance and fine tune the network. Some traffic, such as storage, requires lossless capability, and operators whose network carries these types of traffic are interested in learning about microbursts and tuning network to avoid packet drops during related congestion events.

The BroadView Daemon is recommended to be used with OpenSwitch running on a hardware platform (e.g. AS5712 from Accton).
 
## (Optional) Conceptual or reference info here ##
<!--Change heading for conceptual or reference info, such as Prerequisites. -->
## How to use the feature ##

###Setting up the basic configuration

 1. Step 1

###Setting up the optional configuration

 1. Step 1

###Verifying the configuration

 1. Step 1

###Troubleshooting the configuration

#### Condition 
Type the symptoms for the issue.
#### Cause 
Type the cause for the issue.
#### Remedy  
Type the solution.
## CLI ##
There are no CLI for BroadView Daemon.
## REST API ##
Instrumentation Collectors and Apps interface with the BroadView Daemon via [REST API](http://broadcom-switch.github.io/BroadView-Instrumentation/doc/html/dc/d3f/REST.html)  
## Related features ##
(<!-- Enter content into this section to describe features that may need to be considered in relation to this particular feature, under what conditions and why.  Provide a hyperlink to each related feature.  Sample text is included below as a potential example or starting point.  -->
When configuring the switch for FEATURE_NAME, it might also be necessary to configure [RELATED_FEATURE1](https://openswitch.net./tbd/other_filefeatures/related_feature1.html#first_anchor) so that....

