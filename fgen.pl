#!/home4/thoff/bin/perl
# NAME:
#   fgen.pl - generate code from system specifications
#
# DESCRIPTION:
#   Currently the following systems can be automatically
#   generated from a specification:
#   1. state machines
#
# FILES:
#   *.sm  - state machine specification
#
# TYPE: PERL-SCRIPT
##############################################################################

use File::Basename;

BEGIN 
{
   # add source directory to @INC
   ($DIRPATH, $basename)= ($0 =~ m#(.*/)?(.*)#);
   unshift(@INC, $DIRPATH);
}


# Setup Environment
#
$/                 = "";          # slurp up blank line separated records


# Configurable Parameters. See man page for an explanation of each parameter.
#
$CFG{DEBUG}             = 2;  
$CFG{IS_STDOUT}         = 0; 
$CFG{INLINE}            = "";    
$CFG{CCEXT}             = "cpp";
$CFG{TIMER_INC}         = "\"Osencap/LnTTimer.h\"";
$CFG{TIMER_FUNC}        = "virtual void HandleTimer()";
$CFG{TIMER_CLASS}       = "LnTTimer";
$CFG{TIMER_SET}         = "Start";
$CFG{TIMER_CANCEL}      = "Cancel";
$CFG{MUTEX_CLASS}       = "LnMutex";
$CFG{MUTEX_INC}         = "\"Osencap/LnMutex.h\"";
$CFG{ERR_TYPE}          = "LnStatus";
$CFG{NO_ERR}            = "LN_OK";
$CFG{IS_ERR}            = "LN_FAIL";
$CFG{MSG_CLASS}         = "Msg";
$CFG{MSG_INC}           = "\"Com/Msg.h\"";
$CFG{DEBUG_INC}         = "\"Util/Debug.h\"";
$CFG{MODULE_INC}        = "\"Util/Module.h\"";
$CFG{MODULE_CLASS}      = "Module";
$CFG{DEBUG_LEVEL}       = "Output::LEVEL_L2";
$CFG{DEBUG_FMT}         = "D(XMOD, XLVL, XDATA);";
$CFG{DRIVERS}           = "";
$CFG{CGI_PARSER_DRIVERS}= "";
$CFG{CGI_CODE_DRIVERS}  = "";
$CFG{OVERWRITE}         = "1";
$CFG{GEN_ASSERT}        = "0";
$CFG{GEN_DEBUG}         = "1";
$CFG{INC}               = "";
$CFG{SEARCH_PATH}       = ""; 
$CFG{FGEN_PAGE}         = "http://LighteraNet/main/sw/tools/fgen/fgen.html";
$CFG{IGNORE_SAME_ENTRY_EXIT_TRANSITION}= "0";

# Put the directory fgen.pl is in, in the search path.
# Drivers may reside it that directory.
#
unshift(@INC, dirname($0)); 

# Initialize Installable Drivers
#
@CODE_DRIVERS          = ();  # driver interfaces for code generation
$PARSER_DRIVERS        = {};  # driver interfaces for parsers
@ALL_DRIVERS           = ();  # all driver interfaces


# Create default parsers. The parser driver interface serves to 
# map parsers for both blocks and configurable attributes.
#

##
# Default parser for the configuration block.
#
$cfg_parser              = {};
$$cfg_parser{NAME}       = "CFG Block Parser";
$$cfg_parser{ACTION}     = \&parse_cfg_block;
install_parser_driver("CFG", $cfg_parser);

##
# Default parser for state machines.
#
$sm_parser               = {};
$$sm_parser{NAME}        = "State Machine Block Parser";
$$sm_parser{ACTION}      = \&parse_sm_block;
$$sm_parser{POST_ACTION} = \&parse_post_sm_block;
install_parser_driver("SM", $sm_parser);

$agent_parser               = {};
$$agent_parser{NAME}        = "State Machine Block Parser";
$$agent_parser{ACTION}      = \&parse_agent_block;
install_parser_driver("AGENT", $agent_parser);

$in_parser               = {};
$$in_parser{NAME}        = "State Block Parser";
$$in_parser{ACTION}      = \&parse_in_block;
install_parser_driver("IN", $in_parser);

##
# Default parser for messages.
#
$msg_parser              = {};
$$msg_parser{NAME}       = "Message Block Parser";
$$msg_parser{ACTION}     = \&parse_msg_block;
install_parser_driver("MSG", $msg_parser);

##
# Default parser for config parameters that have multipe value entries
#
$cfg_line_parser         = {};
$$cfg_line_parser{NAME}  = "CFG Line Parser for TIMER_FUNC, INC";
$$cfg_line_parser{ACTION}= \&parse_cfg_line;
install_parser_driver("TIMER_FUNC", $cfg_line_parser);
install_parser_driver("INC", $cfg_line_parser);

##
# Default parser for the DRIVERS attribute.
#
$cfg_drivers_parser         = {};
$$cfg_drivers_parser{NAME}  = "CFG DRIVERS Parser";
$$cfg_drivers_parser{ACTION}= \&parse_cfg_drivers;
install_parser_driver("DRIVERS", $cfg_drivers_parser);

##
# Default parser for the SEARCH_PATH attribute.
#
$cfg_search_path_parser         = {};
$$cfg_search_path_parser{NAME}  = "CFG SEARCH_PATH Parser";
$$cfg_search_path_parser{ACTION}= \&parse_cfg_search_path;
install_parser_driver("SEARCH_PATH", $cfg_search_path_parser);


##
# Misc.
#
$SMNAME= "";      # temporary name for the current state machine

##
# Generate code from specifications contained in files passed
# on the command line.
#
gen_code();


################################### FUNCTIONS #################################


sub gen_code
#
# Take all steps necessary to generate code.
#
{
   ## 
   # Parse the specification files. Our default parsers will be called as well
   # as other user defined parsers.
   #
   parse_spec();

   ##
   # Call all POST_ACTION methods for parsers. This allows them to
   # make changes to the parser datastructers based on global
   # knowledge.
   #
   post_parser_drivers();

   #dump_parse_tree() if $CFG{DEBUG} > 1;
   #dump_sm(); exit;
   #dump_sm();
   #dump_agent();
   #dump_msg();

   ##
   # After parsing invoke all code drivers and then close all drivers
   # as we are done.
   #
   code_drivers();
   post_code_drivers();
   close_drivers();

}# gen_code


################################ PARSE FUNCTIONS #############################


sub parse_spec
#
# Loop through all files on the command line:
# 1. Read in records. Because we set the record separator to "" records are 
#    considered to be blank separated chunks of text. 
# 2. look for key words in the syntax so we can dispatch the record to a
#    a parser handler.
#
{
   my(@block, $attribute, $value);

   TOP: while (<>) 
   {
      @block= squish($_);              # format block into an array

   ENTRIES:
      next TOP unless $block[0];       # next block if no lines left, happens
                                       # when skipping past comments

      if ($block[0]=~ /^#/)            # skip comment lines
      {
         shift @block;
         goto ENTRIES;
      }

      if (($attribute, $value)= ($block[0]=~ /(\S+)\s+(\S+)/))
      { 
         ##
         # A key value pair may be a block start section of a block 
         # that need parsing. Call the parser for the block.
         #
         if (is_parser($attribute))
         {
            my $line= $block[0];  # save current line
            shift @block;         # remove the block start line

            parser_drivers($attribute, $value, $line, @block);
         } 
         else
         {
            die("UNKNOWN BLOCK TYPE: $block[0]"); 
         }
      }
      else
      {
         die("UNKNOWN CONSTRUCT: $block[0]"); 
      }

   }# while blocks 

}# parse_spec


sub parse_cfg_block
#
# All the entries in this block are configurable attributes.
# Some of the attributes have parser drivers installed for them
# as they take special parsing. For attributes without
# parser they are just assumed to be simple attribute value
# pairs.
#
{ 
   my($cfg, $name, $line, @block)= @_;
   my($attribute, $value);

   D1("codegen:parse_cfg_block: NAME=$name");

   foreach $cfg (@block)
   {
      ($attribute, $value)= ($cfg=~ /(\S+)\s+(\S+)/);

      if (is_parser($attribute))
      {
         parser_drivers($attribute, $value, $cfg);
      } 
      else
      {
         $CFG{$attribute}= $value;
      }

   }# foreach cfg entry

}# parse_cfg_block


sub parse_sm_block
#
# Parses a state machine block. 
#
{ 
   my($sm, $name, $line, @block)= @_;   # access the passed in variables

   D2("codegen:parse_sm_block: NAME=$name");

   $SMNAME= $name;  # make the current machine the "current" machine

   # Mark which state machines we've seen. Do not mark "GLOBAL" as it
   # should have no code generated for it.
   #
   $SM_LIST{$SMNAME}= 1;

   # Install defaults.
   #
   $SMS{$SMNAME}{NAME}       = $SMNAME;               # machine name

   # Parse the transitions in a state definition.
   #
   my $orig_entry;
   TOP: foreach $entry (@block)
   {
      $orig_entry= $entry;

      if ($entry=~ /^START\s+/)
      {
         $entry=~ s/START\s*//;  # remove key word
         $SMS{$SMNAME}{START}= $entry;
         next TOP;
      }
      if ($entry=~ /^TIMED_BY\s+/)
      {
         parse_timed_by($SMNAME, $entry);
         next TOP;
      }
      if ($entry=~ /^CASE_ERR\s+/)
      {
         $entry=~ s/CASE_ERR\s*//;  # remove key word
         $SMS{$SMNAME}{CASE_ERR}= $entry;
         next TOP;
      }
      if ($entry=~ /^IS_PROTECT\s+/)
      {
         $entry=~ s/IS_PROTECT\s*//;  # remove key word
         $SMS{$SMNAME}{IS_PROTECT}= $entry;
         next TOP;
      }
      if ($entry=~ /^INC\s+/)
      {
         $entry=~ s/INC//;  # remove key word
         $entry=~ s/\s*//g;
         $SMS{$SMNAME}{INC}.= ",$entry" if     $SMS{$SMNAME}{INC};
         $SMS{$SMNAME}{INC}=    $entry  unless $SMS{$SMNAME}{INC};
         next TOP;
      }
      if ($entry=~ /^METHOD\s+/)
      {
         # Break up the method into its type and body parts.
	 #
         $entry=~ s/METHOD\s+//;  # remove key word
         my($type, @parts)= split(/,/, $entry);
         my($name)= join(",", @parts);
	 my $method= {};
         strip($type);
         strip($name);
	 $name.= ";" if $name=~ /\)$/ &&  $name !~ /;^/;
	 $method->{TYPE}= $type;
	 $method->{BODY}= $name;
         push(@{$SMS{$SMNAME}{METHOD}}, $method);
         next TOP;
      }
      if ($entry=~ /^IS_MODULE\s+/)
      {
         $entry=~ s/IS_MODULE\s*//;  # remove key word
         $SMS{$SMNAME}{IS_MODULE}= $entry;
         next TOP;
      }
      if ($entry=~ /^STATE_TYPE\s+/)
      {
         $entry=~ s/STATE_TYPE\s*//;  # remove key word
         $SMS{$SMNAME}{STATE_TYPE}= $entry;
         next TOP;
      }
      if ($entry=~ /^DEBUG_LEVEL\s+/)
      {
         $entry=~ s/DEBUG_LEVEL\s*//;  # remove key word
         $SMS{$SMNAME}{DEBUG_LEVEL}= $entry;
         next TOP;
      }
      if ($entry=~ /^ON_DOC\s+/)
      {
         $entry=~ s/ON_DOC\s*//;  # remove key word
         my($on, $doc)= $entry=~ /(\w+)\W+(.*)/;
         $SMS{$SMNAME}{ON_DOC}{$on}= $doc;
         next TOP;
      }
      if ($entry=~ /^DO_DOC\s+/)
      {
         $entry=~ s/DO_DOC\s*//;  # remove key word
         my($on, $doc)= $entry=~ /(\w+)\W+(.*)/;
         $SMS{$SMNAME}{DO_DOC}{$on}= $doc;
         next TOP;
      }
      if ($entry=~ /^DERIVE\s+/)
      {
         my($derive)= $entry=~ /DERIVE\s+(.*)/;
         $SMS{$SMNAME}{DERIVE}= $derive;
         next TOP;
      }
      if ($entry=~ /^GEN_INJECT_EVENT\s+/)
      {
         my($gen)= $entry=~ /GEN_INJECT_EVENT\s+(.*)/;
         $SMS{$SMNAME}{GEN_INJECT_EVENT}= $gen;
         next TOP;
      }
      if ($entry=~ /^GEN_EFORWARDER\s+/)
      {
         my($gen)= $entry=~ /GEN_EFORWARDER\s+(.*)/;
         $SMS{$SMNAME}{GEN_EFORWARDER}= $gen;
         $SMS{$SMNAME}{GEN_INJECT_EVENT}= 1; # forwarding implies event 
		                                     # injection
         $SMS{$SMNAME}{IS_PROTECT}= 1;   # multithread protection is required.
         next TOP;
      }
      if ($entry=~ /^GEN_NOTIFY_CALL\s+/)
      {
         my($gen)= $entry=~ /GEN_NOTIFY_CALL\s+(.*)/;
         $SMS{$SMNAME}{GEN_NOTIFY_CALL}= $gen;
         next TOP;
      }
      if ($entry=~ /^GEN_AS_STRING\s+/)
      {
         my($gen)= $entry=~ /GEN_AS_STRING\s+(.*)/;
         $SMS{$SMNAME}{GEN_AS_STRING}= $gen;
         next TOP;
      }
      if ($entry=~ /^DOC\s+/)
      {
         $entry=~ s/DOC\s*//;  # remove key word
         $SMS{$SMNAME}{DOC}= $entry;
         next TOP;
      }
      if ($entry=~ /^ATTRIBUTE\s+/)
      {
         $entry=~ s/ATTRIBUTE\s+//;  # remove key word
         my($type, $name, $init)= split(/,/, $entry);
         strip($type);
         strip($name);
         $SMS{$SMNAME}{ATTRIBUTE}{$name}= $type;
         $SMS{$SMNAME}{ATTRIBUTE_VAL}{$name}= $init || 0;
         next TOP;
      }
      if ($entry=~ /^ON_ARGS\s+/)
      {
         my($on, $args)= $entry=~ /ON_ARGS\s+(.*)\s+ARGS\s+(.*)/;
		 strip($on);
         $SMS{$SMNAME}{EVENT_ARGS}{$on}= $args;
         next TOP;
      }
      if ($entry=~ /^DO_ARGS\s+/)
      {
         my($do, $args)= $entry=~ /DO_ARGS\s+(.*)\s+ARGS\s+(.*)/;
		 strip($do);
         $SMS{$SMNAME}{DO_ARGS}{$do}= $args;

	 # Convert the method signature to just the arguments needed to
	 # call the method.
	 #
	 my $arg;
	 foreach $arg (split(/,/, $args))
	 {
	    $arg=~ s/(.*\s+)//; # get rid of the type information

	    unless ($SMS{$SMNAME}{DO_CALL}{$do})
	    { $SMS{$SMNAME}{DO_CALL}{$do}= $arg; }
	    else
	    { $SMS{$SMNAME}{DO_CALL}{$do}.= ",$arg"; }
	 }

         next TOP;
      }

      die("UKNOWN SM ATTRIBUTE: $orig_entry");

   }# foreach entry

}# parse_sm_block


sub parse_post_sm_block
#
# Every state in a state machine may specify a list of other states
# that should be merge into it. This code does the merging.
#
{ 
   D1("codegen:parse_post_sm_block:");

   # Search every state machine.
   #
   foreach $sm (keys %SM_LIST)
   {
      # Search every state in a state machine.
      #
      foreach $state (keys %{$SMS{$sm}{STATES}})
      {
         my(@in_list)= split(/,/, $SMS{$sm}{$state}{IN_LIST});

         # Merge the states together if there are states to merge.
         #
         foreach $in (@in_list)
         {
            # Merge the transition list.
            #
            push(@{$SMS{$sm}{STATES}{$state}}, @{$SMS{GLOBAL}{STATES}{$in}});

            # Merge the event list
            #
            my(@list)= keys %{$SMS{GLOBAL}{EVENT_LIST}}; 
            foreach $item (@list) {
               $SMS{$sm}{EVENT_LIST}{$item}= 1;
            }

            # Merge the action list
            #
            @list= keys %{$SMS{GLOBAL}{ACTION_LIST}}; 
            foreach $item (@list) {
               $SMS{$sm}{ACTION_LIST}{$item}= 1;
            }

            # Merge the timer list
            #
            @list= keys %{$SMS{GLOBAL}{TIMER_LIST}}; 
            foreach $item (@list) {
               $SMS{$sm}{TIMER_LIST}{$item}= 1;
            }

            # Merge the timer action list
            #
            @list= keys %{$SMS{GLOBAL}{TIMER_ACTION_LIST}}; 
            foreach $item (@list) {
               $SMS{$sm}{TIMER_ACTION_LIST}{$item}= 1;
            }

         }# foreach in state to be included

      }# foreach state in the state machine 

   }# foreach state machine


}# parse_post_sm_block


sub parse_in_block
#
# Parses a state definition. Each block defines one state. There are are
# probably many state blocks.
#
{ 
   my($in, $state, $line, @block)= @_;   # access the passed in variables
   my(@trans_list)               = ();   # temporary transition list
   my(@sm_list)                  = ();   # SM's for this state
   my $is_real_transition        = undef; # it's real if it should be in 
                                          # the transition list

   ::D1("codegen:parse_in_block: STATE=$state");

   # Scan the IN block for any state machines the state is
   # associated with. If no SM attributes are defined then
   # the state is assumed to be with current state machine. 
   #
   foreach $entry (@block)
   {
      my $tmp= $entry;

      if ($tmp=~ /^SM\s+/)
      {
         $tmp=~ s/SM//;   # remove key word
         $tmp=~ s/\s*//g; # remove spaces

         foreach $sm (split(/,/, $tmp))
         {
            push(@sm_list, $sm)     if $sm ne "CURRENT";
            push(@sm_list, $SMNAME) if $sm eq "CURRENT";
         }
      } 
   }# foreach entry

   # If no state machines were defined then use the current.
   #
   push(@sm_list, $SMNAME) if $#sm_list < 0;

   # Parse the transitions in a state definition. Add them to
   # to each state machine.
   #
   TOP: foreach $sm (@sm_list)
   {
      ENTRY: foreach $entry (@block)
      {
         next ENTRY unless $entry;         # is there a line?
         next ENTRY if $entry=~ /^SM\s+/;  # SM already processed

         $SMS{$sm}{STATE_LIST}{$state}= 1; # mark which states we've seen
	 $is_real_transition          = 1; # by default all are "real"

         if ($entry=~ /^IN\s+/)
         {
            $entry=~ s/IN//;   # remove key word
            $entry=~ s/\s*//g; # remove spaces

            $SMS{$sm}{$state}{IN_LIST}.= ",$entry" 
               if $SMS{$sm}{$state}{IN_LIST}; 
            $SMS{$sm}{$state}{IN_LIST} = $entry
               unless $SMS{$sm}{$state}{IN_LIST}; 

            next ENTRY;
         } 

         $transition= {}; # anon hash for state definition

         if ($entry=~ /^TIMED_BY\s+/)
         {
            parse_timed_by($sm, $entry);
            next ENTRY;
         }
         if ($entry=~ /^DOC\s+/)
         {
	    $entry=~ s/DOC//;  # remove key word
            $$transition{DOC}= $entry;
            next ENTRY;
         }
         if ($entry=~ /^ON_ENTRY\s+/)
         {
	    $entry=~ s/ON_ENTRY//;  # remove key word
            $SMS{$sm}{$state}{ON_ENTRY}= $transition; 
            $SMS{$sm}{ON_COUNT}++;
            $SMS{$sm}{ON_ENTRY}{$state}= $transition;
            $is_real_transition= undef; # shouldn't be in transition list
         }
         if ($entry=~ /^ON_EXIT\s+/)
         {
	    $entry=~ s/ON_EXIT//;  # remove key word
            $SMS{$sm}{ON_COUNT}++;
            $SMS{$sm}{ON_EXIT}{$state}= $transition;
            $is_real_transition= undef; # shouldn't be in transition list
         }

         if ($entry=~ /\W+DO\W+/)
         {
            # Separate out the DO section from the rest of the line.
            # The DO part will be parsed on its own.
            #
            my ($do)= $entry=~ /(DO\W+.*)$/s;
            my $left= $entry;
            $left=~ s/$do//s;  # get everything ot the left of the DO

            # The left section should just be AV pairs that gets directly loaded 
            # into the transition hash.
            #
            %{$transition}= split(/\W+/, $left);
            $$transition{DO}= $do;
         }
         else
         {
            %{$transition}= split(/\W+/, $entry);
         }

	 if ($is_real_transition)
	 { 
	    # Add a transition to our list of transitions for the state.
	    #
	    push(@trans_list, $transition);
     
	    # each ON clause is an event
	    #
	    $SMS{$sm}{EVENT_LIST}{$$transition{ON}} = 1 if $$transition{ON};
	 }

         # Parse the DO clause into it separate components.
         #
         my(@do_list); # normalized list of do clauses

         foreach $do_clause (split(/,/, $$transition{DO}))
         {
            # Strip leading and trailing spaces and make sure DO is before 
            # every clause.
            #
            strip($do_clause);
            $do_clause= "DO " . $do_clause unless $do_clause=~ /^DO/;
            push(@do_list, $do_clause);

            if ($do_clause=~ /TIMED_BY/)
            {
               parse_timed_by($sm, $do_clause);
            }
            elsif ($do_clause=~ /TIMER_STOP/)
            {
            }
            else
            {
               $do_clause=~ s/DO\W+//;   # get rid of DO to get to the action
	       $do_clause=~ s/\(.*\)//;  # get rid of parens
               $SMS{$sm}{ACTION_LIST}{$::CFG{ERR_TYPE} . "," . $do_clause}= 1;
            }        

         }# foreach DO entry

         @{$$transition{DO_LIST}}= @do_list; # save the do list

	 # If there is an IF clause set the action method in the method
	 # list so it will be generated.
	 #
         if ($$transition{IF} and ! $if_seen{$$transition{IF}})
	 {
	    my $method= {};
	    $method->{TYPE}= "virtual bool";
	    $method->{BODY}= $$transition{IF}; 
	    push(@{$SMS{$sm}{IF_METHODS}}, $method);
	    $if_seen{$$transition{IF}}++;
	 } 

      }# foreach entry

      # set the transition list for the state
      #
      @{$SMS{$sm}{STATES}{$state}}= @trans_list;

   }# foreach state machine

}# parse_in_block


sub parse_timed_by
{
   my($sm, $entry)= @_;

   my %tmp     = split(/\s+/, $entry);
   my $timed_by= $tmp{TIMED_BY};

   die("Every TIMED_BY must have PERIOD clause: $do_clause\n")
      unless $tmp{PERIOD};
   
   $SMS{$sm}{ACTION_LIST}{$::CFG{ERR_TYPE} . "," . $tmp{DO}}= 1   if $tmp{DO};
   $SMS{$sm}{TIMER_LIST}{$tmp{TIMED_BY}}= 1;
   
   $SMS{$sm}{TIMER_ACTION_LIST}{"Start" . $timed_by . "(int dmsecs, int imsecs)"}= 1;
   $SMS{$sm}{TIMER_ACTION_LIST}{"Stop" . $timed_by}= 1;

   my($dmsecs, $imsecs)= split(/\./, $tmp{PERIOD});

   $SMS{$sm}{PERIODS}{$timed_by}{DMSECS}= gen_method_call($dmsecs);
   $SMS{$sm}{PERIODS}{$timed_by}{IMSECS}= gen_method_call($imsecs); 
   $SMS{$sm}{PERIODS}{$timed_by}{DO}{$tmp{DO}}{DMSECS}= gen_method_call($dmsecs);
   $SMS{$sm}{PERIODS}{$timed_by}{DO}{$tmp{DO}}{IMSECS}= gen_method_call($imsecs);

   $SMS{$sm}{ORDER}{$timed_by}= $tmp{ORDER} || "BEFORE";
   $SMS{$sm}{ORDER}{DO}{$tmp{DO}}{$timed_by}= $tmp{ORDER} || "BEFORE";

}# parse_timed_by


sub parse_agent_block
#
# Parses an agent block. 
#
{ 
   my($sm, $agent, $line, @block)= @_;   # access the passed in variables

   D2("codegen:parse_agent_block: NAME=$agent");

   ##
   # Mark which agents we've seen.
   #
   $AGENT_LIST{$agent}= 1;

   ##
   # Install defaults
   #
   $AGENTS{$agent}{NAME}       = $agent;                # machine name
   $AGENTS{$agent}{MERGE}      = 1;                     # merge code
   $AGENTS{$agent}{CLASS}      = "Agent";               # default agent class
   $AGENTS{$agent}{AGENT_INC}  = "\"Msg/Agent.h\"";     # agent include path

   ##
   # parse the transitions in a state definition 
   #
   TOP: foreach $entry (@block)
   {
      if ($entry=~ /^SM\s+/)
      {
         $entry=~ s/SM//;  # remove key word
         strip($entry);
         $AGENTS{$agent}{SM}.= ",$entry" if     $AGENTS{$agent}{SM};
         $AGENTS{$agent}{SM}=    $entry  unless $AGENTS{$agent}{SM};
         next TOP;
      }
      if ($entry=~ /^MAP\s+/)
      {
         my %tmp= split(/\s+/, $entry);
         strip($tmp{OP});
         strip($tmp{SAP});
         strip($tmp{MAP});

         die("SAP must be specified for MAP statements: $entry") 
            unless $tmp{SAP};

         if ($tmp{OP}) {
            $AGENTS{$agent}{MAP}{SAP}{$tmp{SAP}}{OP}{$tmp{OP}}{DO}= $tmp{MAP};
         }
         else {
            $AGENTS{$agent}{MAP}{SAP}{$tmp{SAP}}{DO}= $tmp{MAP};
         }

         $AGENTS{$agent}{METHOD}{$tmp{MAP}. "($::CFG{MSG_CLASS}* msg)"}
             = $::CFG{ERR_TYPE};

         next TOP;
      }
      if ($entry=~ /^REGISTER\s+/)
      {
         $entry=~ s/REGISTER\s+//;
         my %tmp= split(/\s+/, $entry);
         strip($tmp{OP});
         strip($tmp{SAP});
         strip($tmp{DEST});

         die("You can register for SAP and OP or DEST but not both: $entry\n")
           if $tmp{SAP} and $tmp{DEST} || $tmp{OP} and $tmp{DEST};

         if ($tmp{DEST}) {
            $AGENTS{$agent}{REGISTER}{$tmp{DEST}}{DEST}= $tmp{DEST};
         }
         else {
            $AGENTS{$agent}{REGISTER}{$tmp{SAP} . $tmp{OP}}{SAP}= $tmp{SAP};
            $AGENTS{$agent}{REGISTER}{$tmp{SAP} . $tmp{OP}}{OP} = $tmp{OP};
         }

         next TOP;
      }
      if ($entry=~ /^DERIVED\s+/)
      {
         $entry=~ s/^DERIVED\s+//;
         $AGENTS{$agent}{DERIVED}.= " $entry" if     $AGENTS{$agent}{DERIVED};
         $AGENTS{$agent}{DERIVED}= $entry     unless $AGENTS{$agent}{DERIVED};
         next TOP;
      }
      if ($entry=~ /^DEFINE\s+/)
      {
         my %tmp= split(/\s+/, $entry);
         $AGENTS{$agent}{DEFINE}{$tmp{DEFINE}}= $tmp{VALUE}; 
         next TOP;
      }
      if ($entry=~ /^CLASS\s+/)
      {
         $entry=~ s/CLASS\s*//;  # remove key word
         $AGENTS{$agent}{CLASS}= $entry;
         next TOP;
      }
      if ($entry=~ /^AGENT_INC\s+/)
      {
         $entry=~ s/AGENT_INC\s*//;  # remove key word
         $AGENTS{$agent}{AGENT_INC}= $entry;
         next TOP;
      }
      if ($entry=~ /^SINGLETON\s+/)
      {
         $entry=~ s/SINGLETON\s*//;  # remove key word
         $AGENTS{$agent}{SINGLETON}= $entry;
         next TOP;
      }
      if ($entry=~ /^ATTRIBUTE\s+/)
      {
         $entry=~ s/ATTRIBUTE\s+//;  # remove key word
         my($type, $name, $init)= split(/,/, $entry);
         strip($type);
         strip($name);
         $AGENTS{$agent}{ATTRIBUTE}{$name}= $type;
         next TOP;
      }
      if ($entry=~ /^METHOD\s+/)
      {
         $entry=~ s/METHOD\s+//;  # remove key word
         my($type, @parts)= split(/,/, $entry);
         my($name)= join(",", @parts);
         strip($type);
         strip($name);
         $AGENTS{$agent}{METHOD}{$name}= $type;
         next TOP;
      }
      if ($entry=~ /^INC\s+/)
      {
         $entry=~ s/INC//;  # remove key word
         $entry=~ s/\s*//g;
         $AGENTS{$agent}{INC}.= ",$entry" if     $AGENTS{$agent}{INC};
         $AGENTS{$agent}{INC}=    $entry  unless $AGENTS{$agent}{INC};
         next TOP;
      }

      die("UKNOWN AGENT ATTRIBUTE: $entry");

   }# foreach entry

}# parse_agent_block


sub parse_msg_block
#
# Parse a message definition.
#
{ 
   my($in, $msg, $line, @block)= @_; # access the passed in variables
   my(@field_list)             = (); # temporary message field list
   my($offset)                 = 0;  # field offset
   my($is_len_field)           = 0;  # was there a length field?

   D2("codegen:parse_msg_block: MSG=$msg");

   $MSG_LIST{$msg}= 1; # mark which messages we've seen

   ##
   # parse the transitions in a state definition 
   #
   TOP: foreach $entry (@block)
   {
      $field= {}; # anon hash for message entry definition
  
      if ($entry=~ /^WHAT\s+/)
      {
         $entry=~ s/WHAT\s*//;  # remove key word
         $MSGS{$msg}{WHAT}= $entry;
         next TOP;
      }
      if ($entry=~ /^DEFINE\s+/)
      {
         my %tmp= split(/\s+/, $entry);
         $MSGS{$msg}{DEFINE}{$tmp{DEFINE}}= $tmp{VALUE}; 
         next TOP;
      }
      if ($entry=~ /^LEN\s+/)
      {
         $entry=~ s/LEN\s*//;  # remove key word
         $MSGS{$msg}{LENGTH}= $entry;
         $is_len_field= 1;
         next TOP;
      }
      if ($entry=~ /^TLV/)
      {
         $MSGS{$msg}{TLV}= 1;
         next TOP;
      }
      elsif ($entry=~ /^FLD\s+/)
      {
         %{$field}= split(/\s+/, $entry);  # split into fields

         if ($$field{CLASS})
         {
            die ("CLASS field= $$field{CLASS} must have SIZE attribute") 
               unless $$field{SIZE};
            $$field{TYPE}= $$field{CLASS};
         }

         if (($type, $cnt)= ($$field{TYPE}=~ /(\w+)\[(\d+)\]/))
         {
            $$field{SIZE}= $SIZES{$type} unless $$field{SIZE};

            die("Array has unknown base type $type\n") unless $$field{SIZE};

            $$field{ARRAY}        = 1;
            $$field{FIXED_ARRAY}  = 1;
            $$field{BASE}         = $type; 
            $$field{COUNT}        = $cnt; 
         }
         elsif (($type)= ($$field{TYPE}=~ /(\w+)\[\]/))
         {
            $$field{SIZE}= $SIZES{$type} unless $$field{SIZE};

            die("Array has unknown base type $type\n") unless $$field{SIZE};

            $$field{ARRAY}      = 1;
            $$field{VAR_ARRAY}  = 1; 
            $$field{BASE}       = $type; 
            $$field{COUNT}      = 0 unless $$field{COUNT};

            die("Variable length fields require a COUNTFLD or COUNT attribute")
               unless $$field{COUNTFLD} || $$field{COUNT};
         }
         else
         {
            $$field{COUNT}= 1; 
            $$field{BASE} = $$field{TYPE} unless $$field{BASE};
            $$field{SIZE} = $SIZES{$$field{BASE}} unless $$field{SIZE};

            die("Array has unknown base type $${BASE}\n") unless $$field{SIZE};
         }

         if ($$field{CLASS})
         {
            my $type= $$field{BASE};
            $SIZES{$type}         = $$field{SIZE};
            $XFORM{"$type"}{TOLOCAL}= "$type\:\:ToLocal";
            $XFORM{"$type"}{TONET}  = "$type\:\:ToNet";
         }

         $$field{OFFSET}= $offset;                        # set the field offset
         $$field{VALUE} = 0 unless $$field{VALUE};        # set default value
         $$field{LENGTH}= $$field{COUNT} * $$field{SIZE}; # calc field length
         $offset+= $$field{LENGTH};                       # calc new offset
         $MSGS{$msg}{LENGTH}= $offset unless $is_len_field;
         push(@field_list, $field);                       # add to msg field list
         next TOP;
      }

      if ($entry=~ /^INC\s+/)
      {
         $entry=~ s/INC//;  # remove key word
         $entry=~ s/\s*//g;
         $MSGS{$msg}{INC}.= " $entry" if     $MSGS{$msg}{INC};
         $MSGS{$msg}{INC}=    $entry  unless $MSGS{$msg}{INC};
         next TOP;
      }
      else
      {
         die("UNRECOGNIZED: MSG=$msg ENTRY: $entry");
      }

      push(@field_list, $field);

   }# foreach entry

   die("The specified message length=$MSGS{$msg}{LENGTH} is less than the actual  \
        field length=$offset") if $MSGS{$msg}{LENGTH} < $offset;

   ##
   # set the fields for the message
   #
   @{$MSGS{$msg}{FIELDS}}= @field_list;

}# parse_msg_block


sub parse_cfg_line
#
# Parses configurable attributes that are not strict attribute
# value pairs. An example is for attributes that have a comma
# separated list of values.
#
{
   my($key, $value, $line)= @_;

   D2("parse_cfg_line: KEY: $key LINE: $line");

   $line=~ s/^$key\s+//;
   $CFG{$key}.= " $line" if $CFG{$key};
   $CFG{$key}=  $line unless $CFG{$key};

}# parse_cfg_line


sub parse_cfg_drivers
#
# Parses the DRIVERS configurable attribute. It requires all
# drivers and then initializes the drivers.
#
{
   my($key, $value, $line)= @_;

   D1("parse_cfg_drivers: KEY: $key VAL: $value LINE: $line");

   $line=~ s/^$key//;
   $line=~ s/\s+//g;

   my(@drivers)= split(/,/, $line); # split into drivers
   $CFG{DRIVERS}.= $line;

   ##
   # Bring drivers into this program. Their BEGIN sections execute
   # and install drivers interfaces.
   #
   foreach $file (@drivers) {
      require "$file";
   }

   ##
   # Call init functions of new drivers.
   #
   init_drivers();

}# parse_cfg_drivers


sub parse_cgi_parser_drivers
#
# Parses the CGI_PARSER_DRIVERS configurable attribute. It initializes the
# driver as well.
#
{
   my($key, $value, $line)= @_;

   D2("parse_cgi_parser_drivers: KEY: $key VAL: $value LINE: $line");

   $line=~ s/^$key//;
   $line=~ s/\s+//g;

   my(@drivers)= split(/,/, $line);
   $CFG{CGI_PARSER_DRIVERS}.= $line;

   foreach $entry (@drivers)
   {
      my($program, $block)= split(/=/, $entry);

      $driver= {};  # create anonymous hash for driver functions

      $$driver{NAME}       = "CGI Parser Driver for  BLOCK=$block
PROGRAM=$program";
      $$driver{INIT}       = \&cgi_init;
      $$driver{ACTION}     = \&cgi_action;
      $$driver{POST_ACTION}= \&cgi_post_action;
      $$driver{CLOSE}      = \&cgi_close;
      $$driver{PROGRAM}    = $program;
      $$driver{BLOCK}      = $block;

      install_parser_driver($block, $driver);
   }

   ##
   # Call init functions of new drivers.
   #
   init_drivers();

}# parse_cgi_parser_drivers


sub parse_cgi_code_drivers
#
# Parses the CGI_CODE_DRIVERS configurable attribute. It initializes the driver
# as well.
#
{
   my($key, $value, $line)= @_;

   D2("parse_cgi_code_drivers: KEY: $key VAL: $value LINE: $line");

   $line=~ s/^$key//;
   $line=~ s/\s+//g;

   my(@drivers)= split(/,/, $line);
   $CFG{CGI_CODE_DRIVERS}.= $line;

   foreach $entry (@drivers)
   {
      my($program)= split(/=/, $entry);

      $driver= {};  # create anonymous hash for driver functions

      $$driver{NAME}      = "CGI Parser Driver for  BLOCK=$block
PROGRAM=$program";
      $$driver{INIT}      = \&cgi_init;
      $$driver{ACTION}    = \&cgi_action;
      $$driver{CLOSE}     = \&cgi_close;
      $$driver{PROGRAM}   = $program;

      install_code_driver($driver);
   }

   ##
   # Call init functions of new drivers.
   #
   init_drivers();

}# parse_cgi_code_drivers


sub parse_cfg_search_path
#
# Parses the configurable attribute SEARCH_PATH. It adds the search
# paths to the perl's @INC search path list.
#
{
   my($key, $value, $line)= @_;

   D2("parse_cfg_search_path: KEY: $key VAL: $value LINE: $line");

   $line=~ s/SEARCH_PATH\s+//;  # remove the key word 

   ##
   # Set the search path. Append the search path if one already exists.
   #

   $CFG{SEARCH_PATH}.= ", $line" if $CFG{SEARCH_PATH}; 
   $CFG{SEARCH_PATH}= $line      unless $CFG{SEARCH_PATH}; 

   my(@paths)= split(/,/, $CFG{SEARCH_PATH});

   ##
   # Add the search paths to perl's search path variable @INC.
   #
   foreach (@paths) 
   {
      unshift(@INC, $_);
   }

}# parse_cfg_search_path


sub parse_attr_comma
{
   my($set, $key, $line, $is_append)= @_;

   D2("parse_attr_value: KEY: $key LINE: $line");

   $line=~ s/^$key\s+//;   # get rid of key word
   $line=~ s/\s+//g;    # remove spaces

   if ($is_append eq "a")
   {
      $$set.= ",$line"  if     $$set; 
      $$set=  $line     unless $$set;
   }
   else
   {
      $$set= $line;
   }

}# parse_attr_comma


sub parse_attr_line
{
   my($set, $key, $line, $is_append)= @_;

   D1("parse_attr_line: KEY: $key LINE: $line");

   $line=~ s/^$key\s+//;

   if ($is_append eq "a")
   {
      $$set.= " $line" if     $$set; 
      $$set=    $line  unless $$set;
   }
   else
   {
      $$set= $line;
   }

}# parse_attr_line


############################## DRIVER FUNCTIONS ##############################


sub install_code_driver
#
# Install a code generation driver.
#
{
   my(@drivers)= @_;

   push(@CODE_DRIVERS, @drivers);
   add_all_driver(@drivers);

}# install_code_driver


sub install_parser_driver
#
# Install a parser for a block.
#
{
   my($block_type, @drivers)= @_;

   push(@{$PARSER_DRIVERS{$block_type}}, @drivers);
   add_all_driver(@drivers);

}# install_parser_driver


sub init_drivers
#
# Initialize all drivers that haven't already been initialized.
#
{
   foreach $driver (@ALL_DRIVERS)
   {
      ##
      # Don't init driver if it has been already inited.
      #
      unless ($DRIVER_STATE{$driver}{INIT})
      {
         ## 
         # Don't call the init function if there isn't one.
         #
         if (defined $$driver{INIT})
         {
            $f= $$driver{INIT};
            &$f();
            
            $DRIVER_STATE{$driver}{INIT}= 1; # mark as inited
          }
      }

   }# foreach driver

}# init_drivers


sub close_drivers
#
# Close all drivers.
#
{
   foreach $driver (@ALL_DRIVERS)
   {
      ##
      # Don't close driver if it has been already closed.
      #
      unless ($DRIVER_STATE{$driver}{CLOSE})
      {
         ## 
         # Don't call the close function if there isn't one.
         #
         if (defined $$driver{CLOSE})
         {
            $f= $$driver{CLOSE};
            &$f();
            
            $DRIVER_STATE{$driver}{CLOSE}= 1; # mark as closed 
          }
      }
   }# foreach driver

}# close_drivers


sub code_drivers
#
# Invoke the action function for all installed code drivers.
#
{
   foreach $driver (@CODE_DRIVERS)
   {
      if (defined $$driver{ACTION})
      {
         $f= $$driver{ACTION};
         &$f();
      }
   }

}# code_drivers


sub is_parser
#
# is_parser($type)
#    $type - an attribute name designating a block
# 
# If a value is returned then there is a parser driver 
# installed for the passed in block start keyword.
#
{
   return defined $PARSER_DRIVERS{@_[0]};

}# is_parser


sub parser_drivers
#
# Invoke the ACTION method for all parser drivers installed for
# a block type.
#
{
   my($type, $name, $line, @block)= @_;
   my(@drivers)= @{$PARSER_DRIVERS{$type}};

   D2("parser_drivers: TYPE=$type NAME: $name");

   foreach $driver (@drivers)
   {
      if (defined $$driver{ACTION})
      {
         D2("codegen: calling ACTION");

         $f=$$driver{ACTION};
         &$f($type, $name, $line, @block);
      }

   }# foreach driver

}# parser_drivers


sub post_parser_drivers
#
# Call the POST_ACTION method of all parser drivers. This is called
# after all parsing is done to do any fixup that can only occur
# when everything is parsed.
#
{
   foreach $parser (keys %PARSER_DRIVERS)
   {
      my(@drivers)= @{$PARSER_DRIVERS{$parser}};

      foreach $driver (@drivers)
      {
         if (defined $$driver{POST_ACTION})
         {
            $f=$$driver{POST_ACTION};
            &$f();
         }
   
      }# foreach driver
   }# foreach parser

}# post_parser_drivers


sub post_code_drivers
#
# Call the POST_ACTION method of all code drivers.
#
{
   foreach $code (keys %CODE_DRIVERS)
   {
      my(@drivers)= @{$CODE_DRIVERS{$code}};

      foreach $driver (@drivers)
      {
         if (defined $$driver{POST_ACTION})
         {
            $f=$$driver{POST_ACTION};
            &$f();
         }
   
      }# foreach driver
   }# foreach code driver

}# post_code_drivers


sub add_all_driver
#
# Add a driver to a list of all drivers. It makes sure
# the list has no duplicate drivers.
#
{
   my(@drivers)= @_;

   TOP: foreach $driver (@drivers)
   {
      ##
      # Don't add the driver if it's already there
      #
      foreach $all (@ALL_DRIVERS) {
         next TOP if $all == $driver;
      }

      push(@ALL_DRIVERS, $driver);
   }

}# add_all_driver


sub parse_cfg_search_path
#
# Parses the configurable attribute SEARCH_PATH. It adds the search
# paths to the perl's @INC search path list.
#
{
   my($key, $value, $line)= @_;

   D1("parse_cfg_search_path: KEY: $key VAL: $value LINE: $line");

   $line=~ s/SEARCH_PATH\s+//;  # remove the key word 

   ##
   # Set the search path. Append the search path if one already exists.
   #
   $CFG{SEARCH_PATH}.= ", $line" if $CFG{SEARCH_PATH}; 
   $CFG{SEARCH_PATH}= $line      unless $CFG{SEARCH_PATH}; 

   my(@paths)= split(/,/, $CFG{SEARCH_PATH});

   ##
   # Add the search paths to perl's search path variable @INC.
   #
   foreach (@paths) 
   {
      unshift(@INC, $_);
   }

}# parse_cfg_search_path


################################ DUMP FUNCTIONS ##############################


sub dump_parse_tree
#
# Print the parse tree
#
{
   dump_cfg();
   dump_sm();
   dump_agent();
   dump_msg();

}# dump_parse_tree


sub dump_cfg
{
   print "\nCONFIGURATION PARSE TREE:\n\n";

   print "   INC: @INC\n";

   print "\n   ATTRIBUTES:\n";
   foreach $arg (keys %CFG)
   {
      print "      $arg:\t\t$CFG{$arg}\n";
   }

   print "\n   DRIVERS:\n";
   foreach $driver (@ALL_DRIVERS)
   {
      print "      DRIVER=$$driver{NAME} VERSION=$$driver{VERSION}\n";
   }

}# dump_cfg


sub dump_sm
{
   print "\nSTATE MACHINE PARSE TREE:\n\n";

   print "   SM LIST:\n";
   foreach $sm (keys %SM_LIST)
   {
      print "      $sm\n";
   }

   foreach $sm (keys %SM_LIST)
   {
      print "\n";
      dump_sm_entry($sm);
      print "\n\n";
   }

}# dump_sm


sub dump_sm_entry
{
   my($sm)= @_;

   print "   SM: $SMS{$sm}{NAME}\n";
   print "      NAME         : $SMS{$sm}{NAME}\n";
   print "      START        : $SMS{$sm}{START}\n";
   print "      CASE_ERR     : $SMS{$sm}{CASE_ERR}\n";
   print "      INC          : $SMS{$sm}{INC}\n";

   print "\n      STATE LIST:\n";
   while (($state,$x)=  each %{$SMS{$sm}{STATE_LIST}})
   {
      print "         $state\n";
   }

   print "\n      EVENT LIST:\n";
   foreach $event (keys %{$SMS{$sm}{EVENT_LIST}})
   {
      print "         $event\n";
   }

   print "\n      ACTION LIST:\n";
   foreach $action (keys %{$SMS{$sm}{ACTION_LIST}})
   {
      print "         $action\n";
   }

   print "\n      TIMER LIST:\n";
   foreach $timer (keys %{$SMS{$sm}{TIMER_LIST}})
   {
      print "         $timer\n";
   }

   print "\n      TIMER ACTION LIST:\n";
   foreach $timer (keys %{$SMS{$sm}{TIMER_ACTION_LIST}})
   {
      print "         $timer\n";
   }

   print "\n      STATES:";
   foreach $state (keys %{$SMS{$sm}{STATES}})
   {
      print "\n         STATE: $state\n";
      print "            IN_LIST= $SMS{$sm}{$state}{IN_LIST}\n";
      foreach $transition (@{$SMS{$sm}{STATES}{$state}})
      {
         print "            TRANSITION:\n";
         print "               NEXT    = ", $$transition{NEXT}, "\n";
         print "               ON      = ", $$transition{ON}, "\n";
         #print "               DO      = ", $$transition{DO}, "\n";
         print "               DO_LIST:\n";
         foreach $do (@{$$transition{DO_LIST}}) {
         print "                         $do\n"; 
         }
      } 
   }

   print "\n      PERIODS:\n";
   foreach $timer (keys %{$SMS{$sm}{PERIODS}})
   {
      print "         TIMER: $timer\n";
      print "            DMSECS: $SMS{$sm}{PERIODS}{$timer}{DMSECS}\n";
   }

}# dump_sm_entry


sub dump_agent
{
   print "\nAGENT PARSE TREE:\n\n";

   print "   AGENT LIST:\n";
   foreach $val (keys %AGENT_LIST)
   {
      print "      $val\n";
   }

   foreach $agent (keys %AGENT_LIST)
   {
      print "\n";
      dump_agent_entry($agent);
      print "\n\n";
   }


}# dump_agent


sub dump_agent_entry
{
   my($agent)= @_;

   print "   AGENT: $AGENTS{$agent}{NAME}\n";
   print "      NAME         : $AGENTS{$agent}{NAME}\n";
   print "      SM           : $AGENTS{$agent}{SM}\n";
   print "      DERIVED      : $AGENTS{$agent}{DERIVED}\n";
   print "      CLASS        : $AGENTS{$agent}{CLASS}\n";
   print "      MERGE        : $AGENTS{$agent}{MERGE}\n";
   print "      SINGLETON    : $AGENTS{$agent}{SINGLETON}\n";
   print "      AGENT_INC    : $AGENTS{$agent}{AGENT_INC}\n";
   print "      INC          : $AGENTS{$agent}{INC}\n";

   print "\n      ATTRIBUTES:\n";
   while (($key, $value) = each %{$AGENTS{$agent}{ATTRIBUTE}})
   {
      print "         TYPE     : $value\n";
      print "         ATTRIBUTE: $key\n";
      print "         ----------------------------------------\n";
   }

   print "\n      METHODS:\n";
   while (($key, $value) = each %{$AGENTS{$agent}{METHOD}})
   {
      print "         TYPE     : $value\n";
      print "         METHOD   : $key\n";
      print "         ----------------------------------------\n";
   }

   print "\n   REGISTER:\n";
   while (($k, $v) = each %{$AGENTS{$agent}{REGISTER}})
   {
      if ($AGENTS{$agent}{REGISTER}{$k}{DEST})
      {
         print "      DEST: $AGENTS{$agent}{REGISTER}{$k}{DEST}\n";
      }
      else
      {
         print "      SAP: $AGENTS{$agent}{REGISTER}{$k}{SAP}  OP:
$AGENTS{$agent}{REGISTER}{$k}{OP}\n";
      }
   }

   print "\n   MAP:\n";
   while (($sap, $value) = each %{$AGENTS{$agent}{MAP}{SAP}})
   {
      print "      SAP: $sap\n";
      $do= $AGENTS{$agent}{MAP}{SAP}{$sap}{DO};

      if ($do) {
         print "         DO: $do\n";
      }
      else
      { 
         while (($op, $value) = each %{$AGENTS{$agent}{MAP}{SAP}{$sap}{OP}})
         {
            $do= $AGENTS{$agent}{MAP}{SAP}{$sap}{OP}{$op}{DO};
            print "         OP: $op\n";
            print "            DO: $do\n";
         }
      }
      print "      ----------------------------------------\n";
   }

}# dump_agent_entry


sub dump_msg
{
   print "\nMESSAGE PARSE TREE:\n\n";

   print "   MSG LIST:\n";
   foreach $msg (keys %MSG_LIST)
   {
      print "      $msg\n";
   }

   foreach $msg (keys %MSG_LIST)
   {
      dump_msg_entry($msg); 
   }

}# dump_msg


sub dump_msg_entry
{
   my($msg)= @_;

   print "\n   MSG FIELDS:";
   foreach $msg (keys %MSGS)
   {
      print "\n      MSG: $msg\n";
      print "         WHAT   = $MSGS{$msg}{WHAT}\n";
      print "         LENGTH = $MSGS{$msg}{LENGTH}\n";
      print "         TLV    = $MSGS{$msg}{TLV}\n";
      print "         INCLUDE= $MSGS{$msg}{INC}\n";

      
      print "      DEFINES:\n";
      while (($k, $v)= (each %{$MSGS{$msg}{DEFINE}}))
      {
         print "         DEFINE=$k  VALUE=$v\n";
      }

      print "      FIELDS:\n";
      foreach $field (@{$MSGS{$msg}{FIELDS}})
      {
         print "            FLD        = ", $$field{FLD}, "\n";
         print "            TYPE       = ", $$field{TYPE}, "\n";
         print "            CLASS      = ", $$field{CLASS}, "\n" if
$$field{CLASS};
         print "            BASE       = ", $$field{BASE}, "\n";
         print "            VALUE      = ", $$field{VALUE}, "\n";
         print "            SIZE       = ", $$field{SIZE}, "\n";
         print "            COUNT      = ", $$field{COUNT}, "\n";
         print "            LENGTH     = ", $$field{LENGTH}, "\n";
         print "            OFFSET     = ", $$field{OFFSET}, "\n";
         print "            FIXED_ARRAY= ", $$field{FIXED_ARRAY}, "\n" 
            if $$field{FIXED_ARRAY};
         print "            VAR_ARRAY  = ", $$field{VAR_ARRAY}, "\n" 
            if $$field{VAR_ARRAY};
         print "            COUNTFLD   = ", $$field{COUNTFLD}, "\n" 
            if $$field{COUNTFLD};
         print "            -------------------------\n";

      }# foreach field 

   }# foreach msg

}# dump_msg_entry


############################# UTILITY FUNCTIONS #############################


##
# squish is a rather ugly function to merge lines with line  
# continuation. It could undoubtably be much more cleverly 
# implemented but this all i can think of.
#
sub squish
{
   my(@lines);           # lines to pass back
   my($is_merge)= 0;     # true when merging lines

   ##
   # look at every line to determine if it has a line continuation
   # mark. If it does merge lines together.
   #
   foreach $line (split(/\n/, shift))
   {
      next if $line=~ /\s*#/;                   # skip all comment lines

      $line=~ s/^\s*//;                         # remove leading spaces
      $line=~ s/$\s*//;                         # remove trailing spaces

      if ($line=~ /\\/)
      {
         $line=~ s/\\//;                        # remove slash
         $line=~ s/$\s*//;                      # remove trailing spaces
         $lines[$#lines].= $line if $is_merge;  # append when merge
         push(@lines, $line) unless $is_merge;  # push when not merge
         $is_merge++;
      }
      else
      {
         $lines[$#lines].= $line if $is_merge;  # append when merge
         push(@lines, $line) unless $is_merge;  # push when not merge
         $is_merge= 0;
      }
   }# foreach line

   return @lines;

}# squish


sub strip
{
   @_[0]=~ s/^\s+//;  # remove leading spaces
   @_[0]=~ s/\s+$//;  # remove trailing spaces

}# strip


##
# CGI Driver Functions

sub cgi_init
{

}# cgi_init


sub cgi_close
{

}# cgi_close


sub cgi_action
{

}# cgi_action


sub cgi_post_action
{

}# cgi_post_action


##
# Debugging Functions
#
sub D
{ 
   print STDERR @_[0], "\n";

}# D

sub D1
{ 
   print STDERR @_[0], "\n" if $::CFG{DEBUG} >= 1; 

}# D1

sub D2
{ 
   print STDERR @_[0], "\n" if $::CFG{DEBUG} >= 2; 

}# D2

sub D3
{ 
   print STDERR @_[0], "\n" if $::CFG{DEBUG} >= 3; 

}# D3

sub D4
{ 
   print STDERR @_[0], "\n" if $::CFG{DEBUG} >= 4; 

}# D4

sub D5
{ 
   print STDERR @_[0], "\n" if $::CFG{DEBUG} >= 5; 

}# D5


##
# Get the implementation for the passed in method.
#
sub get_impl
{
   my($src, $method, $default_impl)= @_;
   my($is_found)= 0;
   my($impl)    = "";
   my(@lsrc)    = @{$src};  # make local copy


   foreach $tmp_method (@lsrc)
   {
      if ($is_found)
      {  
         $impl.= $tmp_method;
         last if $tmp_method=~ /^}/;
      }  
      else
      {
         ## 
         # Remove characters that screw up the pattern match
         # This is a horrible hack. Have to figure out how
         # to do this better.
         #
         $method    =~ s/\(//g;
         $method    =~ s/\<\<//g;
         $method    =~ s/\)//g;
         $method    =~ s/\*//g;
         $tmp_method=~ s/\(//g;
         $tmp_method=~ s/\)//g;
         $tmp_method=~ s/\*//g;
         $tmp_method=~ s/\<\<//g;

         $is_found++ if $tmp_method=~ /$method/;
      }

   }# foreach line in source

   return ($is_found) ? $impl : $default_impl;

}# get_impl


sub gen_method_call
{
   my $body= shift;

   return 0 unless defined $body;

   # If it's an integer just return it.
   #
   return $body if $body=~ /^\d/;


   # Add a void function clause if it doesn't any arguments.
   #
   $body.= "()" unless $body=~ /\(/;

   return $body;

}# gen_method_call
