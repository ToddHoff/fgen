# fgen
<CENTER>
   <H2> Generate Framework Code from System Specifications </H2>
</CENTER>

<H2> Contents </H2>
<OL>
<LI> <A HREF="#descr">        Description </A>
   <OL>
   <LI> <A HREF="#is">           What <I>fgen</I> Is </A>
   <LI> <A HREF="#not">          What <I>fgen</I> Is Not </A>
   </OL>
<LI> <A HREF="#synopsis">     Synopsis </A>
     <OL>
     <LI> <A HREF="#setup">      Setup</A>
     <LI> <A HREF="#cli">        Command Line Invocation </A>
     </OL>
<LI> <A HREF="#syntax">       Syntax </A>
   <OL>
   <LI> <A HREF="#bnf">          <I>fgen</I>'s BNF </A>
   <LI> <A HREF="#cfgbnf">       Configuration BNF </A>
   <LI> <A HREF="#smbnf">        State Machine BNF </A>
   <LI> <A HREF="#ssf">          Source Specification Files </A>
   <LI> <A HREF="#comment">      <I>#</I> Comment Marker </A> 
   <LI> <A HREF="#extender">     <I>\</I> Line Extender </A> 
   </OL>
<LI> <A HREF="#config">       Configurable Attributes - CFG Block </A>

<LI> <A HREF="#sm">           State Machine - SM and IN Blocks </A>
   <OL> 
   <LI> <A HREF="#smdefined">    Defined </A>
   <LI> <A HREF="#msmexample">   Motivating Example of All the Work You can Save </A>
   <LI> <A HREF="#smfiles">      Files Produced </A>
   <LI> <A HREF="#smbs">         SM Definition (SM block)</A>
   <LI> <A HREF="#smstate">      State Definition (IN block) </A>
   <LI> <A HREF="#smcode">       Implementing the State Machine </A>
   <LI> <A HREF="#smexample">    State Machine and Agent Example </A>
   </OL>
<LI> <A HREF="Internals.html"> Internals </A> - fgen internals
<LI> <A HREF="#bugs">         Bugs</A>
<LI> <A HREF="#files">        Files </A>
</OL>


<P><HR><A NAME="desc">
<H2> Description </H2>

<A NAME="is"></A>
<H3> What <I>fgen</I> Is </H3>

<I>fgen</I> takes a system specification and generates code based on
the specification.  The idea is a programmer can make a simple
description of something and then generate a lot of infrastructure
type code based on the specification. <I>fgen</I> also makes
it easy to add new system specifications, extensions to existing specifications,
and different code generators for specifications.
<P>

Systems can be anything, but they usually map to something relatively complex
where automatic code generation can provide a big productivity win. 
<A HREF="#sm">State machines</A> are one such area where a state machine
can be simply specified and a lot of code generated.
<P>

<A NAME="not"></A>
<H3> What <I>fgen</I> Is Not </H3>

<I>fgen</I> is not a perfect compiler implementing a perfect
grammar. It is a quick and simple way to describe things and generate
code from the description. That's all. Please don't be too dissapointed :-)
<P>


<P><HR><A NAME="synopsis"></A>
<H2> Synopsis </H2>

<I>fgen</I> automatically generates code from simple system
specifications. Currently <I>fgen</I> understands the following system 
specifications:
<UL>
<LI> <A HREF="#sm">State Machine</A>
</UL>

Fgen is designed to make it very easy to add new system specifications 
and parsers. 
<P>

<A NAME="setup"></A>
<H2> Setup </H2>

<H3>Getting fgen</H3>

Fgen is in <A HREF="../../../HomePages/Tools/index.html">Perforce</A> 
in directory <B>//depot/main/sw/tools/fgen</B>. To use fgen you must 
first sync fgen into your workarea.
<P>

<H3>Install Perl</H3>

Fgen is a perl script which requires Perl5 to be installed
on your system. Perl5 is located in directory <B>\\Lightera1\Apps</B>.
<P>

<H3>Path Setup</H3>

Fgen should be in your system's search path. Add the directory
fgen is in to your system's PATH environment variable. Environment
variables are set using <B>Start/Setting/System/Environment</B>.


<A NAME="cli"></A>
<H2> Command Line Invocation </H2>

<PRE>
perl fgen.pl files...
</PRE>

<H3> perl </H3>

Perl is a call to the perl program that should be installed.
If perl is not in your path then the full path to perl must
be specified on the command line.
<P>

<H3> fgen.pl </H3>

<B>fgen.pl</B> is the program that reads your specifications
and generates code.  If it is not in the current directory
then the full path to fgen.pl must be specfied.
<P>

<H3> files </H3>

<I>fgen</I> takes multiple files on its command line which means
specifications can be placed in several files and then processed
together. All files are first parsed and then code generated which means
if an attribute is redefined in a later file the definition of the later file 
is used.  If a the file is not local then the full path to the
file must be specified.
<P>


<I>fgen</I> pays no attention to file extensions so you can
organize files however you want. Suggested file extensions are:

<OL>
<LI> <B>.sm</B>   - state machine specification files 
</OL>
<P>

<H3> Example: </H3>
<PRE>
% perl D:\main\sw\tools\fgen\fgen.pl ImageDownload.sm
</PRE>

In the example the ++ state machine specified in ImageDownload.sm is 
generated. Perl is assumed to be in the path. ImageDownload.sm is
assumed to be local.
<P>


<P><HR><A NAME="syntax"></A>
<H2> Syntax </H2>

<I>fgen</I>'s syntax is very simple, it consists of:
<UL>
<LI> Blank line separated blocks. Lines <B>must</B> be separated by lines
     contaning new-lines only. Even spaces on the "blank" line can confuse
     the parser.
<LI> Where each block consists of one or more new-line separated lines.
<LI> Where each line consists of attributes and their values.
<LI> Where attributes are separated by spaces.
<LI> Blocks begin with a block type key word and a block name.
     The rest of lines in the block describe the system.
</UL><P>

All system specifications follow this basic syntax. Each
system must come up with a way to map the system to this syntax.
Attributes can be anything and values can be anything so it's not really 
difficult. Go <A HREF="#smexample"> here </A> to see an example.
<P>

<A NAME="bnf"></A>
<H2> fgen's BNF </H2>

<PRE>
(file)        ::= (specs)*
(specs)       ::= (block) | (cfg) | (sm) | (msg)

(block)       ::= (block_start) 
                  (block_entry)+
                  (blank_line)
(block_start) ::= (block_type) (ws) (block_name) (nl)
(block_type)  ::= (id) // a keyword identifying the block type
(block_name)  ::= (id) // name of the block
(block_entry) ::= (block_line) (nl)
(block_line)  ::= whatever grammar required for the block type

