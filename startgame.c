#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "region.h"

region_t r[48];

//Case statement for start data
//name, type, supply, player, ncountrys
//Player codes: 0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland
//type: 0 - Inland, 1 - Coastal, 2 - Water
//occupied type: -1 - not occupied 0 - Army, 1 - Fleet
int start_data(int i) {
int j;
switch (i) {
case 0: //ode
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 5; //str
r[i].ncountrys[1] = 1; //vis
for (j=2;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 1: //vis
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 0; //ode
r[i].ncountrys[1] = 5; //str
r[i].ncountrys[2] = 6; //dan
r[i].ncountrys[3] = 7; //wkr
r[i].ncountrys[4] = 2; //war
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 2: //war
r[i].type = 0;
r[i].occupy_type = 0;
r[i].supply = 1;
r[i].player = 0;
r[i].ncountrys[0] = 1; //vis
r[i].ncountrys[1] = 7; //wkr
r[i].ncountrys[2] = 8; //nar
r[i].ncountrys[3] = 3; //bre
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 3: //bre
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 2; //war
r[i].ncountrys[1] = 8; //nar
r[i].ncountrys[2] = 4; //min
for (j=3;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 4: //min
r[i].type = 0;
r[i].occupy_type = 0;
r[i].supply = 1;
r[i].player = 1;
r[i].ncountrys[0] = 3; //bre
r[i].ncountrys[1] = 8; //nar
r[i].ncountrys[2] = 9; //wes
for (j=3;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 5: //str
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 0;
r[i].ncountrys[0] = 0; //ode
r[i].ncountrys[1] = 1; //vis
r[i].ncountrys[2] = 6; //dan
r[i].ncountrys[3] = 10; //kat
r[i].ncountrys[4] = 12; //sba
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 6: //dan
r[i].type = 1;
r[i].occupy_type = 1;
  r[i].supply = 1;
r[i].player = 0;
r[i].ncountrys[0] = 5; //str
r[i].ncountrys[1] = 1; //vis
r[i].ncountrys[2] = 7; //wkr
r[i].ncountrys[3] = 14; //kon
r[i].ncountrys[4] = 12; //sba
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 7: //wkr
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 1; //vis
r[i].ncountrys[1] = 6; //dan
r[i].ncountrys[2] = 14; //kon
r[i].ncountrys[3] = 8; //nar
r[i].ncountrys[4] = 12; //sba
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
  case 8: //nar
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 2; //war
r[i].ncountrys[1] = 7; //wkr
r[i].ncountrys[2] = 14; //kon
r[i].ncountrys[3] = 17; //rig
r[i].ncountrys[4] = 9; //wes
r[i].ncountrys[5] = 4; //min
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 9: //wes
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 8; //nar
r[i].ncountrys[1] = 17; //rig
r[i].ncountrys[2] = 19; //tal
r[i].ncountrys[3] = 35; //gof
r[i].ncountrys[4] = 18; //stp
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 10: //kat
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 5; //str
r[i].ncountrys[1] = 11; //cop
r[i].ncountrys[2] = 27; //ska
r[i].ncountrys[3] = 28; //got
r[i].ncountrys[4] = 25; //bol
r[i].ncountrys[5] = 26; //mal
r[i].ncountrys[6] = 12; //sba
for (j=7;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 11: //cop
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 10; //kat
for (j=1;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 12: //sba
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 5; //str
r[i].ncountrys[1] = 10; //kat
r[i].ncountrys[2] = 26; //mal
r[i].ncountrys[3] = 15; //ron
r[i].ncountrys[4] = 16; //hab
r[i].ncountrys[5] = 13; //mba
r[i].ncountrys[6] = 14; //kon
r[i].ncountrys[7] = 6; //dan
for (j=8;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 13: //mba
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 12; //sba
r[i].ncountrys[1] = 16; //hab
r[i].ncountrys[2] = 24; //gos
r[i].ncountrys[3] = 23; //sli
r[i].ncountrys[4] = 22; //nba
r[i].ncountrys[5] = 17; //rig
r[i].ncountrys[6] = 14; //kon
for (j=7;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 14: //kon
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 6; //dan
r[i].ncountrys[1] = 12; //sba
r[i].ncountrys[2] = 13; //mba
r[i].ncountrys[3] = 17; //rig
r[i].ncountrys[4] = 8; //nar
r[i].ncountrys[5] = 7; //wkr
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 15: //ron
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 12; //sba
r[i].ncountrys[1] = 16; //hab
for (j=2;j<12;j++) r[i].ncountrys[j] = -1;
  break;
  case 16: //hab
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 15; //ron
r[i].ncountrys[1] = 12; //sba
r[i].ncountrys[2] = 26; //mal
r[i].ncountrys[3] = 25; //bol
r[i].ncountrys[4] = 24; //gos
r[i].ncountrys[5] = 13; //mba
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 17: //rig
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 1;
r[i].ncountrys[0] = 13; //mba
r[i].ncountrys[1] = 22; //nba
r[i].ncountrys[2] = 20; //gor
r[i].ncountrys[3] = 19; //tal
r[i].ncountrys[4] = 9; //wes
r[i].ncountrys[5] = 8; //nar
r[i].ncountrys[6] = 14; //kon
for (j=7;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 18: //stp
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 1;
r[i].ncountrys[0] = 9; //wes
r[i].ncountrys[1] = 35; //gof
r[i].ncountrys[2] = 36; //lla
r[i].ncountrys[3] = 37; //kuy
r[i].ncountrys[4] = 39; //sai
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 19: //tal
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 17; //rig
r[i].ncountrys[1] = 20; //gor
r[i].ncountrys[2] = 22; //nba
r[i].ncountrys[3] = 35; //gof
r[i].ncountrys[4] = 9; //wes
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 20: //gor
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 21; //saa
r[i].ncountrys[1] = 22; //nba
r[i].ncountrys[2] = 19; //tal
r[i].ncountrys[3] = 17; //rig
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 21: //saa
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 20; //gor
r[i].ncountrys[1] = 22; //nba
for (j=2;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 22: //nba
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 13; //mba
r[i].ncountrys[1] = 23; //sli
r[i].ncountrys[2] = 24; //gos
r[i].ncountrys[3] = 32; //ala
r[i].ncountrys[4] = 33; //sgo
r[i].ncountrys[5] = 34; //hel
r[i].ncountrys[6] = 35; //gof
r[i].ncountrys[7] = 19; //tal
r[i].ncountrys[8] = 20; //gor
r[i].ncountrys[9] = 21; //saa
r[i].ncountrys[10] = 17; //rig
for (j=11;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 23: //sli
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 13; //mba
r[i].ncountrys[1] = 24; //gos
r[i].ncountrys[2] = 22; //nba
for (j=3;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 24: //gos
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 25; //bol
r[i].ncountrys[1] = 31; //sto
r[i].ncountrys[2] = 33; //sgo
r[i].ncountrys[3] = 32; //ala
r[i].ncountrys[4] = 22; //nba
r[i].ncountrys[5] = 23; //sli
r[i].ncountrys[6] = 13; //mba
r[i].ncountrys[7] = 16; //hab
for (j=8;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 25: //bol
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 26; //mal
r[i].ncountrys[1] = 10; //kat
r[i].ncountrys[2] = 28; //got
r[i].ncountrys[3] = 23; //sil
r[i].ncountrys[4] = 31; //sto
r[i].ncountrys[5] = 24; //gos
r[i].ncountrys[6] = 16; //hab
for (j=7;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 26: //mal
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 10; //kat
r[i].ncountrys[1] = 25; //bol
r[i].ncountrys[2] = 16; //hab
r[i].ncountrys[3] = 12; //sba
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 27: //ska
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 10; //kat
r[i].ncountrys[1] = 28; //got
for (j=2;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 28: //got
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 2;
r[i].ncountrys[0] = 27; //ska
r[i].ncountrys[1] = 10; //kat
r[i].ncountrys[2] = 29; //van
r[i].ncountrys[3] = 25; //bol
r[i].ncountrys[4] = 30; //sil
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 29: //van
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 28; //got
r[i].ncountrys[1] = 23; //sil
for (j=2;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 30: //sil
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 29; //van
r[i].ncountrys[1] = 28; //got
r[i].ncountrys[2] = 25; //bol
r[i].ncountrys[3] = 31; //sto
r[i].ncountrys[4] = 42; //sun
r[i].ncountrys[5] = 43; //stj
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 31: //sto
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 2;
r[i].ncountrys[0] = 25; //bol
r[i].ncountrys[1] = 30; //sil
r[i].ncountrys[2] = 42; //sun
r[i].ncountrys[3] = 33; //sgo
r[i].ncountrys[4] = 24; //gos
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 32: //ala
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 24; //gos
r[i].ncountrys[1] = 33; //sgo
r[i].ncountrys[2] = 22; //nba
for (j=3;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 33: //sgo
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 32; //ala
r[i].ncountrys[1] = 24; //gos
r[i].ncountrys[2] = 31; //sto
r[i].ncountrys[3] = 42; //sun
r[i].ncountrys[4] = 41; //ngo
r[i].ncountrys[5] = 40; //vas
r[i].ncountrys[6] = 34; //hel
r[i].ncountrys[7] = 22; //nba
for (j=8;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 34: //hel
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 3;
r[i].ncountrys[0] = 22; //nba
r[i].ncountrys[1] = 33; //sgo
r[i].ncountrys[2] = 40; //vas
r[i].ncountrys[3] = 38; //kuo
r[i].ncountrys[4] = 39; //sai
r[i].ncountrys[5] = 35; //gof
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 35: //gof
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 22; //nba
r[i].ncountrys[1] = 34; //hel
r[i].ncountrys[2] = 39; //sai
r[i].ncountrys[3] = 18; //stp
r[i].ncountrys[4] = 9; //wes
r[i].ncountrys[5] = 19; //tal
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 36: //lla
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 18; //stp
r[i].ncountrys[1] = 39; //sai
r[i].ncountrys[2] = 38; //kuo
r[i].ncountrys[3] = 37; //kuy
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 37: //kuy
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 36; //lla
r[i].ncountrys[1] = 38; //kuo
r[i].ncountrys[2] = 47; //ima
r[i].ncountrys[3] = 18; //stp
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 38: //kuo
r[i].type = 1;
r[i].occupy_type = 0;
r[i].supply = 1;
r[i].player = 3;
r[i].ncountrys[0] = 36; //lla
r[i].ncountrys[1] = 39; //sai
r[i].ncountrys[2] = 34; //hel
r[i].ncountrys[3] = 40; //vas
r[i].ncountrys[4] = 46; //oul
r[i].ncountrys[5] = 47; //ima
r[i].ncountrys[6] = 37; //kuy
for (j=7;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 39: //sai
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 18; //stp
r[i].ncountrys[1] = 36; //lla
r[i].ncountrys[2] = 35; //gof
r[i].ncountrys[3] = 34; //hel
r[i].ncountrys[4] = 37; //kuo
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 40: //vas
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 3;
r[i].ncountrys[0] = 34; //hel
r[i].ncountrys[1] = 33; //sgo
r[i].ncountrys[2] = 41; //ngo
r[i].ncountrys[3] = 46; //oul
r[i].ncountrys[4] = 38; //kuo
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 41: //ngo
r[i].type = 2;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 33; //sgo
r[i].ncountrys[1] = 42; //sun
r[i].ncountrys[2] = 44; //keb
r[i].ncountrys[3] = 45; //tor
r[i].ncountrys[4] = 46; //oul
r[i].ncountrys[5] = 40; //vas
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 42: //sun
r[i].type = 1;
r[i].occupy_type = 1;
r[i].supply = 1;
r[i].player = 2;
r[i].ncountrys[0] = 30; //sil
r[i].ncountrys[1] = 43; //stj
r[i].ncountrys[2] = 44; //keb
r[i].ncountrys[3] = 41; //ngo
r[i].ncountrys[4] = 33; //sgo
r[i].ncountrys[5] = 31; //sto
for (j=6;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 43: //stj
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 30; //sil
r[i].ncountrys[1] = 42; //sun
r[i].ncountrys[2] = 44; //keb
for (j=3;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 44: //keb
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 42; //sun
r[i].ncountrys[1] = 43; //stj
r[i].ncountrys[2] = 45; //tor
r[i].ncountrys[3] = 41; //ngo
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 45: //tor
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 1;
r[i].player = -1;
r[i].ncountrys[0] = 44; //keb
r[i].ncountrys[1] = 47; //ima
r[i].ncountrys[2] = 46; //oul
r[i].ncountrys[3] = 41; //ngo
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 46: //oul
r[i].type = 1;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 41; //ngo
r[i].ncountrys[1] = 45; //tor
r[i].ncountrys[2] = 47; //ima
r[i].ncountrys[3] = 38; //kuo
r[i].ncountrys[4] = 40; //vas
for (j=5;j<12;j++) r[i].ncountrys[j] = -1;
break;
case 47: //ima
r[i].type = 0;
r[i].occupy_type = -1;
r[i].supply = 0;
r[i].player = -1;
r[i].ncountrys[0] = 45; //tor
r[i].ncountrys[1] = 46; //oul
r[i].ncountrys[2] = 38; //kuo
r[i].ncountrys[3] = 37; //kuy
for (j=4;j<12;j++) r[i].ncountrys[j] = -1;
break;
default:
printf("Case statement failed\n");
return 0;
//return -1;
break;
}
return 0;
}


//Creating output for the start condition of the game
int genStart(char out[]) {
	char* outfile = "/tmp/startgame";
	int* ptr;

	ptr = (int *) malloc(sizeof(r));

	printf("Beginning start game generation... \r\n");
	if (strlen(outfile) < 0) {
  		printf("No outfile set for game start data, resorting to /tmp/startgame\r\n");
	} else {
  		outfile = out;
	}

	//open and create output file
   int fd = open(outfile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
   if (fd < 0) {
  		perror("open");
  		exit(1);
   }

	//Load up start data
	//Just start data for 4 players supported now
	int a=0;
	for(a=0; a < 48;a++) {
		if (start_data(a) == -1) {
		fprintf(stderr,"Something went wrong calling start_data()\r\n");
		exit(1);
		}
	}

	int i=0;
	for(i=0; i < 48;i++) {
  		region_t t;
		t = r[i];
		//DEBUG OUTPUT
		printf("Region %i: ",i);
		printf("type- %i ",t.type); 
		printf("occupy_type- %i ",t.occupy_type); 
		printf("supply- %i ",t.supply);
		printf("player- %i ",t.player);
		int size = sizeof(t.ncountrys) / (sizeof(int));
		int k;
		printf("neighbours: ");
		for(k=0;k<size;k++){
			printf(" %i,",t.ncountrys[k]);
		}
		printf("\r\n");

		//Print out to file
		int rc = write(fd, &r, sizeof(r));
		if (rc != sizeof(r)) {
     		perror("write");
     		exit(1);
  		}
   }

	(void) close(fd);
	free(ptr);
	return 0;
}

