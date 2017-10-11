/*
 * AI_touch_notifier.c
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
//#include <linux/AI_touch_notifier.h>

static BLOCKING_NOTIFIER_HEAD(AI_touch_notifier_list);

void AI_touch_register_notify(struct notifier_block *nb) {
	blocking_notifier_chain_register(&AI_touch_notifier_list, nb);

}
EXPORT_SYMBOL_GPL(AI_touch_register_notify);

void AI_touch_unregister_notify(struct notifier_block *nb) {
	blocking_notifier_chain_unregister(&AI_touch_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(AI_touch_unregister_notify);
