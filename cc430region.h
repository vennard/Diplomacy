/*
 * cc430region.h
 *
 *  Created on: Mar 17, 2014
 *      Author: Kevin
 */

#ifndef CC430REGION_H_
#define CC430REGION_H_

typedef struct __cc430region_t {

	char owner; //4 means no unit present(0 - Poland, 1 - Russia, 2 - Sweden, 3 - Finland)
	char unit_type; //0 - army, 1 - fleet

	char order;
	/*
	 * Phase 0:
	 * 	0 - Hold, 1 - Move, 2 - Support, 3 - Convoy
	 *
	 * Phase 2:
	 * 	0 - disband, 1 - retreat
	 *
	 * Phase 3:
	 * 	Menu 3: 0 - keep, 1 - disband
	 */

	char tcountry;
	/*
	 * Phase 0:
	 *	for move: destination country
	 *	for support move: destination country of supported move
	 *	for convoy: destination of troop
	 * Phase 2:
	 * 	where the unit is retreating to
	 * Phase 3:
	 * 	Menu 1:
	 * 		0 - add nothing
	 * 		1 - want to add army
	 * 		2 - want to add fleet
	 */

	char scountry;
	/*
	 * Phase 0:
	 * 	for support hold: supported units country
	 * 	for support move: supported units origin country
	 * 	for troop issued convoy: -1
	 *
	 * Phase 2:
	 * 	0 - if unit does not have to retreat, 1 - if unit does have to retreat
	 */


	//char ncountrys[12]; //neighbouring countries(represented by ints)

} cc430region_t;


#endif /* CC430REGION_H_ */
