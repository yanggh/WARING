#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x01;
const uint16_t UT_INPUT_REGISTERS_NB = 0x2;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A, 0x000B };

const uint16_t UT_REGISTERS_NB = 0x3;

int main(int argc, char *argv[])
{
    uint16_t *tab_rp_registers = NULL;
    modbus_t *ctx = NULL;
    int i;
    int rc;
    
    ctx = modbus_new_tcp("192.168.34.28", 1502);
    modbus_set_slave(ctx, 1);

    if (ctx == NULL) {
        syslog(LOG_ERR, "Unable to allocate libmodbus context\n");
        return -1;
    }

    if (modbus_connect(ctx) == -1) {
        syslog(LOG_ERR, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the registers */
    int nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));

    while(1)
    {
        memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

        /** INPUT REGISTERS **/
        rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
                UT_INPUT_REGISTERS_NB,
                tab_rp_registers);

        for (i=0; i < rc; i++) {
            syslog(LOG_INFO, "%02x ", tab_rp_registers[i]);
        }
        sleep(1);
    }

    /* Free the memory */
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);
    ctx = NULL;

    return  0;
}

