#include	<stdio.h>
#include	<string.h>

main()
{
	char	incoming[1000];
	int	ret, i;

	printf("Will repeat characters received.  Will exit if first\n");
	printf("character in a line is Q\n");
	do {
		ret = accept_speech_input(incoming, sizeof(incoming));
		for (i = 0; i < ret; i++) {
			printf("%c",incoming[i]);
		}
	} while ( (incoming[0] != 'Q') && (ret != -1) );
	close_speech_input();
}
