#!/bin/awk -f

# Changed this into a one-pass solution, but that doesn't let me use
# CONNECTION_NAME, so we go back to two passes (use registry.awk to
# expand hnames into input for code.awk instead of final code)

# BEGIN { CONNECTION_NAME = "connection" }

# $1 ~ /CONNECTION_NAME/ && NF == 2 \
#   { CONNECTION_NAME = $2 }

# NF == 2 \
#   { print "  vrpn_int32 " $1 "_type;" ;
#     print "  " $1 "_type = " CONNECTION_NAME "->register_message_type (\""
#           $1 "\");";
#     print "  " CONNECTION_NAME "->register_handler (" $1 "_type, " \
#           "handle_" $1 ", NULL);"; }

# It'd be nice if we could just say "all the remaining fields", $$ or
# something, but we also need to have $1, so I can't see how to do it
# by changing the field delimiter...

NF == 1 \
  { print "  vrpn_int32 " $1 "_type;" ;
    print "REGISTER_TYPE " $1; 
    print "REGISTER_HANDLER " $1; }

NF == 2 \
  { print "  vrpn_int32 " $1 "_type;" ;
    print "REGISTER_TYPE " $1; 
    print "REGISTER_HANDLER " $1 $2; }

NF == 3 \
  { print "  vrpn_int32 " $1 "_type;" ;
    print "REGISTER_TYPE " $1; 
    print "REGISTER_HANDLER " $1 $2 $3; }


