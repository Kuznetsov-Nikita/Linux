#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("NikEvKuz kuznetsov.ne@phystech.edu");
MODULE_DESCRIPTION("Simple keyboard handler");
MODULE_VERSION("0.1");


#define COUNTING_TIME_MS 60000


static const unsigned int KEYBOARD_IRQ = 1;

static atomic_long_t counter = ATOMIC_LONG_INIT(0);

static struct tasklet_struct task;
struct task_struct* printer_task;


static irqreturn_t irq_handler(int irq, void* device_id) {
	tasklet_schedule(&task);
	return IRQ_HANDLED;
}

void tasklet_handler(unsigned long data) {
	atomic_long_inc(&counter);
}

int printer(void* data) {
	unsigned int count = 0;

	while (!kthread_should_stop()) {
		msleep(COUNTING_TIME_MS);
		count = atomic_long_xchg(&counter, 0);
		printk(KERN_INFO "Pressed %lu keyboard keys in this minute", count);
	}

	return 0;
}

static int __init keyboard_handler_init(void) {
	int res = request_irq(KEYBOARD_IRQ, irq_handler, IRQF_SHARED, "keyboard handler module", &task);
	if (res < 0) {
		printk(KERN_INFO "Interrupt handlers registration failed\n");
		return res;
	}

	tasklet_init(&task, tasklet_handler, 0);
	printer_task = kthread_run(printer, NULL, "keyboard handler printer");

	printk(KERN_INFO "Keyboard handler module created\n");

	return 0;
}

static void __exit keyboard_handler_exit(void) {
	kthread_stop(printer_task);
	tasklet_kill(&task);
	free_irq(KEYBOARD_IRQ, &task);

	printk(KERN_INFO "Keyboard handler module destroyed\n");
}

module_init(keyboard_handler_init);
module_exit(keyboard_handler_exit);
