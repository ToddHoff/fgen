#
# This package is required in the context of the codegen, a code
# generator driver. See codegen for information on what data structures
# are available and the format of the driver table.
#
BEGIN 
{ 
   $sm_driver= {};  # create anonymous hash for driver functions

   $$sm_driver{NAME}      = "Lightera Generator";
   $$sm_driver{VERSION}   = "1.0";
   $$sm_driver{INIT}      = \&lngen::gen_ln_init;
   $$sm_driver{ACTION}    = \&lngen::gen_ln;
   $$sm_driver{CLOSE}     = \&lngen::gen_ln_close;

   install_code_driver($sm_driver);

}# BEGIN


package lngen;

use Text::Wrap;


sub gen_ln_init
{
   ::D1("lngen:gen_ln_init:");

}# gen_ln_init


sub gen_ln_close
{
   ::D1("lngen:gen_ln_close:");

}# gen_ln_close


sub gen_ln
{
   ::D1("lngen:gen_ln:");

   gen_sm();
#gen_agent();

}# gen_ln


sub gen_sm
{
   ::D1("lngen:gen_sm:");

   foreach $sm (keys %::SM_LIST)
   {
      die("You must have a name for the state machine and an initial state")
         unless $::SMS{$sm}{NAME} && $::SMS{$sm}{START};

      my @states= keys %{$::SMS{$sm}{STATE_LIST}};
      unless ($#states >= 0)
      {
         warn("WARNING: State Machine=$sm has no states\n");
         next;
      }

      gen_sm_class($sm);
      gen_sm_impl($sm);

   }# foreach state machine

}# gen_sm


##
# Generating the state machine class interface.
#
sub gen_sm_class
{
   my($sm)= @_;

   ::D1("lngen:gen_sm_class: SM=$sm");

   open(SMH, ">$::SMS{$sm}{NAME}.h") || die("FAIL open $::SMS{$sm}{NAME}.h: $!")
      unless $::CFG{IS_STDOUT};
   select(SMH) unless $::CFG{IS_STDOUT};

   # Set defaults.
   #
   my $is_state_type_provided= $::SMS{$sm}{STATE_TYPE};
   my $state_type= $::SMS{$sm}{STATE_TYPE} || "State";
  

   # Gen begin include guard
   #
   print "#ifndef _", $::SMS{$sm}{NAME}, "_h_\n";
   print "#define _", $::SMS{$sm}{NAME}, "_h_\n\n";

   print <<EOF;
// Abstract state machine class for state machine "$sm".
//
// NOTE: this code is auto generated. Your changes will be 
// written over. Derive from the base class to make changes.


EOF
   
   # Gen include file statements
   #
   gen_includes();

   foreach (split(/,/, $::SMS{$sm}{INC})) {
      print "#include $_\n";
   }
   print "#include $::CFG{TIMER_INC}\n";
   print "#include $::CFG{DEBUG_INC}\n" if $::CFG{GEN_DEBUG};
   print "#include \"Util/Log.h\"\n" if $::CFG{GEN_DEBUG};
   print "#include $::CFG{MUTEX_INC}\n" 
      if $::CFG{MUTEX_INC} and $::SMS{$sm}{IS_PROTECT};
   print "\n\n";

   my $is_module= is_sm_module($sm);
   print "#include $::CFG{MODULE_INC}\n"  if $is_module;


   # Generate event forwarding.
   #
   print "#include \"Util/Action.h\"\n\n" if $::SMS{$sm}{GEN_EFORWARDER};
 

   # Gen timer classes
   #
   print "class $::SMS{$sm}{NAME};\n\n";

   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      print "class $timer : public $::CFG{TIMER_CLASS}\n{\npublic:\n";
      print "   $timer($::SMS{$sm}{NAME}* parent=0) : $::CFG{TIMER_CLASS}(), mpParent(parent) {}\n";
      print "   $::CFG{TIMER_FUNC};\n";
      print "   $::SMS{$sm}{NAME}*   mpParent;\n};\n\n\n";
   } 

   # Gen state machine class
   # 
   my $doc= wrap(" * ", " * ", $::SMS{$sm}{DOC});

   print <<EOF;
/**
 * Abstract state machine class for state machine <B>$sm</B>.
 * This class was generated by the 
 * <A HREF="$::CFG{FGEN_PAGE}">fgen</A> state machine
 * generator.
 *
$doc
 *
 * <H3> Base Class Usage </H3>
 *
 * It is expected a class will derive from this class and
 * implement the virtual methods to provide the state machine
 * specific behavior. State machine transitions are triggered
 * by calls to events. Events make calls to actions defined
 * in the state machine specification. Actions are implemented
 * by a derived class.
 *
 * <H3> Some State Machine Generator Tricks You may Not Know </H3>
 *
 * <H4> Mutex Generation </H4>
 * If you want a mutex generated set "IS_PROTECT 1" in the SM
 * block of the state machine specfication. 
 *
 * <H4> Debug Generation </H4>
 * Debug generation
 * can be turned on with "GEN_DEBUG 1" in the CFG
 * block of the state machine specfication. "GEN_DEBUG 0" turns
 * off debug generation. 
 *
 * <H4> Using a Module </H4>
 * Setting "IS_MODULE 1" in the CFG block
 * makes the state machine accept a module parameter in its
 * constructor. 
 *
 * <H4> Adding a Method </H4>
 * Arbitrary methods can be added to the state machine
 * with the "METHOD" directive in the SM block.
 *
 * \@see <A HREF="$::CFG{FGEN_PAGE}">fgen documentation</A>
 */
EOF

   print "class $::SMS{$sm}{NAME}";
   print " : $::SMS{$sm}{DERIVE}" if $::SMS{$sm}{DERIVE};
   print "\n{\npublic:\n\n";

   # Generate enum for states
   #
   if ($is_state_type_provided)
   {
print <<EOF;
   /**
    * The enum of state machine states is being provided by the application
    * and is called $state_type. The state machine definition must provide 
    * an include statement (using INC) to resolve the enum.
    */ 
EOF
   }
   else
   {
print <<EOF;
   /**
    * Enum defining states in the state machine.
    */ 
EOF

      print "   enum $state_type\n   {\n";

      @state_names= keys %{$::SMS{$sm}{STATE_LIST}};
      $len= @state_names;
      $i= 0;
      foreach $state (@state_names) 
      {
         $i++;
         if ($i < $len)  { print "      $state,\n"; } 
         else            { print "      $state\n"; }
      }
      print "   };\n\n";
   }
   

   # Gen constructor
   #
   if ($is_module)
   {
print <<EOF;
   /**
    * Construct and initialize the state machine.
    *
    * \@param module The module to associate with the state machine.
    *    The module is passed to debug and log macros to control
    *    the logging for the state machine.
    * \@param pDebugId An ID for the state machine that is used in debug output.
    *    MEMORY: COPIED. 
    */
EOF
      psig("", "$::SMS{$sm}{NAME}($::CFG{MODULE_CLASS}* module= 0, const char* pDebugId= 0);\n"); 
   }
   else
   {
print <<EOF;
   /**
    * Construct and initialize the state machine. If debug was generated
    * it will pass 0 as the module.
    *
    * \@param pDebugId An ID for the state machine that is used in debug output.
    *    MEMORY: COPIED. 
    */
EOF
      psig("", "$::SMS{$sm}{NAME}(const char* pDebugId= 0);\n");
   }


   # Define a destructor.
   #
print <<EOF;
   /**
    * Virtual destructor for the state machine. 
    *
    */
EOF

   print "   virtual ~$::SMS{$sm}{NAME}()\n";
   print "   {\n";

   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      if ($::CFG{TIMER_IS_LNOBJECT})
      {
	 print "      if (mp$timer) mp$timer->Destroy();\n";
      }
      else
      {
	 print "      delete mp$timer;\n";
      }
   }
   print "   }\n\n";


   # Gen get state setting
   #
