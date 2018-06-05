#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/parser.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/backing-dev.h>

#include <rdma/ib_verbs.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_user_verbs.h>

static struct ib_device *ibd;

static void ib_add_one(struct ib_device *device)
{
	ibd = device;
	pr_info("%s(): ibd=%p(%s) device=%p(%s)\n",
		__func__, ibd, ibd ? ibd->name : " ", device, device->name);
}

static void ib_remove_one(struct ib_device *device)
{
	return;
}

static struct ib_client test_ib_client = {
	.name   = "test_ib_client",
	.add    = ib_add_one,
	.remove = ib_remove_one,
};

static void test_dma(void *kva, size_t size)
{
	void *dma_addr, *phys_addr;

	dma_addr = (void *)ib_dma_map_single(ibd, kva, size, DMA_BIDIRECTIONAL);
	phys_addr = (void *)virt_to_phys(kva);

	pr_info("kva: %18p dma: %18p phy: %18p\n", kva, dma_addr, phys_addr);
}

int a;

static int test_ib_init(void)
{
	int ret;
	void *buf;

	ret = ib_register_client(&test_ib_client);
	if (ret) {
		pr_err("fail to register client\n");
		return ret;
	}

	buf = kmalloc(32, GFP_KERNEL);
	test_dma(buf, 32);
	test_dma(&ret, 4);
	test_dma(&a, 4);

	kfree(buf);
	return 0;
}

static void test_ib_exit(void)
{
	ib_unregister_client(&test_ib_client);
}

module_init(test_ib_init);
module_exit(test_ib_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yizhou Shan ys@purdue.edu");
