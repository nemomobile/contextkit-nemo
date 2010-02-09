extern "C" {
    #include "bmeipc.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int32_t	_sd = -1;



int main(int argc, char *argv[])
{
    bmestat_t st;

    if (0 > (_sd = bmeipc_open())){
        fprintf(stderr, "Failed to connect BME server, %s\n",
                strerror(errno));
        exit(errno);
    }
    printf("Connected to BME server\n");

    if (0 > bmeipc_stat(_sd, &st)) {
        fprintf(stderr, "Failed to get statistics, %s\n",
                strerror(errno));
    }

    switch (st[CHARGING_STATE]) {
    case CHARGING_STATE_STOPPED:
        if (BATTERY_STATE_FULL != st[BATTERY_STATE]) {
            fprintf(stdout,"off\n");
        } else {
            fprintf(stdout,"full\n");
        }
        break;

    case CHARGING_STATE_STARTED:
        fprintf(stdout,"chaging started\n");
        break;

    case CHARGING_STATE_SPECIAL:
        fprintf(stdout,"special\n");
        break;
    case CHARGING_STATE_ERROR:
        fprintf(stdout,"error\n");
        break;
    default:
        fprintf(stdout,"unknown\n");

    }

    switch (st[BATTERY_STATE]) {
    case BATTERY_STATE_EMPTY:
        fprintf(stdout,"empty\n");
        break;
    case BATTERY_STATE_LOW:
        fprintf(stdout,"low\n");
        break;
    case BATTERY_STATE_OK:
        fprintf(stdout,"ok\n");
        break;
    case BATTERY_STATE_FULL:
        fprintf(stdout,"full\n");
        break;
    case BATTERY_STATE_ERROR:
    default:
        fprintf(stdout,"unknown\n");
        break;
    }

    printf("level:%d bars:%d\n",st[BATTERY_LEVEL_NOW],st[BATTERY_LEVEL_MAX]);

}