print <<EOF;
   /**
    * Return the current state of the state machine.  When the
	* state machine first starts the previous state is set to
	* the initial state specified in the state machine specification.
    *
    * \@return The current state of the state machine.
    */
EOF
   psig($state_type, "CurrentState() const { return mState; }");
   print "\n";


   # Gen get previous state setting. 
   #
print <<EOF;
   /**
    * Return the previous state of the state machine. When the
	* state machine first starts the previous state is set to
	* the initial state  specified in the state machine specification.
    *
    * \@return The previous state of the state machine.
    */
EOF
   psig($state_type, "PrevState() const { return mPrevState; }");
   print "\n";


   # Gen get previous different state setting. 
   #
print <<EOF;
   /**
    * Return the previous different state of the state machine. When the
	* state machine first starts the previous different state is set to
	* the initial state  specified in the state machine specification.
    *
    * \@return The previous different state of the state machine.
    */
EOF
   psig($state_type, "PrevDifferentState() const { return mPrevDifferentState; }");
   print "\n";


   # Gen get DebugId
   #
print <<EOF;
   /**
    * Return the debug id. It is used to identify the state machine in debug
    * output.
    *
    * \@return The debug id. 
    */
EOF
   psig("const RWCString&", "GetDebugId() const { return mDebugId; }");
   print "\n";


   # Gen all events
   #
   print "// Events\n";
   foreach $event (keys %{$::SMS{$sm}{EVENT_LIST}})
   {
print <<EOF;

   /**
    * Event $event. When an event is called the state machine 
    * transitions and associated actions are executed.
    *
EOF
      # If there is event documentation then generate it.
      #
      my $doc= $::SMS{$sm}{ON_DOC}{$event};
      print wrap("    * ", "     * ", $doc), "\n" if $doc;

print <<EOF;
    *
    * \@return $::CFG{NO_ERR} If the state transitioned successfully. 
    * \@return $::CFG{IS_ERR} If the state failed to transition.
    */
EOF
      my $args= $::SMS{$sm}{EVENT_ARGS}{$event};
      psig("$::CFG{ERR_TYPE}", "$event($args);") unless $seen_event{$event};

      $seen_event{$event}= 1;
   }


   # Gen inject signature.
   #
   if ($::SMS{$sm}{GEN_INJECT_EVENT})
   {
		my $method_name= "InjectEvent";

print <<EOF;

   /**
    * Inject an event into the state machine using the name of the event.
    * The method matching the event name will be called.
    *
	* \@param pEventName The name of the event.
	*
    * \@return $::CFG{NO_ERR} If the event was known, 
    *    $::CFG{IS_ERR} If the event was not known.
    */
EOF
      psig("virtual $::CFG{ERR_TYPE}", "$method_name(const char* pEventName);");
   }


   # Gen state as string method.
   #
   if ($::SMS{$sm}{GEN_AS_STRING})
   {
      my $method_name= "CurrentStateName";

print <<EOF;

   /**
    * Return the current state as a string.
    *
    * \@return The current state as a string.
    */
EOF
      psig("virtual const char*", "$method_name(void) const;");
   }


   if ($is_module)
   {
print <<EOF;

   /**
    * Set the module that state machine uses to control debug.
    *
	* \@param The module the state machine to use to control debug.
    */
EOF

      psig("void",  "StateMachineModule(Module& module) { mpModule= &module; }");

print <<EOF;

   /**
    * Return the module that state machine uses to control debug.
    *
    * \@return The module that state machine uses to control debug.
    */
EOF

      psig("Module*",  "StateMachineModule(void) const { return mpModule; }");
   }


   # Gen state as string method.
   #
   if ($::SMS{$sm}{GEN_EFORWARDER})
   {
print <<EOF;

   /**
    * Pure virtual method that must be implemented to forward an event action
	* to the correct thread context. The action's Doit method will invoke the
	* correct event on the correct state machine. Only events triggered via 
	* InjectEvent will cause a call to FwdEvent. The event invocation is 
	* encapsulated in an Action object because the event must be executed in 
	* a different thread than the call to InjectEvent. The event will trigger
	* any number of different actions that must not occur in the caller's
	* thread (generally speaking).
    *
    * \@return LnStatus.
	*
	* \@see Actor
	* \@see #InjectEvent
    */
EOF
      psig("virtual LnStatus", "FwdEvent(Action* action) const = 0;");
   }

   print "\n";

   # Gen protectected methods.
   #
   print "protected:\n\n";

   # Gen actions
   #
   gen_actions($sm, "pure");

   
   # Gen timer actions
   #
   print "\n// Timers\n";
   foreach $action (keys %{$::SMS{$sm}{TIMER_ACTION_LIST}})
   {
print <<EOF;

   /** 
    * Timer manipulation method.
    *
    * \@return $::CFG{NO_ERR} If the operation succeeded. 
    * \@return $::CFG{IS_ERR} If the operation failed.
    */
EOF
      psig("virtual $::CFG{ERR_TYPE}", "$action();") unless $action=~ /\(/;
      psig("virtual $::CFG{ERR_TYPE}", "$action;") if $action=~ /\(/;
   }

   # Gen methods 
   #
   print "\n// Methods\n\n";
   foreach $method (@{$::SMS{$sm}{METHOD}})
   {
      my $type= $method->{TYPE};
      $type=~ s/virtual\s+//;

print <<EOF;

   /**
    * State machine defined method.
    *
    * \@return $type
    */
EOF
      psig($method->{TYPE}, gen_method_body($method->{BODY}, "pure"));
   }

   # Gen ifs 
   #
   print "\n// If\n\n";
   foreach $method (@{$::SMS{$sm}{IF_METHODS}})
   {
      my $type= $method->{TYPE};
      $type=~ s/virtual\s+//;

print <<EOF;

   /**
    * $method->{BODY} is used as an if test in a transition test.
    *
    * \@return $type
    */
EOF

      psig($method->{TYPE}, gen_method_body($method->{BODY}, "pure"));
   }

   # Gen state setting
   #
print <<EOF;

   /**
    *  Set the current state for the state machine.
    *
    * \@param state The state to make the current state for the state machine.
    */
EOF

   # psig("void",  "NextState($state_type state) { mPrevState= mState; mState= state; }")
   psig("void",  "NextState($state_type state) { if (state != mState) {mPrevDifferentState= mState;} mPrevState= mState;  mState= state;  }")

      if is_next_inline($sm); 

   psig("void",  "NextState($state_type state);")
      unless is_next_inline($sm); 


   if ($::SMS{$sm}{GEN_NOTIFY_CALL})
   {
      print <<EOT;

   /**
    * This method is invoked when the state machine has changed state.
	* Get the new state using CurrentState.
	* It is generated when GEN_NOTIFY_CALL is set to 1 in the fgen
	* state machine description.
	*
	* \@see #CurrentState
	*/
EOT

		psig("virtual void", "SmChangedState() = 0;"); 
   }

   # Gen protected class attributes:
   #
   print "\nprotected:\n";

   # Gen attributes.
   #
   while (($key, $value) = each %{$::SMS{$sm}{ATTRIBUTE}}) 
   {
      psig($value, $key . ";"); 
   }


   # Gen module pointer.
   #
   psig("$::CFG{MODULE_CLASS}*", "mpModule;") if $is_module;

   # Gen timers.
   #
   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      psig("$timer *", "mp" .  $timer . ";");
   }


   # Gen private class attributes:
   #
   print "\nprivate:\n";

   # Gen state attribute.
   #
   psig($state_type, "mState;");


   # Gen previous state attribute.
   #
   psig($state_type, "mPrevState;");


   # Gen previous different state attribute.
   #
   psig($state_type, "mPrevDifferentState;");


   # Gen mutex attribute.
   #
   psig("LnMutex", "mProtection;") if $::SMS{$sm}{IS_PROTECT};


   # Gen mutex attribute.
   #
   psig("RWCString", "mDebugId;");


   # Gen state entry and exit routines.
   #
   if ($::SMS{$sm}{ON_COUNT} > 0)
   {
      psig("void",  "DoOnExit($state_type state);");
      psig("void",  "DoOnEntry($state_type state);");
   }


   print "};\n\n\n";


   # Generate event forwarding.
   #
   if ($::SMS{$sm}{GEN_EFORWARDER})
   {
      my $action_class  = $sm . "Action";
	  my $action_typedef= $sm . "Methodp";

      print <<EOT;

// Typedef to for a pointer to a state machine method that returns LnStatus.
typedef LnStatus ($sm\:\:* $action_typedef) ();


class $action_class : public Action
{
public:
	$action_class($sm* pSm, $action_typedef action)
		: mpSm(pSm), mAction(action)
	{}

	virtual void Doit(void)
	{
	   // Make the call back to the state machine event.
	   // We are invoking a pointer to a method in the sync state machine.
  	   //
	   (mpSm->* mAction)();
	   Destroy();   // get rid of the action
	}


private:
	$sm*            mpSm;
	$action_typedef mAction;
};


EOT
   }# if gen eforwarder


   ##
   # end include guard
   #
   print "#endif // _", $::SMS{$sm}{NAME}, "_h_\n";


   close(SMH) unless $::CFG{IS_STDOUT};  # close output file

}# gen_sm_class


