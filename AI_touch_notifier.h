/*
 * AI_touch_notifier.h
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

#ifndef AI_TOUCH_NOTIFIER_H_
#define AI_TOUCH_NOTIFIER_H_

#include "test_flags.h"

void AI_touch_register_notify(struct notifier_block *nb);
void AI_touch_unregister_notify(struct notifier_block *nb);


#endif /* AI_TOUCH_NOTIFIER_H_ */
