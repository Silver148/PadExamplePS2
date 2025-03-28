#include <libpad.h>
#include <kernel.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>
#define ROM_PADMAN
#if defined(ROM_PADMAN) && defined(NEW_PADMAN)
#error Only one of ROM_PADMAN & NEW_PADMAN should be defined!
#endif

#if !defined(ROM_PADMAN) && !defined(NEW_PADMAN)
#error ROM_PADMAN or NEW_PADMAN must be defined!
#endif

char padBuf[256] __attribute__((aligned(64)));

int load;
static char actAlign[6];
static int actuators;

void Modulos()
{
    int load;

#ifdef ROM_PADMAN
    load = SifLoadModule("rom0:SIO2MAN", 0, NULL);
#else
    load = SifLoadModule("rom0:XSIO2MAN", 0, NULL);
#endif   

#ifdef ROM_PADMAN
    load = SifLoadModule("rom0:PADMAN", 0, NULL);
#else
    load = SifLoadModule("rom0:XPADMAN", 0, NULL);
#endif 
    
    if(load < 0)
    {
        scr_printf("No se pudo cargar los modulos\n");
        SleepThread();
    }
}

int EsperandoControlador(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];

    state = padGetState(port, slot);
    lastState = -1;
    while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState) {
            padStateInt2String(state, stateString);
            scr_printf("Espere por favor, el control(%d,%d) esta en estado %s\n", 
                       port, slot, stateString);
        }
        lastState = state;
        state=padGetState(port, slot);
    }

    if (lastState != -1) {
        scr_printf("Control listo!\n");
    }
    return 0;
}

int IniciarControlador(int port, int slot)
{

    int ret;
    int modes;
    int i;

    EsperandoControlador(port, slot);

    modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
    scr_printf("The device has %d modes\n", modes);

    if (modes > 0) {
        scr_printf("( ");
        for (i = 0; i < modes; i++) {
            scr_printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
        }
        scr_printf(")");
    }

    scr_printf("It is currently using mode %d\n", 
               padInfoMode(port, slot, PAD_MODECURID, 0));

    if (modes == 0) {
        scr_printf("This is a digital controller?\n");
        return 1;
    }

    i = 0;
    do {
        if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
            break;
        i++;
    } while (i < modes);
    if (i >= modes) {
        scr_printf("This is no Dual Shock controller\n");
        return 1;
    }

    ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
    if (ret == 0) {
        scr_printf("This is no Dual Shock controller??\n");
        return 1;
    }

    scr_printf("Enabling dual shock functions\n");

    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

    EsperandoControlador(port, slot);
    scr_printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

    EsperandoControlador(port, slot);        
    scr_printf("enterPressMode: %d\n", padEnterPressMode(port, slot));

    EsperandoControlador(port, slot);
    actuators = padInfoAct(port, slot, -1, 0);
    scr_printf("# of actuators: %d\n",actuators);

    if (actuators != 0) {
        actAlign[0] = 0;
        actAlign[1] = 1;
        actAlign[2] = 0xff;
        actAlign[3] = 0xff;
        actAlign[4] = 0xff;
        actAlign[5] = 0xff;

        EsperandoControlador(port, slot);
        scr_printf("padSetActAlign: %d\n", 
                   padSetActAlign(port, slot, actAlign));
    }
    else {
        scr_printf("Did not find any actuators.\n");
    }

    EsperandoControlador(port, slot);

    return 1;
}

int main(int argc, char *argv[])
{
    int ret;
    int port, slot;
    int i;
    struct padButtonStatus buttons;
    u32 paddata;
    u32 old_pad = 0;
    u32 new_pad;
    port = 0;
    slot = 0; 

    SifInitRpc(0);

    init_scr();

    Modulos();

    padInit(0);

    padPortOpen(slot, port, padBuf);

    IniciarControlador(slot, port);

    if(IniciarControlador < 0)
    {
        scr_printf("No se pudo incializar el controlador\n");
        SleepThread();
    }

    for (;;)
    {
        i=0;
        ret=padGetState(port, slot);

        while((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1)) {
             if(ret==PAD_STATE_DISCONN) {
                scr_printf("El control(%d, %d) esta desconectado\n", port, slot);
            }
            ret=padGetState(port, slot);
        }

        if(i==1)
        {
            scr_printf("Control listo!");
        }

        ret = padRead(port, slot, &buttons);

        if(ret != 0)
        {
            paddata = 0xffff ^ buttons.btns; 
            new_pad = paddata & ~old_pad;
            old_pad = paddata;

            if(new_pad & PAD_UP)
            {
                scr_printf("Arriba\n");
            }

            if(new_pad & PAD_DOWN)
            {
                scr_printf("Abajo\n");
                
            }

            if(new_pad & PAD_LEFT)
            {
                scr_printf("Izquierda\n");
                
            }

            if(new_pad & PAD_RIGHT)
            {
                scr_printf("Derecha\n");
                
            }

            if(new_pad & PAD_CROSS)
            {
                scr_printf("X\n");
                
            }
            if(new_pad & PAD_CIRCLE)
            {
                scr_printf("O\n");
                
            }
            if(new_pad & PAD_SQUARE)
            {
                scr_printf("Cuadrado\n");
                
            }
            if(new_pad & PAD_TRIANGLE)
            {
                scr_printf("Triangulo\n");
                
            }
            if(new_pad & PAD_L1)
            {
                scr_printf("L1\n");
                
            }
            if(new_pad & PAD_L2)
            {
                scr_printf("L2\n");
                
            }
            if(new_pad & PAD_R1)
            {
                scr_printf("R1\n");
                
            }
            if(new_pad & PAD_R2)
            {
                scr_printf("R2\n");
                
            }
            if(new_pad & PAD_L3)
            {
                scr_printf("L3\n");
                
            }
            if(new_pad & PAD_R3)
            {
                scr_printf("R3\n");
                
            }
            if(new_pad & PAD_SELECT)
            {
                scr_printf("Select\n");
                
            }
            if(new_pad & PAD_START)
            {
                scr_printf("Start\n");
                
            }
            
            }    
    }

    SleepThread();

    return 0;
}

