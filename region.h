#ifndef __region_h__
#define __region_h__


typedef struct __region_t {
		  //Countries represented by index integer - see gdrive
		  int type; //0 - Inland, 1 - Coastal, 2 - Water
		  int occupy_type; //-1 - not occupied, 0 - army, 1 - fleet
		  int supply; //0 - no, 1 - yes
		  int player; //-1 means no unit present(0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland)
		  int ncountrys[12]; //neighbouring countries(represented by ints)
} region_t;

int genStart(char outfile[]);

#endif
/*
 * See Google Drive for country to integer mapping
 */

		  
