#include <stdio.h>
#include "pico/stdlib.h"
#include "oled.h"
#include "menu.h"
#include "joystick.h" 

int main() {
    stdio_init_all();  // Inicializa a comunicação UART
    oled_init();       // Inicializa o OLED
    joystick_init();   // Inicializa o joystick
    menu_navigation(); // Inicia a navegação no menu

    return 0;
}