(dir_path)    ::= "(path)" | <(path)>
(inc_list)     ::= INC (ws) dir_path [, (ws) dir_path]*
(path)        ::= (id) // a relative or full path to a directory
(value)       ::= (id)
(id)          ::= a string with no embedded spaces
(nl)          ::= a carriage return
(blank_line)  ::= a blank line
(ws)          ::= white space
(number)      ::= a string of digits
(lighteraff)  ::= any series of characters
(boolean)     ::= 0 | 1
</PRE>


<P><A NAME="cfgbnf"></A>
<H2> CFG Block BNF </H2>

The CFG block allows the specification of general attributes
used in the generation of code. For example, the attribute DEBUG_LEVEL
specified the default debug level to use when generating debug
statements. You may have a lot of configuration attributes are
almost none.
<P>

<PRE>
(cfg)         ::= (cfg_start)
                  (cfg_entry)+ 
                  (blank_line)
(cfg_start)   ::= CFG (ws) (id) (nl)
(cfg_entry)   ::= (cfg_attr) (cfg_value) (nl)
(cfg_attr)    ::= (id)  // ID from the configurable attributes list
(cfg_value)   ::= whatever the value should be for the attribute
</PRE>
<P>

Any attribute not in the configurable attribute list is automatically added
even though it's not "official." This allows new parsers to have 
their own configurable attributes. All configurable attributes are
in the associative array <I>$::CFG{$attribute}</I>. A small example:
<PRE>
CFG Remote Peer State Machine
   DEBUG            1
   DRIVERS          lngen.pl
   INC              "Project/LnTypes.h"
   IGNORE_SAME_ENTRY_EXIT_TRANSITION 0
</PRE>
<P>


<P><A NAME="smbnf"></A>
<H2> State Machine BNF </H2>

A state machine is made up of SM block and one or more IN blocks.
The SM block defines the state machine in general and the IN blocks
define the events, state transitions, and the actions to take.

<H3> SM Block </H3>

The state machine block defines attributes about the state machine.
It is like a CFG block for the state machine.
<P>

<PRE>
(sm)          ::= (sm_start) 
                  (start_state) 
                  (sm_entry)+ 
                  (blank_line)
                  (in)+

(sm_start)    ::= SM (ws) (sm_name) (nl)
(sm_name)     ::= (id) // name of the state machine
(sm_entry)    ::= (case_err) | (inc_list) | (timer) | (is_protect)
                  | (state_type) | (sm_module) | (method) | (doc)
                  | (on_doc) | (on_args) | (do_args) | (do_doc) | (attr) | (debug)
				  | (derive) | (gen_as_string) | (gen_inject_event) 
				  | (gen_notify_call) | (gen_eforwarder)
(start_state) ::= START (ws) (id) (nl)
(case_err)    ::= CASE_ERR (ws) (lighteraff) (nl)
(is_protect)  ::= IS_PROTECT (ws) (boolean) (nl)
(state_type)  ::= STATE_TYPE (ws) (id) (nl)
(sm_module)   ::= IS_MODULE (ws) (boolean) (nl)
(doc)         ::= DOC (ws) (text) (nl)
(on_doc)      ::= ON_DOC (text) (nl)
(do_doc)      ::= DO_DOC (ws) (action) (ws) (text) (nl)
(on_args)     ::= ON_ARGS (ws) (event_name) (ws) ARGS (ws) (args) (nl)
(do_args)     ::= DO_ARGS (ws) (action_name) (ws) ARGS (ws) (args) (nl)
(method)      ::= METHOD (ws) (var_type) , (method_body) (nl)
(debug)       ::= DEBUG_LEVEL (ws) (level) 
(attr)        ::= ATTRIBUTE (ws) (var_type) , (var_name), (var_value) (nl)
(var_type)    ::= A type designation for a method attribute.
(var_name)    ::= Name of the attribute.
(var_value)   ::= Value for an attribute.
(text)        ::= A string of any length. The \ character can be used to
                  break a string across lines.
(level)       ::= A debug level approriate to the code generator.
(event_name)  ::= The name of an event.
(action_name) ::= The name of an action.
(derive)      ::= DERIVE (ws) (dclasses)
(dclasses)    ::= The classes to derive the state machine from. It's
                  a standard C++ derivation specification (public X).
(gen_as_string) ::= GEN_AS_STRING (ws) (0|1)
(gen_inject_event) ::= GEN_INJECT_EVENT (ws) (0|1)
(gen_notify_call) ::= GEN_NOTIFY_CALL (ws) (0|1)
(gen_eforwarder) ::= GEN_EFORWARDER (ws) (0|1)
(args)        ::= (var_type) (var_name) [, (var_type) (var_name)]

</PRE>

A small example is:
<PRE>
SM RemotePeerSm
   START            IDLE
   METHOD           virtual int, gt()
   ATTRIBUTE        int, mVal, 0
   ON_ARGS HealthCheckOk ARGS int x, int y
   DO_ARGS RepairOm ARGS int x, int y
   DOC This class remotely peers with a peer. \
More text of great use.

</PRE>
<P>

<H3> IN Block </H3>

The IN block defines what happens when an event is generate when the
state machine is <B>in</B> a given state. A state machine can
have any number of IN blocks. You'll have one IN block for every
event that transitions the state machine.
<P>

<PRE>
(in)          ::= (in_start) 
                  (which_sm)* (which_in)* (timer)* (transition)+ 
                  [(on_entry)] [(on_exit)]
                  (blank_line)

(which_sm)    ::= SM (ws) (which_state) [, (ws) (which_state)]* (nl)
(which_state) ::= (state_name) | CURRENT | GLOBAL

(which_in)    ::= IN [(doc)] | (in_list) 
(doc)         ::= DOC text
(in_list)     ::= (ws) (state_name) [, (ws) (state_name)]* (nl)

