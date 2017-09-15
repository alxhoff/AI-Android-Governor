/*
 * AI_gov_kernel_write.h
 *
 *  Created on: Sep 8, 2017
 *      Author: alxhoff
 */

#ifndef AI_GOV_KERNEL_WRITE_H_
#define AI_GOV_KERNEL_WRITE_H_

#include "test_flags.h"

#define KERNEL_DEBUG_MSG(...) \
            do { printk(KERN_INFO __VA_ARGS__); } while (0)

#define KERNEL_ERROR_MSG(...) \
            do { printk(KERN_ERR __VA_ARGS__); } while (0)

#define KERNEL_LOGGG_MSG(...) \
            do { printk(KERN_ERR __VA_ARGS__); } while (0)

#define KERNEL_VERBOSE_MSG(...) \
            do { printk(KERN_INFO __VA_ARGS__); } while (0)

#define KERNEL_WARNING_MSG(...) \
            do { printk(KERN_WARNING __VA_ARGS__); } while (0)

#endif /* AI_GOV_KERNEL_WRITE_H_ */
