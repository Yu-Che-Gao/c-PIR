#include <stdio.h>  // printf, scanf
#include <stdlib.h>
#include <bcm2835.h>

#define     PIN_PIR     18
#define     STB_TIME    3


int main(int argc, char** argv)
{
    if(!bcm2835_init())
        return 1;
    bcm2835_gpio_fsel(PIN_PIR, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(PIN_PIR, BCM2835_GPIO_PUD_DOWN);
    printf("\n");
    for(int i = 0; i < STB_TIME; i++)
    {
        bcm2835_delay(1000);
        printf("      Waitting for PIR to stable, %d sec\r", STB_TIME - i);
        fflush(stdout);
    }
    
    printf("\n\n  READY ...\n\n");
    
    while(1)
    {
        if(bcm2835_gpio_lev(PIN_PIR))
        {
            printf("THE PIR SENSOR DETECTED MOVEMENT           \r");
            fflush(stdout);
            bcm2835_delay(3000);
        }
        else
            printf("PIR sensor get ready for movement detection\r");    
    }
    
    bcm2835_close();
    return 0;
}
