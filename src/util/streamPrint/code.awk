# awk script to preprocess vrpn code

# TODO:
#   find out how Awk counts lines & if we can do NUM_LINES automatically
#   check that #line works on non-SGI platforms

# CONNECTION_NAME myconnection
#  =>
# all future instances of 'connection' replaced by 'myconnection'

# REGISTER_TYPE foo
#  =>
#   foo_type = connection->register_message_type("foo");

# REGISTER_HANDLER foo
#  =>
#   connection->register_handler(foo_type, handle_foo, NULL)

# REGISTER_HANDLER foo bar
#  =>
#   connection->register_handler(foo_type, handle_foo, bar)

# REGISTER_HANDLER foo bar baz
#  =>
#   connection->register_handler(foo_type, handle_foo, bar, baz)

# VRPN_HANDLER foo
#   int32 x
#   float32 y
#   char z 4
# {
#   ...
# }
#  =>
# static int handle_foo (void * userdata, vrpn_HANDLERPARAM p) {
#   const char * buffer = p.buffer;
#   vrpn_int32 x;
#   vrpn_unbuffer(&buffer, &x);
#   vrpn_float32 y;
#   vrpn_unbuffer(&buffer, &y);
#   char z [4];
#   vrpn_unbuffer(&buffer, &z, 4);
#  ...
# }

# This program is a state machine with two states:
#    reading_text and reading_arguments
#
# If in reading_text mode it runs across a line beginning with VRPN_HANDLER,
#    it treats the next word on that line as the name of the function and
#    moves into reading_arguments mode.
# If in reading_text mode it runs across a line beginning
# All other lines encountered in reading_text are printed verbatim.
#
# If in reading_arguments mode it runs across a line with { it moves
#    back into reading_text mode.
# All other lines encountered in reading_arguments are expected to have
#    two fields:  type and variable name.  A variable is declared of the
#    appropriate vrpn_ type and then vrpn_unbuffer is called to unpack it
#    from the message buffer.

BEGIN { READING_ARGS = 0;
        CONNECTION_NAME = "connection";
        CLASS_NAME = "";
        CLASS_PREFIX = "";
        MEMBER_VARIABLE_PREFIX = "";
        NUM_LINES = 0; }

{ NUM_LINES++; }

!READING_ARGS && $1 ~ /CONNECTION_NAME/ \
       { CONNECTION_NAME = $2;
         next }

!READING_ARGS && $1 ~ /CLASS_NAME/ \
       { CLASS_NAME = $2;
         CLASS_PREFIX = $2 "::";
#        print "Class named " CLASS_NAME " has prefix " CLASS_PREFIX;
         next }

!READING_ARGS && $1 ~ /MEMBER_VARIABLE_PREFIX/ \
       { MEMBER_VARIABLE_PREFIX = $2 }


!READING_ARGS && $1 ~ /REGISTER_TYPE/ \
       { print "  " MEMBER_VARIABLE_PREFIX $2 "_type = " CONNECTION_NAME \
               "->register_message_type (\"" MEMBER_VARIABLE_PREFIX $2 "\");";
         next }
               
!READING_ARGS && $1 ~ /REGISTER_HANDLER/ && NF == 2 \
       { print "  " CONNECTION_NAME "->register_handler (" \
               MEMBER_VARIABLE_PREFIX $2 "_type, " \
               "handle_" $2 ", NULL);";
         next; }

!READING_ARGS && $1 ~ /REGISTER_HANDLER/ && NF == 3 \
       { print "  " CONNECTION_NAME "->register_handler (" \
               MEMBER_VARIABLE_PREFIX $2 "_type, " \
               "handle_" $2 ", " $3 ");";
         next; }

!READING_ARGS && $1 ~ /REGISTER_HANDLER/ && NF == 4 \
       { print "  " CONNECTION_NAME "->register_handler (" \
               MEMBER_VARIABLE_PREFIX $2 "_type, " \
               "handle_" $2 ", " $3 ", " $4 ");";
         next; }

!READING_ARGS && $1 ~ /VRPN_HANDLER/ \
       { print "#line " NUM_LINES;
         print "static int handle_" $2 \
               " (void * userdata, vrpn_HANDLERPARAM p) {"; \
         NUM_READ = 0;
         READING_ARGS = 1;
         next }

!READING_ARGS && $1 ~ /VRPN_HANDLER_MEMBER/ \
       { print "#line " NUM_LINES;
         print "int " CLASS_PREFIX "handle_" $2 \
               " (void * userdata, vrpn_HANDLERPARAM p) {"; \
         NUM_READ = 0;
         READING_ARGS = 1;
         next }

# handle char x n

READING_ARGS && $1 ~ /char/ \
       { if (!NUM_READ)
           printf "  const char * buffer = p.buffer;";
         print "  char " $2 " [" $3 "];";
         print "  vrpn_unbuffer(&buffer, " $2 ", " $3 ");";
         NUM_READ++;
         next }

# handle (vrpn_)foo<n> x

READING_ARGS && $1 !~ /\{/ \
       { if (!NUM_READ)
           printf "  const char * buffer = p.buffer;";
         print "  vrpn_" $1 " " $2 ";";
         print "  vrpn_unbuffer(&buffer, &" $2 ");";
         NUM_READ++;
         next }

READING_ARGS && /\{/ \
       { READING_ARGS = 0;
         print "#line " NUM_LINES;
         next }

{ print }

END {}

