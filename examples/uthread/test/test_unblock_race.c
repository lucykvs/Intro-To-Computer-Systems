#include <uthread.h>
#include <stdlib.h>
#include <stdio.h>

void *foo(void *arg) {
	for(volatile int i=0; i<1000; i++)
		uthread_yield();
	uthread_block();
	return (void *)0xdeadbeef;
}

int main() {
	uthread_init(3);
	uthread_t t = uthread_create(foo, NULL);
	uthread_yield();
	uthread_unblock(t);
	void *result;
	uthread_join(t, &result);
	printf("%p\n", result);
}
