#ifndef __region_h__
#define __region_h__


typedef struct __region_t {
		  //Countries represented by index integer (0->47) - see gdrive
		  int type; //0 - Inland, 1 - Coastal, 2 - Water
		  int occupy_type; //-1 - not occupied, 0 - army, 1 - fleet
		  int supply; //0 - no, 1 - yes
		  int player; //-1 means no unit present(0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland)
		  int ncountrys[12]; //neighbouring countries(represented by ints)

          //Below TO BE SET AND CLEARED BY ARBITRATOR
          int aS; //attack strength 
          int dS; //defense strength
} region_t;

int genStart(char outfile[]);

#endif
/*
 * See Google Drive for country to integer mapping
 *
 * --------------REGION EXCEPTIONS------------------
 *  1. fleet on coast of 17 cannot move to 9
 *  2. 25 has two coasts
 *  3. 30 can only have a fleet move to occupy from 29
 *  4. 36 cannot move to 35 if 18 is occupied by opposing force?? TODO confirm 
 *  5. 34 cannot move from 35 to 33 or 33 to 35 ?
 *  6. 26 cannot move from 16 to 10 or 10 to 16 ?
 *
 */

		  