sub gen_sm_impl
{
   my($sm)= @_;
   my $sm_name  = $::SMS{$sm}{NAME};
   my $is_module= is_sm_module($sm);
   my $dlevel   = $::SMS{$sm}{DEBUG_LEVEL} || $::CFG{DEBUG_LEVEL};

   ::D1("lngen:gen_sm_impl: SM=$sm");

   my($fname)= $::SMS{$sm}{NAME} . "." . $::CFG{CCEXT};
   open(SMC, ">$fname") || die("FAIL open: $fname: $!") 
      unless $::CFG{IS_STDOUT};
   select(SMC) unless $::CFG{IS_STDOUT};

   # Set defaults.
   #
   my $is_state_type_provided= $::SMS{$sm}{STATE_TYPE};
   my $state_type= $::SMS{$sm}{STATE_TYPE} || "State";

   # Include header file.
   #
   print "// NOTE: this code is auto generated. Your changes will be \n";
   print "// written over. Derive from the base class to make changes.\n\n\n";

   print "#include \"Osencap/LnLockGuard.h\"\n" 
      if $::SMS{$sm}{IS_PROTECT};

   print "#include \"$::SMS{$sm}{NAME}.h\"\n\n\n";

   # Gen timer callback implementations.
   #
   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      $fire= $timer . "Fire";

      print "void $timer\:\:HandleTimer()\n";
      print "{\n";
      print "   mpParent->$fire();\n";
      print "}\n\n";

   }# foreach timer 

   # Gen Constructor.
   #
   print "$sm_name\:\:$sm_name($::CFG{MODULE_CLASS}* module, const char* pDebugId)\n" 
      if $is_module;

   print "$sm_name\:\:$sm_name(const char* pDebugId)\n" unless $is_module;

   print "   : mProtection(LN_SEM_Q_PRIORITY, false/*no priinv*/)" 
      if $::SMS{$sm}{IS_PROTECT};

   print "\n{\n";
   print "   mState= $::SMS{$sm}{START};\n";
   print "   mPrevState= $::SMS{$sm}{START};\n";
   print "   mPrevDifferentState= $::SMS{$sm}{START};\n";
   print "   mDebugId= pDebugId;\n";

   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      print "   mp$timer= new $timer;\n";
      print "   mp$timer->mpParent= this;\n";
   }

   print "   mpModule= module;\n" if $is_module;

   while (($key, $value) = each %{$::SMS{$sm}{ATTRIBUTE}}) 
   {
      my $init= $::SMS{$sm}{ATTRIBUTE_VAL}{$key};
      print "   $key= $init;\n"; 
   }


   print "\n}\n\n";

   # Gen timers
   #
   foreach $timer (keys %{$::SMS{$sm}{TIMER_LIST}})
   {
      print "$::CFG{ERR_TYPE}\n", $::SMS{$sm}{NAME}, "::Start$timer(int dmsecs, int imsecs)\n{\n";
      print "   mp$timer->$::CFG{TIMER_SET}(dmsecs, imsecs);\n   return $::CFG{NO_ERR};\n}\n\n\n"; 

      print "$::CFG{ERR_TYPE}\n", $::SMS{$sm}{NAME}, "::Stop$timer()\n{\n";
      print "   mp$timer->$::CFG{TIMER_CANCEL}();\n   return $::CFG{NO_ERR};\n}\n\n\n"; 
   } 


   # Gen state transitions for each event.
   #
   foreach $event (keys %{$::SMS{$sm}{EVENT_LIST}})
   {
      my(%state_seen);

      print "$::CFG{ERR_TYPE}\n";
      my $args= $::SMS{$sm}{EVENT_ARGS}{$event};
      print "$::SMS{$sm}{NAME}", "::", "$event($args)\n{\n";
      print "   ", sm_debug($sm_name, $dlevel, "\"$sm_name:$event:start\"")
		  if $::CFG{GEN_DEBUG};

      print "\n   $::CFG{ERR_TYPE} rc= $::CFG{NO_ERR};\n\n";

      print "   LnLockGuard lock(mProtection);\n" if $::SMS{$sm}{IS_PROTECT};


      print "   switch (CurrentState())\n   {\n";
      my $is_first= 0;
      my $is_wantout= 0;  # controls goto label

      # Walk through each state looking for states for this event.
      #
      foreach $state (keys %{$::SMS{$sm}{STATES}})
      {
         ::D2("STATE:$state EVENT:$event");

	 my $is_if= undef; 

         foreach $transition (@{$::SMS{$sm}{STATES}{$state}})
         {
            ::D2("Top transition state=$state event=$event");

            if ($$transition{ON} eq $event)
            {
               ::D2("NEXT= $$transition{NEXT}");
               ::D2("ON  = $$transition{ON}");
               ::D2("DO  = $$transition{DO}");
               ::D2("IF  = $$transition{IF}");
           
               my $tdlevel= $$transition{DLEVEL} || $dlevel;

               unless ($state_seen{$state}) 
               {  
                  my(@case_list)= split(/,/, $state);
                  print "   break;\n\n" if $is_first;
                  foreach $case (@case_list) {
                     print "   case $case:\n";
                  }
		  $is_first++;
               }

               if ($$transition{IF})
               {
                  $is_if++;

                  if ($$transition{FNEXT})
                  {
                     if ($::CFG{GEN_DEBUG})
                     {
                        print "         ", sm_debug($sm_name, $tdlevel, 
                              " \" $state\: NEXT=$$transition{NEXT}\""); 
		        print "\n";
                     }
                     print "     NextState(", $$transition{NEXT}, ");\n"
                  }

                  my $parens= "";   
                  $parens= "()" unless $$transition{IF}=~ /\(/;

                  print "      {\n";
                  print "      if ($$transition{IF}$parens)\n";
               }
               print "      {\n";

               if ($::CFG{GEN_DEBUG} and ! $$transition{FNEXT})
               {
                  print "         ", sm_debug($sm_name, $tdlevel, 
                        " \"$state\: NEXT=$$transition{NEXT}\"");
				  print "\n";
               }

               my $onerr= "if (rc) return rc;";

               if ($$transition{ONERR})
               {
                  if ($$transition{ONERR} eq "PREVSTATE")
                  { 
                     print "         "; 
                     print "State prev_state= CurrentState();\n";
                     $onerr= "if (rc) { NextState(prev_state); return rc; }";
                  }
                  elsif ($$transition{ONERR} =~ /\(/)
                  {
                     $onerr= "if (rc) { $$transition{ONERR}; return rc; }";
                  }
                  else
                  {
                     $onerr= "if (rc) { NextState($$transition{ONERR}); return rc; }";
                  }
               }

               print "      NextState(", $$transition{NEXT}, ");\n"
                  unless $$transition{FNEXT} and $$transition{IF};

               foreach $do (@{$$transition{DO_LIST}})
               {
                  if ($do=~ /TIMED_BY/)
                  {
                     my %tmp= split(/\s+/, $do);
                     my $timed_by = $tmp{TIMED_BY};
                     my ($dmsecs, $imsecs)= (format_timer_period($tmp{PERIOD}));

                     # Order when the time starts. The timer can start
                     # before the action or after.
                     #
		     my $args= $::SMS{$sm}{DO_CALL}{$tmp{DO}};
                     my $order= $::SMS{$sm}{ORDER}{DO}{$tmp{DO}}{$timed_by};
                     if ($order eq "BEFORE")
                     {
                        print "      Start$timed_by" . "($dmsecs, $imsecs);\n";
                        print "      rc= $tmp{DO}($args);\n";
	             }
	             else
		     {
                        print "      rc= $tmp{DO}($args);\n";
                        print "      Start$timed_by" . "($dmsecs, $imsecs);\n";
		     }
                  }
                  elsif ($do=~ /TIMER_STOP/)
                  {
                     my @args= split(/\W+/, $do);
                     print "      Stop" . $args[2] . "();\n";
                  }
                  else
                  {
                     $do=~ s/DO\W+//;   # remove DO to get to the action
		     my $args= $::SMS{$sm}{DO_CALL}{$do};
                     print "      rc= $do($args);\n" unless $do=~ /\(/;
                     print "      rc= $do;\n"   if     $do=~ /\(/;
                  }        
              }# foreach DO


              if ($is_if)
			  {
			     print "      goto wantout;\n";
				 $is_wantout++; # mark that the label should be generated
			  }

              print "      break;\n" unless $is_if;
              print "      }\n";

              $state_seen{$state}= 1;

           }# if target event

           if ($is_if)
           {
              print "      }\n";
              $is_if= undef;
           }

         }# foreach transition

      }# foreach state


      # Gen the end of the switch statement. If an error case was provided
      # then use it. Otherwise form one from debug.
      #
      print "   break;\n\n";
      print "   default:\n";
      print "   {\n";
	  if ($::SMS{$sm}{CASE_ERR})
	  {
         $tmp= "      $::SMS{$sm}{CASE_ERR}";
	  }
	  else
	  {
		  $tmp= sm_debug($sm, $dlevel, "\"UNHANDLED EVENT=event\"");
	  }
      $tmp=~ s/event/$event/g;
      print "      $tmp\n";
      print "   }\n";
      print "   break;\n\n";
      print "   }// switch\n\n";

      # Gen close of event
      #
      print "wantout:\n" if $is_wantout;
      print "   return rc;\n";
      print "\n}// $event\n\n\n";

   }# foreach event


   # Generate NextState
   #
   unless (is_next_inline($sm))
   {
      my $class= $::SMS{$sm}{NAME} . "\:\:" . "NextState($state_type state)";

print <<EOT;
void
$class
{
EOT

      if (is_node_sm($sm))
      {
print <<EOT;
   // Call any code that should executed when leaving a state.
   // Don't trigger call if transitioning to the same state.
   //  
EOT
		   if ($::CFG{IGNORE_SAME_ENTRY_EXIT_TRANSITION})
		   {
print <<EOT;
   if (state != mState)
   {
      DoOnExit(CurrentState());
   }

EOT
           }
		   else
		   {
print <<EOT;
   DoOnExit(CurrentState());

EOT
           }

	}# if sm

print <<EOT;
   // Set the prvious sate.
   mPrevState= mState;

   // Set the prvious sate.
   mPrevDifferentState= mState;

   // Set the new state.
   mState= state;

EOT

      if ($::SMS{$sm}{GEN_NOTIFY_CALL})
      {
print <<EOT;
   SmChangedState();

EOT
      }

      if (is_node_sm($sm))
      {
print <<EOT;
   // Call any code that should executed when entering a state.
   // Don't trigger call if transitioning to the same state.
   //
EOT
         if ($::CFG{IGNORE_SAME_ENTRY_EXIT_TRANSITION})
		 {
print <<EOT;
   if (state != mPrevState)
   {
      DoOnEntry(CurrentState());
   }

EOT
         }
		 else
		 {
print <<EOT;
   DoOnEntry(CurrentState());

EOT
		 }

      }# if sm

print <<EOT;
}// NextState
EOT
   }


   # Generate on entry method
   #
   if ($::SMS{$sm}{ON_COUNT} > 0)
   {
      my $key;
      my $on_entry_count= 0;
      foreach $key (keys %{$::SMS{$sm}})
      {
         $on_entry_count++ if $::SMS{$sm}{ON_ENTRY}{$key};
      }

      my $class= $::SMS{$sm}{NAME} . "\:\:";
	  $class.= $on_entry_count ? "DoOnEntry($state_type state)" : "DoOnEntry(State /*state*/)";

      print <<EOT;


void
$class
{
EOT

   if ($on_entry_count)
   {
print <<EOT;

   $::CFG{ERR_TYPE} rc= $::CFG{NO_ERR};

   switch (state)
   {
EOT

   my $key;
   foreach $key (keys %{$::SMS{$sm}})
   {
      my $action= $::SMS{$sm}{ON_ENTRY}{$key};
      next unless $action;

      print <<EOT;
   case $key:
EOT

   gen_do($sm, $action);

      print <<EOT;
   break;

EOT

    }# foreach state

    print <<EOT;

	default:
		break;

   }// switch
EOT

   }# if on entries

print <<EOT;

}// DoOnEntry

EOT
   }# if any exit/entry states


   # Generate on exit method
   #
   if ($::SMS{$sm}{ON_COUNT} > 0)
   {
      my $key;
      my $on_exit_count= 0;
      foreach $key (keys %{$::SMS{$sm}})
      {
         $on_exit_count++ if $::SMS{$sm}{ON_EXIT}{$key};
      }

      my $class= $::SMS{$sm}{NAME} . "\:\:";
	  $class.= $on_exit_count ? "DoOnExit($state_type state)" : "DoOnExit($state_type /*state*/)";


print <<EOT;

void
$class
{
EOT

   if ($on_exit_count)
   {
print <<EOT;
   $::CFG{ERR_TYPE} rc= $::CFG{NO_ERR};

   switch (state)
   {
EOT

   foreach $key (keys %{$::SMS{$sm}})
   {
      my $action= $::SMS{$sm}{ON_EXIT}{$key};
      next unless $action;

print <<EOT;
   case $key:
EOT

   gen_do($sm, $action);

print <<EOT;
   break;

EOT

    }# foreach state

print <<EOT;

    default:
        break;

   }// switch
EOT

   }# if on exit clauses

print <<EOT;

}// DoOnExit

EOT

   }# if any exit/entry states


   # Gen InjectEvent.
   #
   if ($::SMS{$sm}{GEN_INJECT_EVENT})
   {
      my $method_name= "InjectEvent";
      my $class= $::SMS{$sm}{NAME} . "\:\:" . "$method_name(const char* pEventName)";

      print <<EOT;

LnStatus  
$class
{
EOT

      foreach $event (keys %{$::SMS{$sm}{EVENT_LIST}})
      {
         if ($::SMS{$sm}{GEN_EFORWARDER})
		 {
	        my $action_class  = $sm . "Action";

	 print <<EOT;
   if (strcmp("$event", pEventName) == 0)
      return FwdEvent(new $action_class(this, &$sm\:\:$event));

EOT
         }
		 else
		 {
	 print <<EOT;
   if (strcmp("$event", pEventName) == 0)
      return $event();

EOT
         }
      }# foreach event

      print <<EOT;

   return LN_FAIL;

}// InjectEvent


EOT

   }


   # Generate method implementations.
   #
   while (($key, $value) = each %{$::SMS{$sm}{IMPL}}) 
   {
      psig($value, $key . ";"); 
   }


   # Generate method to return the current state as a string.
   #
   if ($::SMS{$sm}{GEN_AS_STRING})
   {
      my $method_name= "CurrentStateName";
      my $class= $::SMS{$sm}{NAME} . "\:\:" . "$method_name(void) const";

      print <<EOT;
const char*
$class
{
   switch (CurrentState())
   {
EOT
      foreach $state (keys %{$::SMS{$sm}{STATES}}) 
      {
	 print <<EOT;
   case $state: return "$state";
EOT
   }

      print <<EOT;

   default:
      break;

   }// switch

   return 0;

}// CurrentStateName()
EOT

   }


   close(SMC) unless $::CFG{IS_STDOUT};  # close output file

}# gen_sm_impl


sub gen_actions
{
   my($sm, $pure)= @_;

   $pure= ($pure) ? "= 0" : "";

   print "// Actions\n";
   foreach $item (keys %{$::SMS{$sm}{ACTION_LIST}})
   {
      my($type, $action)= split_action_list($item);

print <<EOF;

   /**
    * Action $action. Actions are because events happen.
    *
EOF
      # If there is event documentation then generate it.
      #
      my $doc= $::SMS{$sm}{DO_DOC}{$action};
      print wrap("    * ", "     * ", $doc), "\n" if $doc;

print <<EOF;
    *
    * \@return $type
    */
EOF

      my $args= $::SMS{$sm}{DO_ARGS}{$action};
      psig("virtual $type", "$action($args)$pure;") 

   }# foreach action

}# gen_actions


sub gen_method_body
{
   my($body, $pure)= @_;

   # If the caller want pure then set it up.
   #
   $pure= ($pure) ? " = 0" : "";

   # Add a void function clause if it doesn't any arguments.
   #
   $body.= "(void)" unless $body=~ /\(/;

   # If a pure virtual is wanted then we need to get rid of
   # any ; that intefere with ; after the pure virtual.
   #
   $body=~ s/;// if $pure;

   # Add in the pure virtual part.
   #
   $body.= "$pure;";

   return $body;

}# gen_method_body



sub gen_includes
{
   foreach $path (split(/\s+/, $::CFG{INC})) {
      print "#include $path\n";
   }

}# gen_includes


sub psig
{
   print fmtsig(@_[0], @_[1]);

}# psig


##
# Format a line in a class interface according to this template:
#
#///X///////////////////////X////////////////////X/////////////////////////////
#
sub fmtsig
{
   my($type, $func)= @_;
   my($line)       = " " x 3 . $type;
   my($diff)       = (length($line) < 27) ? 27 - length($line)
                                          : 48 - length($line);
   $line.=  " " x $diff;
   $line.= $func . "\n";

   return $line;

}# fmtsig


##
# Get the class header for the passed in file.
# The file header is delimitted by:
# //
# ///
#
sub get_class_header
{
   my($src)     = @_;
   my($is_found)= 0;
   my($impl)    = "";

   foreach (@{$src})
   {
      if ($is_found)
      {  
         $impl.= $_;
         last if /\/\/\//;
      }  

      unless ($is_found)
      {
         $is_found++ if /^\/\//;
         $impl.= $_;
      }

   }# foreach line in source

   return $impl 

}# get_class_header


sub split_action_list
{
   my($type, @restof)= split(/,/, @_[0]);
   my($action)= join(",", @restof);

   ::strip($type);
   ::strip($action);
   $action= $type unless $action;

   return ($type, $action);

}# split_action_list


sub sm_debug
{
   my $sm   = shift;
   my $level= shift;
   my $out  = shift;

   return "" unless $::CFG{GEN_DEBUG};

   $out=  "mDebugId << \": \" << " . $out;

   my $is_module= is_sm_module($sm);
   my $debug    = $::CFG{DEBUG_FMT};

   $debug=~ s/XMOD/mpModule/ if $is_module;
   $debug=~ s/XMOD/0/        unless $is_module;
   $debug=~ s/XLVL/$level/;
   $debug=~ s/XDATA/$out/;

   return $debug;

}# sm_debug



sub is_sm_module
{
   my $sm= shift;

   return 0 unless $sm;
   return $::SMS{$sm}{IS_MODULE};

   # is_agent when genning agents

}# is_sm_module


sub gen_do
{
   my $sm        = shift;
   my $transition= shift;
   my $do;

   foreach $do (@{$$transition{DO_LIST}})
   {
      if ($do=~ /TIMED_BY/)
      {
         my %tmp= split(/\s+/, $do);
         my $timed_by = $tmp{TIMED_BY};
         my ($dmsecs, $imsecs)= (format_timer_period($tmp{PERIOD}));

         # Order when the time starts. The timer can start
         # before the action or after.
         #
	 my $args= $::SMS{$sm}{DO_CALL}{$tmp{DO}};
         my $order= $::SMS{$sm}{ORDER}{DO}{$tmp{DO}}{$timed_by};
         if ($order eq "BEFORE")
         {
	    print "      Start$timed_by" . "($dmsecs, $imsecs);\n";
            print "      rc= $tmp{DO}($args);\n";
	 }
	 else
	 {
            print "      rc= $tmp{DO}($args);\n";
            print "      Start$timed_by" . "($dmsecs, $imsecs);\n";
         }
      }
      elsif ($do=~ /TIMER_STOP/)
      {
	 my @args= split(/\W+/, $do);
         print "      Stop" . $args[2] . "();\n";
      }
      else
      {
	 $do=~ s/DO\W+//;   # remove DO to get to the action
	 my $args= $::SMS{$sm}{DO_CALL}{$tmp{DO}};
         print "      rc= $do($args);\n" unless $do=~ /\(/;
         print "      rc= $do;\n"   if     $do=~ /\(/;
      }        
   }# foreach DO

}# // gen_do


sub is_next_inline
{
   my $sm= shift;

   return 1 if $::SMS{$sm}{ON_COUNT} <= 0 && ! $::SMS{$sm}{GEN_NOTIFY_CALL};

   return 0;

}# is_next_inline


sub is_node_sm
{
   my $sm= shift;

   return 1 if $::SMS{$sm}{ON_COUNT} > 0;

   return 0;

}# is_node_sm

sub format_timer_period
{
   my $period= shift;
   my($dmsecs, $imsecs)= split(/\./, $period); 

   $dmsecs= ::gen_method_call($dmsecs);
   $imsecs= ::gen_method_call($imsecs); 

   return ($dmsecs, $imsecs);

}# format_timer_period


1; # for require