#include <stdio.h>
#include <unistd.h>

int authenticated(char *password) {
	int auth = 0;
	char buf[20];

	strcpy(buf, password);

	if (strcmp(password, "hunter2") == 0) {
		auth = 1;
	}

	return auth;	
}

void jumpMe() {
    printf("You did it!\n");
    exit(20);
}

int main(int argc, char **argv) {

	if (argc < 2) {
		printf("Usage: %s <password>\n", argv[0]);
		exit(0);
	}

    if (authenticated(argv[1])) {
        printf("***********\n");
        printf("* I'm in. *\n");
        printf("***********\n");
    }
    else {
        printf("Sorry, that ain't it. :(\n");
    }
}
