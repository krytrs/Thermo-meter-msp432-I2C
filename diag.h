/*
 * diag.h
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *      this function wil flashing with a led and will do diagnostick
 *
 */

#ifndef diag_H_
#define diag_H_

#define DK_DIAG_STATUS_OK       0        // diagnostika skoncila stavem v poradku
#define DK_DIAG_STATUS_CHYBA    1        // diagnostika skoncila neuspesne

// zadefinovani verejnych funkci
int diag(void);

#endif /* diag_H_ */


