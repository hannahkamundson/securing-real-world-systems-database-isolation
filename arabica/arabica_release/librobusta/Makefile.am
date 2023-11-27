trunk=../
lib_LTLIBRARIES = librobusta.la
librobusta_la_SOURCES =  robusta_watcher.c robusta_jvm.c robusta_jvmti.c robusta_nacl.c JavaTypeParser.c sel_ldr_dl.c robusta_jni_checks.c stub_policy.c jinn/agent.c jinn/agent_class.c jinn/c2j_proxy.c jinn/common.c jinn/hashtable.c jinn/hashtable_itr.c jinn/j2c_proxy.c jinn/java_method.c jinn/jnicheck.c jinn/options.c jinn/state.c
librobusta_la_LIBADD = \
	-lsel -lnacl_xdr -lnonnacl_srpc -lnrd_xfer -lnonnacl_util_c \
	-lncvalidate -lgoogle_nacl_imc_c -lcontainer -lplatform_qual_lib \
	-lplatform -lgio -lexpiration -lrt -lpthread -lssl -lcrypto \
	-lSDL -lSDLmain -llist -lstdc++ -lm -ljava
librobusta_la_LDFLAGS = \
	-m32 -L/usr/lib32 -L/usr/lib \
	-L$(trunk)/native_client/scons-out/opt-linux-x86-32/lib \
	-L$(trunk)/native_client/src/third_party/sdl/linux/v1_2_13/lib \
	-L$(trunk)/thirdparty/linkedlist \
	-L$(trunk)/../JVM/openjdk-7/build/linux-i586/lib/i386 \
	-Wl,-rpath=$(trunk)/native_client/scons-out/opt-linux-x86-32/lib \
	-Wl,--allow-shlib-undefined
librobusta_la_CPPFLAGS = \
	-Wall \
	-DNACL_WINDOWS=0 -DNACL_OSX=0 -DNACL_LINUX=1 -D_BSD_SOURCE=1 \
	-D_POSIX_C_SOURCE=199506 -D_XOPEN_SOURCE=600 -D_GNU_SOURCE=1 \
	-D__STDC_LIMIT_MACROS=1 -DNACL_BLOCK_SHIFT=5 -DNACL_BLOCK_SIZE=32 \
	-DNACL_BUILD_ARCH=x86 -DNACL_BUILD_SUBARC=32 -DNACL_TARGET_ARCH=x86 \
	-DNACL_TARGET_SUBARCH=32 -DOS_LINUX=OS_LINUX -DNDEBUG \
	-I$(trunk) \
	-I$(trunk)/include \
	-I$(trunk)/native_client/src/third_party/gtest/files/include \
	-I$(trunk)/native_client/src/third_party/sdl/linux/v_1_2_13/include \
	-I/usr/local/jdk1.6.0_17/include \
	-I/usr/local/jdk1.6.0_17/include/linux
