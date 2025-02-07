#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x949f7342, "__alloc_percpu" },
	{ 0xb19a5453, "__per_cpu_offset" },
	{ 0x17de3d5, "nr_cpu_ids" },
	{ 0x9e683f75, "__cpu_possible_mask" },
	{ 0x53a1e8d9, "_find_next_bit" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0xaf5a0bb8, "pcpu_hot" },
	{ 0x53569707, "this_cpu_off" },
	{ 0xa5526619, "rb_insert_color" },
	{ 0x48d88a2c, "__SCT__preempt_schedule" },
	{ 0xa81a8d5a, "kmalloc_caches" },
	{ 0x820c563, "kmalloc_trace" },
	{ 0xa19b956, "__stack_chk_fail" },
	{ 0xbafbbb6a, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FA08F9764B8AC064A0ADA0A");
