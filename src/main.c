#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <esp_display/lcd.h>

int main(int argc, char **argv) {
    char *device = NULL;
    int columns = -1;
    int rows = -1;
    bool backlight = false;
    bool cursor = false;
    bool blink = false;
    int option;

    while ((option = getopt(argc, argv, ":d:r:c:bsl")) != -1) {
        switch (option) {
            case 'd':
                device = strdup(optarg);
                break;
            case 'r':
                rows = atoi(optarg);
                break;
            case 'c':
                columns = atoi(optarg);
		        break;
            case ':':
                fprintf(stderr, "Error: option %c takes an argument.\n", optopt);
                break;
            case 'b':
                backlight = true;
                break;
            case 's':
                cursor = true;
                break;
            case 'l':
                blink = true;
                break;
            case '?':
                fprintf(stderr, "Warning: unknown option %c\n", optopt);
                break;
	    }
    }

    if ((device == NULL) || (rows <= 0) || (columns <= 0)) {
    	fprintf(stderr, "Usage: %s -d <device> -r <rows> -c <columns>\n", argv[0]);
	    return 1;
    }

    lcd_display_p disp = lcd_connect(device, columns, rows);
    free(device);
    
    lcd_clear(disp);
    lcd_set_backlight(disp, backlight);
    lcd_set_cursor(disp, cursor);
    lcd_set_blink(disp, blink);
    lcd_print(disp, "uwu");

    lcd_disconnect(disp);

    return 0;
}
