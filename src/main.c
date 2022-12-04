#include <stdio.h>

#include <esp_display/lcd.h>

int main() {
    lcd_display_p disp = lcd_connect("/dev/ttyUSB0", 20, 4);
    
    lcd_clear(disp);
    lcd_set_backlight(disp, true);
    lcd_print(disp, "uwu");

    lcd_disconnect(disp);

    return 0;
}
