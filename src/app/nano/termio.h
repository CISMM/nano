extern int open_raw_term(int echoEnabled);
extern int close_raw_term(int ttyFD);
extern int read_raw_term(int ttyFD, char buffer[]);
extern int set_raw_term(int ttyFD, int echoEnabled);
extern int reset_raw_term(int ttyFD);