(in_start)    ::= IN (ws) (state_name) [, (state_name]+ (nl)
(transition)  ::= [NEXT | FNEXT] (ws) (state_name) (ws) ON (event) (ws) 
                  [(onerr) (ws)] [(if) (ws)] [(do) (ws)] (nl)
(do)          ::= DO (ws) (action) (ws) [(timer)][, (do)] |
                  DO (ws) (action) | (timer_stop) [, (do)]
(if)          ::= IF (ws) (test)
(onerr)       ::= ONERR (ws) PREVSTATE | (state_name) | (invoke) 
(timer_stop)  ::= TIMER_STOP (ws) (timer_name)
(timer)       ::= TIMED_BY (ws) (timer_name) 
                  PERIOD (dmsecs)[(imsecs)]
                  [ACTION TRUE|FALSE]
(key)         ::= identifier of one of the predefined configurable values
(dmsecs)      ::= number of delay milli-seconds | (method_name()).
                  Delay is the delay before the timer first fires.
(imsecs)      ::= number of interval milli-seconds | (method_name())
(method_name) ::= name of a method to call to the attribute value.
(test)        ::= a method returning an integer

(timer_name)  ::= (id)   // name of a timer
(action)      ::= (id)   // name of an action 
(event)       ::= (id)   // name of an event 
(state_name)  ::= (id)   // name of a state
(invoke)      ::= (id)() // function/method/macro name with parens
(on_entry)    ::= ON_ENTRY (do)
(on_exit)     ::= ON_EXIT (do)
</PRE>
<P>

A small example:<BR>
<PRE>
IN IDLE     
   DOC This state means we are rich.
   ON_ENTRY DO TIMER_STOP HealthTimer
   ON_EXIT  DO RepairOm  TIMED_BY HealthTimer PERIOD  3.4
   NEXT KNOWN      ON PeerStateMsgRcvd                                       \
      DO NewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON PeerFailed                                             \
      DO TIMER_STOP MsgTimer,                                                \
      DO FailedState

IN FAILED
   DOC This state means we have lost all our money.
   NEXT IDLE       ON PeerNotFailed
   NEXT KNOWN      ON PeerStateMsgRcvd IF IsPeerNotFailed()                  \
</PRE>
<P>


<A NAME="comment"></A>
<H2> <I>#</I> Comment Marker </H2> 

Lines beginning with <I>#</I> are comments.<P>

<A NAME="extender"></A>
<H2> <I>\</I> Line Extender </H2> 

Lines span more than one line by putting a <I>\</I>
at the end of the line.
<P>


<P><HR><A NAME="config"></A>
<H2> Configurable Attributes - CFG Block </H2>

The CFG block allows the specification of general attributes
used in the generation of code. For example, the attribute DEBUG_LEVEL
specified the default debug level to use when generating debug
statements. You may have a lot of configuration attributes are
almost none. 
<P>

The BNF is in section <A HREF="#cfgbnf">CFG BNF</A>.

<P>
A small example:
<PRE>
CFG Remote Peer State Machine
   DEBUG            1
   DRIVERS          lngen.pl
   INC              "Project/LnTypes.h"
</PRE>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="15%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="15%"> Default   </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DEBUG </TD>
   <TD ALIGN=CENTER> 0      </TD>
   <TD> Set the debug level: 
        <UL>
        <LI> 0 means off
        <LI> 1 means trace 
        <LI> 2 dumps parser data structures and more detailed info
        </UL>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> CCEXT </TD>
   <TD ALIGN=CENTER> cpp    </TD>
   <TD> C++ extension to use for C++ source files. 
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_CLASS </TD>
   <TD ALIGN=CENTER> Timer      </TD>
   <TD> Name of the timer class to use when generating timers.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_INC </TD>
   <TD ALIGN=CENTER> "Osencap/LnTTimer.h" </TD>
   <TD> Path to use in the include statement to include the timer class. 
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_IS_LNOBJECT </TD>
   <TD ALIGN=CENTER> 0      </TD>
   <TD> Controls if code is generated to use Destroy() to delete the timer.
        It should be used when timers are dirived from LnObjec.t
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_FUNC </TD>
   <TD ALIGN=CENTER> virtual int HandleTimer() </TD>
   <TD> Method invoked on timer callbacks.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_SET </TD>
   <TD ALIGN=CENTER> Start </TD>
   <TD> Method of the timer object to start a timer. This method must
        accept the argument:
        <UL>
        <LI> <I>int msecs</I>  - the number of milli-seconds between timer 
             firings.
        </UL>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMER_CANCEL </TD>
   <TD ALIGN=CENTER> Cancel       </TD>
   <TD> Method of the timer object that cancels a timer.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> MUTEX_CLASS </TD>
   <TD ALIGN=CENTER> LnMutex             </TD>
   <TD> Name of the mutex class.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> MUTEX_INC </TD>
   <TD ALIGN=CENTER> "Osencap/LnMutex.h" </TD>
   <TD> Path to use in the include statement to include the mutex class. 
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ERR_TYPE </TD>
   <TD ALIGN=CENTER> LnStatus          </TD>
   <TD> The type of the error return.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> NO_ERR </TD>
   <TD ALIGN=CENTER> LN_OK          </TD>
   <TD> Value use to indicate no error occurred.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> MSG_CLASS </TD>
   <TD ALIGN=CENTER> Msg               </TD>
   <TD> Name of the message class to use.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> INC </TD>
   <TD ALIGN=CENTER> none </TD>
   <TD> Automically includes the specified include file in every generated
        class definition. For example:<P> 
        <PRE>
        INC "Util/Log.h"
        </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> SEARCH_PATH </TD>
   <TD ALIGN=CENTER> <I>fgen</I>'s directory </TD>
   <TD> SEARCH_PATH is comma separated list of paths of where to 
        so search drivers, specified with the DRIVERS
        attribute. By default, the directory where fgen is located
        is added to the search path. For example:<P>
        <PRE>
        SEARCH_PATH /h/bin, /h2/someplace
        </PRE>
   </TD>
</TR>
<A NAME="driversattr"></A>
<TR>
   <TD ALIGN=CENTER> DRIVERS </TD>
   <TD ALIGN=CENTER> none </TD>
   <TD> DRIVERS is a comma separated list of paths to drivers. 
        Each driver is expected to generate appropriate code based
        on the specification. Paths should be full paths unless the module
        will be found in the perl library search path (the @INC variable).
        For example:<P>
        <PRE>
        DRIVERS /h/fgen.pl, yourfgen.pl
        </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> OVERWRITE </TD>
   <TD ALIGN=CENTER> 1 </TD>
   <TD> OVERWRITE controls if generated files overwrite
        existing files. Options:<P>
        <UL>
        <LI> 1 - means overwrite
        <LI> 0 - means don't overwrite
        </UL>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_ASSERT </TD>
   <TD ALIGN=CENTER> 0 </TD>
   <TD> GEN_ASSERT tells the code generator to generate assertions.
        By default code is generally generated without asserts.
        <UL>
        <LI> 1 - means generate assert statements
        <LI> 0 - means don't generate assert statements
        </UL>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_DEBUG </TD>
   <TD ALIGN=CENTER> 1 </TD>
   <TD> GEN_DEBUG tells the code generator to generate debug statements
        where it thinks they would make sense.
        <UL>
        <LI> 1 - means generate debug statements
        <LI> 0 - means don't generate debug statements
        </UL>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DEBUG_LEVEL <I>level</I> </TD>
   <TD ALIGN=CENTER> 
       Output::LEVEL_L2
   </TD>
   <TD> 
       The default debug level to use when creating debug statements.
       It can be any valid level.

      <P> 
      For example:

      <PRE>
      CFG Test
         DEBUG_LEVEL Output::LEVEL_L2
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IGNORE_SAME_ENTRY_EXIT_TRANSITION </TD>
   <TD ALIGN=CENTER> 
       0
   </TD>
    <TD> Decides if ON_ENTRY and ON_EXIT behaviours are called when transition
	     to the same state. Applications may have reasons for not wanting
		 these behaviours called on same state transitions.
        <UL>
        <LI> 1 - do not call exit and entry routines when transitioning
		         to the same state.
        <LI> 0 - always call exit and entry routines. The default.
        </UL>
   </TD>
</TR>
</TABLE><P>

See an <A HREF="#smexample"> example </A>.<P>



<P><HR><A NAME="sm"></A>
<H2> State Machine - SM and IN Blocks </H2>

The easiest way to understand how the state machine specification
works is to take a look at an <A HREF="#smexample">example</A>.
<P>

A state machine is made up of SM block and one or more IN blocks.
The SM block defines the state machine in general and the IN blocks
define the events, state transitions, and the actions to take.

A small example is:
<PRE>
CFG Remote Peer State Machine
   DEBUG            1
   DRIVERS          lngen.pl
   INC              "Project/LnTypes.h"

SM RemotePeerSm
   START            IDLE
   METHOD           virtual int, GetNormalMsgToVal()
   ATTRIBUTE        int, mVal, 0
   ON_ARGS HealthCheckOk ARGS int x, int y
   DO_ARGS RepairOm ARGS int x, int y
   DOC This class remotely peers with a peer. \
More text of great use.
   ON_DOC PeerStateMsgRcvd This event means a message was received.
   DO_DOC NewState This action does something. 

IN IDLE     
   DOC This state means we are rich. \
Let me tell you what i do all day.
   ON_ENTRY DO TIMER_STOP HealthTimer
   ON_EXIT  DO RepairOm  TIMED_BY HealthTimer PERIOD  3.4
   NEXT KNOWN      ON PeerStateMsgRcvd                                       \
      DO NewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON PeerFailed                                             \
      DO TIMER_STOP MsgTimer,                                                \
      DO FailedState

IN FAILED
   DOC This state means we have lost all our money.
   NEXT IDLE       ON PeerNotFailed
</PRE>
<P>



<P>
For the more technically minded state machine's are defined by 
<A HREF="#smbnf"> BNF </A>.
<P>

<A NAME="smdefined"></A>
<H2> Defined </H2>

A state machine consists of:
<UL>
<LI> list of states that can be entered
<LI> events causing movement from one state to another
<LI> actions performed when a state transition occurs
</UL>
<P>

<I>fgen</I> allows the above to be encoded simply and also provides some
value-add with the addition of timers. Operations can be timed. Timer objects
are automatically generated and managed.
<P>

A state machine is defined by two blocks:
<UL>
<LI> <A HREF="#smstart"> <I>SM</I> </A> defines the overall state machine. There is one
     <I>SM</I> block per state machine.
<LI> <A HREF="#smstate"> <I>IN</I> </A> defines each state in the state machine.
     There is one <I>IN</I> block per state in the state machine.
</UL><P>

Multiple state machines can be defined in the same file. By default states are 
associated with the current state machine which means the state machine
definition (<I>SM block</I>) they are lexically below in the file. <P>
Using the <I>SM</I> attribute of an <I>IN</I> block which defines state
a state may be associate with one or more state machines. <P>



<P><A NAME="msmexample">   
<H2> Motivating Example of All the Work You can Save </H2>

This example shows a fairly complicated state machine for
maintaining the state of a remote peer entity. Notice how
the state machine uses timers. The timers are automatically
generated and managed. <B>All the code implementing the state
machine is generated</B>. All you have to do is specify the
state machine.
<P>

To use the state machine you just derive from it, implement the
actions, and call the event methods when they occur. This tool
can save a considerable amount of effort, especially when a 
state machine is changing a lot. Agents and state machines
combined together are especially powerful. An agent can define
its state machine using fgen and then inherit from it. Messages
arriving to an agent are events that move the state machine
through its states. The actions are implemented by the agent
to do the agent's work.
<P>


<PRE>
# NAME:
#   RemotePeer.sm - state machine for Remote Peer stste
#
# SYNOPSIS:
#   fgen.pl RemotePeer.sm
#
# DESCRIPTION:
#   This file describes an example state machine for maintaining the
#   state of a remote peer entity.
#
#   Running fgen on this file will create the C++ class implementing 
#   the state machine.
#
# SEE ALSO: fgen
#
# TYPE: SPECIFICATION
#########################################################################

CFG Remote Peer State Machine
   DEBUG            1
   DRIVERS          lngen.pl
   INC              "Project/LnTypes.h"

SM RemotePeerSm
   START            IDLE
   METHOD           virtual int, gt()
   METHOD           virtual bool, IsPeerFailed()
   METHOD           virtual bool, IsPeerNotFailed()
   METHOD           virtual int, GetNormalMsgToVal()
   ATTRIBUTE        int, mVal, 0
   ON_ARGS          HealthCheckOk ARGS int x, int y
   DO_ARGS          RepairOm ARGS int x, int y
   IS_MODULE        1

IN IDLE     
   ON_ENTRY DO TIMER_STOP HealthTimer
   ON_EXIT  DO RepairOm  TIMED_BY HealthTimer PERIOD  3.4
   NEXT KNOWN      ON PeerStateMsgRcvd                                       \
      DO NewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON PeerFailed                                             \
      DO TIMER_STOP MsgTimer,                                                \
      DO FailedState
   NEXT FAILED     ON Entry IF IsPeerFailed() DO FailedState
   NEXT IDLE       ON Entry                                                  \
      DO UnknownState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT OVERDUE    ON MsgTimerFire                                           \
      DO Nothing TIMED_BY MsgTimer PERIOD getOverdueMsgToVal()

IN FAILED
   NEXT IDLE       ON PeerNotFailed
   NEXT KNOWN      ON PeerStateMsgRcvd IF IsPeerNotFailed()                  \
      DO NewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()

IN KNOWN
   NEXT KNOWN      ON PeerStateMsgRcvd                                       \
      DO CheckForNewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON Entry IF IsPeerFailed() DO FailedState
   NEXT IDLE       ON Entry                                                  \
      DO UnknownState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON PeerFailed                                             \
      DO TIMER_STOP MsgTimer,                                                \
      DO FailedState
   NEXT OVERDUE    ON MsgTimerFire                                           \
      DO Nothing TIMED_BY MsgTimer PERIOD getOverdueMsgToVal()

IN OVERDUE
   NEXT KNOWN      ON PeerStateMsgRcvd                                       \
      DO CheckForNewState TIMED_BY MsgTimer PERIOD GetNormalMsgToVal()
   NEXT FAILED     ON PeerFailed                                             \
      DO TIMER_STOP MsgTimer,                                                \
      DO FailedState
   NEXT FAILED    ON MsgTimerFire                                           \
      DO FailedState
</PRE>




<P><A NAME="smfiles"></A>
<H2> Files Produced </H2>

The following files are produced:
<UL>
<LI> A state machine header file. The filename is based on the value of attribute
     <B>SM</B> in the SM block start.
<LI> A state machine source file. The filename is based on the value of attribute
     <B>SM</B>. The extension is defined by attribute <B>CCEXT</B>.
</UL>
<P>


<A NAME="smbs"></A>
<H2> SM Definition (SM block)</H2>

A state machine block starts when the keyword <I>SM</I> is seen at the
beginning of a line. State machine blocks describe attributes about a
state machine. Other <A HREF="#inbs"> blocks </A> describe each state 
in the state machine. <P>

<H3> State Machine Start Block </H3>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="30%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> SM <I>name</I> </TD>
   <TD>This is block start for the <I>SM</I> block. <I>name</I> is the 
       name of the state machine. The generated source files are named 
       after <I>SM</I>. Attributes for <I>SM</I> are defined in their
       own <A HREF="#smattr"> table </A>. <P>

       For example:
       <PRE>
       SM DownloadSm
       </PRE>
   </TD>
</TR>
</TABLE><P>

<A NAME="smattr"></A>
<H3> State Machine Attributes </H3>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="25%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="5%">  Default   </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> START <I>state</I> </TD>
   <TD ALIGN=CENTER> required </TD>
   <TD> 
      Initial state of the machine. A required field for state machines. <P>
      For example:
      <PRE>
      SM Test
         START IDLE
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> INC <I>inc_list</I> </TD>
   <TD ALIGN=CENTER>NA</TD>
   <TD>
      A list of files to include in the statema machine header.
      Usually files are included to resolve any user defined types.
       
      For example:
      <PRE>
      SM Test
         INC  "A.h", "B.h"
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IS_PROTECT <I>1 | 0</I> </TD>
   <TD ALIGN=CENTER>0, no Mutex</TD>
   <TD>
      If IS_PROTECT is set to <I>1</I> then all state machine 
      transitions are protected by a mutex. The default is no 
      mutex protection.<P>
       
      For example:
      <PRE>
      SM Test
         IS_PROTECT 1
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ATTRIBUTE <I>var_type, var_name, var_value</I> </TD>
   <TD ALIGN=CENTER>0</TD>
   <TD>
      ATTRIBUTE specifies a variable to make part of the state machine class.
      For each attribute in a state machine there will be a corresponding
      attribute in the state machine class definition. The variable will have
      type <I>var_type</I> and variable name <I>var_name</I>. The variable
      will be initialized in the constructor to value <I>var_value</I>.
      This allows the state machine to be better separated from its derived
      class.
       
      For example:
      <PRE>
      SM Test
         ATTRIBUTE int, mVal, 0
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ON_ARGS <I>event_name</I> ARGS </I>(args)</I> </TD>
   <TD ALIGN=CENTER></TD>
   <TD>
      ON_ARGS allows an event to be called with arguments. The arguments will 
      be passed to any actions defined as taking parameters (see DO_ARGS). 
      This allows, for example, a message to be passed to an action directly 
      without the message having to be stored in a object variable.
       
      For example:
      <PRE>
      SM Test
         ON_ARGS HealthCheckOk ARGS int x, int y
      </PRE>

      The HealthCheckOk method will be defined as taking argument "int x" and 
      "int y".
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DO_ARGS <I>action_name</I> ARGS </I>(args)</I> </TD>
   <TD ALIGN=CENTER></TD>
   <TD>
      DO_ARGS allows an action to be called with arguments. Arguments will be
      passed from an event to the action. The variable names have to be the
      same in the event and in the action.
       
      For example:
      <PRE>
      SM Test
        ON_ARGS HealthCheckOk ARGS int x, int y
        DO_ARGS RepairOm ARGS int x, int y

      IN OM_FAILED
         NEXT OM_PRESENT ON HealthCheckOk                 \
            DO RepairOm                                   \
               TIMED_BY HealthTimer PERIOD  3.4                              
      </PRE>

      In this example the values passed to HealthCheckOk will be passed
      in turn to RepairOm.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> STATE_TYPE <I>id</I> </TD>
   <TD ALIGN=CENTER>State</TD>
   <TD>
      STATE_TYPE allows you to replace a state machine's automatically
      generated state enumeration with your own preexisting state type. 
      You must make sure the enum labels in your state machine match 
      the labels in your enum. An INC statement must be used to
      to include a file that will resolve the enum type.
      <P>

      For example:
      <PRE>
      SM Test
         STATE_TYPE       OperationStateType
      </PRE>

   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMED_BY </TD>
   <TD ALIGN=CENTER>NA</TD>
   <TD>
      A timer may be defined that is not used explicitly by any
      transition. Code for the timer is generated. Such timers must be
      manually managed by the programmer using the start and stop timer
      methods. Please see <A HREF="#statetrans"> TIMED_BY </A>
      for more details on the the syntax. <P>

      Timers defined in the state machine block do not generate an action
      method in the state machine by default. This is because it might conflict
      with an event used in an <I>ON</I> clause in a transition. To make
      TIMED_BY generate an action method use the <I>ACTION</I> clause and set 
      it to true. When you do this the method <I>timer_name</I> + <I>Fire</I>
      is added to the state machine's interface and will be called whenever
      the timer fires. WARNING: you cannot then use <I>timer_name</I> +
      <I>Fire</I> event in a transition <I>ON</I> clause or the state machine
      won't compile. <P>

      For example:
      <PRE>
      SM Test
         TIMED_BY TestTimer PERIOD 10 ACTION TRUE
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IS_MODULE <I>boolean</I> </TD>
   <TD ALIGN=CENTER> 
      0, not a module
   </TD>
   <TD> 
      A state machine can be made part of a module when the IS_MODULE
      keyword is set to 1. When set the state machines constructor
      will take a module pointer as an argument. Debug macros are 
      passed the module pointer.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> METHOD <I>type, body</I> </TD>
   <TD ALIGN=CENTER> 
      NA
   </TD>
   <TD> 
      User defined methods can be easily added into the state machine
      and used as timer value accessors or if tests. The method may be
      virtual or completely inline.
      <P> 
      <I>type</I> is the return type of the method. <BR>
      <I>body</I> body is method name and parameter list. It can be
      just the method name or a complete inlined method.  <BR>

      <P> 
      For example:

      <PRE>
      SM Test
         METHOD int,  GetTimer(void) { return 5; } 
         METHOD virtual int,  AnotherTimer(void)
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DOC <I>text</I> </TD>
   <TD ALIGN=CENTER> 
      NA
   </TD>
   <TD> 
      Documentation for the state machine class. It is included in a comment 
      block. <I>text</I> is a string of any length. The \ character can be used
      to break comments across lines.
      <P> 
      For example:

      <PRE>
      SM Test
         DOC Some example documentation. \
This is broken on another line.
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ON_DOC <I>event</I> <I>text</I> </TD>
   <TD ALIGN=CENTER> 
      NA
   </TD>
   <TD> 
      Documentation for an event. It is included in a comment block
      for the event. <I>event</I> is the name of an event in the state machine.
       <I>text</I> is a string of any length. The \ character can be used
      to break comments across lines.
      <P> 
      For example:

      <PRE>
      SM Test
         ON_DOC Event1 Some example documentation. \
This is broken on another line.
         ON_DOC Event2 Some example documentation. \
This is broken on another line.
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DO_DOC <I>action</I> <I>text</I> </TD>
   <TD ALIGN=CENTER> 
      NA
   </TD>
   <TD> 
      Documentation for an action used an a DO clause. It is included in a 
      comment block for the action. <I>action</I> is the name of the action
      used in a DO clause. <I>text</I> is a string of any length. The \ 
      character can be used to break comments across lines.
      <P> 
      For example:

      <PRE>
      SM Test
         DO_DOC Do1 Some example documentation. \
This is broken on another line.
         ON_DOC Do2 Some example documentation. \
This is broken on another line.
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DEBUG_LEVEL <I>level</I> </TD>
   <TD ALIGN=CENTER> 
       Output::LEVEL_L2
   </TD>
   <TD> 
       The debug level to use when creating debug statements for the state machine.
       It can be any valid level. It defaults to the level specified in CFG.

      <P> 
      For example:

      <PRE>
      SM Test
         DEBUG_LEVEL Output::LEVEL_L2
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> CASE_ERR <I>code</I> </TD>
   <TD ALIGN=CENTER> 
      D(XMOD, XLVL,  \"UNHANDLED: EVENT=event STATE=\" << CurrentState() <<\"\\n\");\n\t\trc= LN_FAIL;\n";
   </TD>
   <TD> 
      Code fragment to use in the default case of a state switch for
      an event. The string <I>event</I> in the fragment is replaced with 
      the current event name.  The string <I>XMOD</I> is replace with the name
      of the module. The string <I>XLVL</I> is replaced with the current debug level.
      <P>

      <I>"</I> and <I>\n</I> must be quoted as this 
      line is outputted via a print statement.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DERIVE (ws) (dclasses) </TD>
   <TD ALIGN=CENTER> 
       None.
   </TD>
   <TD> 
      This directive specifies classes the state machine should derive from.
	  It allows state machines to have base classes.
      <P>

       For example:

      <PRE>
      SM Test
         DERIVE public BaseClass, AnotherClass
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_AS_STRING (ws) (0|1) </TD>
   <TD ALIGN=CENTER> 
       0
   </TD>
   <TD> 
      Causes the method <B>CurrentStateName</B> to be generated. This method
	  returns the current state as a string value. The value <I>0</I> means
	  don't generate the method. This is the default. The value <I>1</I>
	  means generate the method.

      <PRE>
      SM Test
         GEN_AS_STRING 1
      </PRE>
      <P>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_EFORWARDER (ws) (0|1) </TD>
   <TD ALIGN=CENTER> 
       0
   </TD>
   <TD> 
      Causes the pure virtual method <B>FwdEvent</B> to be generated. 
	  It also causes GEN_INJECT_EVENT to be defined. The InjectEvent
	  method instead of calling events directly, creates an Action object that 
	  when its Doit method is invoked, causes the correct state machine event 
	  method to be invoked. The Action object is passed to FwdEvent for
	  forwading to a thread context that will invoke the Doit method in its
	  thread context. The Doit method calls the correct event method and
	  thus causes state changes in the state machine. State machine actions
	  invoked because of the event will be executed in the thread context.
	  
	  <P>
	  The value <I>0</I> means don't generate the method. This is the default. 
	  The value <I>1</I> means generate the method.

      <P>
      <PRE>

      SM Test
         GEN_EFORWARDER 1
      </PRE>
      <P>

   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_NOTIFY_CALL (ws) (0|1) </TD>
   <TD ALIGN=CENTER> 
       0
   </TD>
   <TD> 
      Causes the pure virtual method <B>SmChangedEvent</B> to be generated. 
	  This method is called by the state machine whenever the state machine 
	  changes state. Derived class are exected to implement this method
	  and do whatever is appropriate when the state changes.
	  
	  <P>
	  The value <I>0</I> means don't generate the method. This is the default. 
	  The value <I>1</I> means generate the method.

      <P>
      <PRE>

      SM Test
         GEN_NOTIFY_CALL 1
      </PRE>
      <P>

   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> GEN_INJECT_EVENT (ws) (0|1)</TD>
   <TD ALIGN=CENTER> 
       0
   </TD>
   <TD> 
      Causes the method <B>InjectEvent</B> to be generated. 
	  This method can be used to trigger an event using only the event
	  name. Normally an event is triggered by calling the method
	  corresponding to the event. InjectEvent will take the event name
	  and invoke the correct event method. No arguments can be passed
	  to events when this option is used.
	  
	  <P>
	  The value <I>0</I> means don't generate the method. This is the default. 
	  The value <I>1</I> means generate the method.

      <P>
      <PRE>

      SM Test
         GEN_INJECT_EVENT 1
      </PRE>
      <P>

   </TD>
</TR>
</TABLE><P>


See an <A HREF="#smexample"> example </A> <I>SM</I> block.<P>


<A NAME="smstate"></A>
<H2> State Definition (IN block) </H2>

Each state is defined by a state block which includes:
<UL>
<LI> the name of the state
<LI> one line each for each state the current state can transition to
<LI> a blank line for separating state blocks
</UL>
<P>

<H3> State Start Block </H3>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="30%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IN <I>state_name [, state_name]+ </I> </TD>
   <TD> 
      This is the block start for the state definition.
      <I>state_name</I> is the name of the state. An <I>IN</I> statement
      may have multiple state names which means the transitions will be
      executed if any of the specified states are the current state.
      <P>

      After <I>block start</I> what follows is a number of lines defining the 
      states that can be transitioned to from this current state. Attributes
      describing a transition are defined in their own 
      <A HREF="#stateentries"> table </A>. <P>

      For example:
      <PRE>
         IN IDLE
      </PRE>
    or
      <PRE>
         IN IDLE, BLACKOUT, WORLD_ENDS
      </PRE>
   </TD>
</TR>
</TABLE><P>

<A NAME="stateentries"></A>
<H3> State Entries </H3>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="30%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> SM <I>which_sm</I> </TD>
   <TD>
      <I>SM</I> specifies which state machines a state is associated with.
      <I>which_sm</I> is a comma separated list of state machines names
      for which the state definition should be apart. <P>

      If <B>no</B> <I>SM</I> attribute is specified then the state is associated
      with the current state machine. The current state machine is the
      state mechine the state definition is lexigraphically under. <P>

      If <B>an</B> <I>SM</I> attribute is specified then the state is associated
      with only the specified state machine names in <I>which_sm</I>. 
      The token <I>CURRENT</I> is a predefined value which means include this state 
      for the current state machine as well as any other state machines defined
      on <I>which_sm</I>. <P>

      More than one <I>SM</I> attribute may be defined per state. <P> 

      One common use for <I>SM</I> is the ability to define a common set
      of states that should be in a set of state machines. <P>

      The state machine name <I>GLOBAL</I> is a predefined special state machine
      name. When a state is associated with state machine <I>GLOBAL</I> it 
      makes all the state's transitions available for inclusion any other state 
      in any other state machine. See the <I>IN</I>attribute in this table for
      more information.

      For example:
      <PRE>
      SM DownloadSm
   or
      SM TestSm, CURRENT
   or
      SM GLOBAL
   or
      do not specify one 
      </PRE>
    </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IN <I>which_in</I> </TD>
   <TD>
      <I>IN</I> specifies the global state definitions to merge into this one.
      <I>which_in</I> is a comma separated list of state names in the global
      state machine that are to be merged into this one. See attribute <I>SM</I>
      in this table for more information. <P>

      Globally available states allow a common set of transitions to be
      defined once and then reused many times in other states. If every state
      must respond to a power off event, for example, then the transitions for
      this event can be defined once and reused in all other states. 

      For example:
      <PRE>
      IN Common, MoreCommon
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> NEXT <I>state_name</I> </TD>
   <TD>
      Specifies the next state to transition to when the specified event 
      occurs. <I>NEXT</I> is defined in its own 
      <A HREF="#statetrans"> table </A>.
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> TIMED_BY </TD>
   <TD>
      A timer may be defined that is not used explicitly by any
      transition. Code for the timer is generated. Such timers must be
      manually managed by the programmer using the start and stop timer
      methods. Please see <A HREF="#statetrans"> TIMED_BY </A>
      for more details on the the syntax. <P>
   </TD>
</TR>
</TABLE>


<A NAME="statetrans"></A>
<H3> State Transitions </H3>

<TABLE BORDER=1 WIDTH="100%">
<TR>
   <TD ALIGN=CENTER WIDTH="30%"> Attribute </TD>
   <TD ALIGN=CENTER WIDTH="70%"> Meaning   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> NEXT <I>state_name</I> </TD>
   <TD>
      Specifies the next state to transition to when the specified event 
      occurs. The next state must exist in the state machine which means it
      must have its own definition. <P>

      For example:
      <PRE>
         NEXT INFO
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> FNEXT <I>state_name</I> </TD>
   <TD>
      <I>FNEXT</I> stands for <I>forced next</I> and is the same as <I>NEXT</I> except 
      the state is <B>ALWAYS</B> set to <I>state_name</I>. <I>FNEXT</I> is used when an 
      <I>IF</I> clause is present as an <I>IF</I> clause makes the state transition 
      conditional. Sometimes we want
      the state to always change but have the action only be executed when
      the <I>IF</I> condition is true.<P>

      For example:
      <PRE>
         FNEXT ADMIN_STATE_OFFLINE ON MoveOffline  IF IsGoOffline DO GoOffline
      </PRE>
   </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ON <I>event</I> </TD>
   <TD>
      Specifies the event that must occur to cause the state transition. A method
      is created in the state machine class named directly after <B>event</B>. When
      the event occurs the event's method must be called to trigger the state
      transitions. The state machine can't know when events occur, code using the
      the state machine must interpret events. <P>

      When timers are used an event is created named after the timer: 
      the <I>timer name</I> + <I>Fire</I>.
      You can make use of the timer events in the ON specification.<P>

      For example:
      <PRE>
      NEXT INFO     ON AcqSuccess
   or
      NEXT INFO     ON YourTimerNameFire
      </PRE>
    </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> ONERR <I>PREVSTATE | (state_name) | (invoke)</I> </TD>
   <TD>
      ONERR specifies what should happen when an action called during
      a state transition fails. As there is no one prefered approach
      to handling action failures several options are provided. Remember,
      a state machine's state has been set to target state of a 
      transition before any actions are called. So by default
      when an action returns an error the state transition exits  
      immediately remaining in the current state. <P>
   
      Your options for dealing with action failures are: <P> 

      If you want the state machine to revert back to the state before
      this current transition then use the keyword PREVSTATE. <P>

      If you want a function/method/macro called then specify a name
      with "()" in it, for example: dosomething(). It's up to you to
      make sure the call is resolved somehow. <P>

      If you want the state machine to enter another state then specify
      the state name. Of course the state specified must exist in the
      state machine. <P>


      For example:
      <PRE>
      NEXT INFO     ON AcqSuccess  ONERR PREVSTATE
    or
      NEXT INFO     ON AcqSuccess  ONERR EndWorld()
    or
      NEXT INFO     ON AcqSuccess  ONERR IDLE
      </PRE>
    </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> IF <I>test</I> </TD>
   <TD>
      The IF clause places a conditional on the state transition.
      The <I>test</I> method is implemented by a class.
      <I>test</I> is called before state transition happens.
      If <I>test</I> returns 0 then the transition will 
      not happen. Any non-zero value will cause the transition 
      to occur.<P>

      For example:
      <PRE>
      NEXT INFO     ON AcqSuccess IF IsStable
   or
      NEXT INFO     ON YourTimerNameFire IF IsStable
      </PRE>
    </TD>
</TR>
<TR>
    <TD ALIGN=CENTER> DO <I>action</I> </TD>
    <TD>
       The action to perform on the state transition. More than one DO clause 
       can be specified by comma separating them. A pure virtual function is 
       defined for each action in the state machine. Classes are expected to 
       derive from the state machine class and implement the actions. See the 
       generated header files for the actions to implement. Actions are called 
       as events arrive and trigger state transitions. <P>

       For example:
       <PRE>
       NEXT INFO    ON AcqSuccess   DO SendDnldInfoReq
    or
       NEXT INFO    ON AcqSuccess   DO SendDnldInfoReq, \
                                    DO NothingMuch
       </PRE>
    </TD>
</TR>
<TR>
    <TD ALIGN=CENTER> ON_ENTRY <I>do</I> </TD>
    <TD>
       ON_ENTRY allows actions to be performed every time a state is entered. 
       Actions are specified using the DO clause. Actions are performed
       before transition actions are performed.

       For example:
       <PRE>
       ON_ENTRY     DO TIMER_STOP HealthTimer
     or
       ON_ENTRY     DO SendDnldInfoReq, \
                    DO NothingMuch
       </PRE>
    </TD>
</TR>
<TR>
    <TD ALIGN=CENTER> ON_EXIT <I>do</I> </TD>
    <TD>
       ON_EXIT allows actions to be performed every time a state is exited. 
       Actions are specified using the DO clause. Actions are performed
       before transition actions are performed.

       For example:
       <PRE>
       ON_EXIT     DO TIMER_STOP HealthTimer
     or
       ON_EXIT     DO SendDnldInfoReq, \
                    DO NothingMuch
       </PRE>
    </TD>
</TR>
<TR>
    <TD ALIGN=CENTER VALIGN=TOP> 
              TIMED_BY <I>timer_name</I>
              PERIOD <I>(dmsecs)[.(imsecs)]</I>
    </TD>
    <TD>
       Creates a timer to time the action specified in the <I>DO</I> clause. 
       Timers not created in a state transition context should ignore the 
       DO clause explanations.
       <P>
 
       A timer object is automatically created using the name 
       <I>timer_name</I> as the class name. <P>

       Timers fire at a specified period defined by the <I>PERIOD</I> attribute. 
       The period description needed depends on the type of timer. 
       The <I>PERIOD</I> clause must be defined for every <I>TIMED_BY</I> 
       clause.
       This allows using the same timer with different periods depending on the
       context.

       One shot timers need to specify the <I>dmsecs</I> parts of 
       <I>PERIOD</I>, which is the delay milli-seconds. The delay is 
       the delay before the timer first fires. <P>

       Cyclic timers need to specify dmsecs and <I>imsecs</I>.
       The interval between timer firings is specified 
       using <I>imsecs</I> which is the interval milli-seconds.
      
       Don't confuse delay and interval times. Delay is the time before the
       first time a timer fires. Interval is the period between firings after
       the first timer fires. <P>
       
       The same timer can be used by multiple <I>DO</I> clauses, but only 
       one can use it at a time. <P>

       Times in the <I>PERIOD</I> attribute can be specified using a 
       simple number like <I>10</I> or a method call like <I>GetSecs()</I>. 
       The method is expected to return the value of the parameter. You are 
       expected to provide an
       a method and implementation using the <I>ATTRIBUTE</I> and
       <I>METHOD</I> features. <P>

       When a timer fires it generates the event <I>timer</I> + <I>Fire</I>. So
       a timer named <I>Retry</I> would generate the event <I>RetryFire</I>
       when the timer expires. <P>

       Timers causes the following code to be generated:
       <UL>
       <LI> a timer class
       <LI> start and stop methods for each timer
       </UL><P>

       For example:
       <PRE>
       NEXT INFO  ON Success  DO SendInfoReq \
          TIMED_BY InfoReqTimer PERIOD 5
    or
       NEXT INFO  ON Success  DO SendInfoReq \
          TIMED_BY InfoReqTimer PERIOD GetMsecs()
    or
       SM LedSm
          START        IDLE
          TIMED_BY     FlashTimer PERIOD 0.10 
       </PRE>
     </TD>
</TR>
<TR>
   <TD ALIGN=CENTER> DOC <I>text</I> </TD>
   <TD ALIGN=CENTER> 
      NA
   </TD>
   <TD> 
      Documentation for the state. It is included in a comment block for
      the state.  <I>text</I> is a string of any length. The \ character 
      can be used to break comments across lines.
      <P> 
      For example:

      <PRE>
      IN Test
         DOC Some example documentation. \
This is broken on another line.
      </PRE>
   </TD>
</TR>
</TABLE><P>


<A NAME="smcode"></A>
<H2> Implementing the State Machine </H2>

<H3> Generated Files </H3>

>From a state machine specification a state machine header file
and implementation file are generated. The header file is divided
in to sections. 
<OL>
<LI> <B>Timer Classes</B><BR>
A class is generated for each timer specified.

<LI><B>State Machine Class</B><BR>
Next is the state machine class. In the state machine class:

   <OL>
   <LI> <B>State Enum</B><BR>
   An enum for every state in your state machine.

   <LI> <B>Event Methods</B><BR>
   A method for every event driving your state machine. 
   Events are generated from the <B>ON</B> clauses in your
   state machine.

   <LI> <B>Action Methods</B><BR>
   A pure virtual function for every action in your state machine. 
   Actions are generated from the <B>DO</B> clauses in your
   state machine. Events cause action methods to be called.

   <LI> <B>Timer Methods</B><BR>
   A start and stop method is generated for every timer. This allows
   you to stop and start the timer in your state machine actions.
   </OL>
</OL>

<H3> Derive a Class </H3>

State machines are driven by events. As events come into your
system, from messages, interrupts, or some other source, you
must invoke the corresponding event method in the state machine
object. Events are coded to call the action methods you specified
and change states appropriately. 
<P>

The action methods are virtual. You must derive a class
from the state machine class and implement the action methods.
Its in the action methods that the behaviour of the state machine
is coded.


<A NAME="smexample"></A>
<H2> State Machine and Agent Example </H2>

A combined agent and state machine example is provided
in the 
<A HREF="http://LighteraNet/HomePages/Software/Infrastructure/Com.html#example_agent">Com</A> documentation.
<P>


<P><HR><A NAME="bugs"></A>
<H2> Bugs </H2>

<UL>
<LI> The error handling could be improved.  A lot errors just don't generate
     good code.
</UL>
<P>


<P><HR><A NAME="files"></A>
<H2> Files </H2>

<UL>
<LI> *.sm    - state machine specification file
</UL>
