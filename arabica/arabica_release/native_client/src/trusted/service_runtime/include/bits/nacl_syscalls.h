/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NaCl kernel / service run-time system call numbers
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_NACL_SYSCALLS_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_NACL_SYSCALLS_H_

/* intentionally not using zero */

/*
 * TODO(bsy,sehr): these identifiers should be NACL_ABI_SYS_<name>.
 */

#define NACL_sys_null                    1

#define NACL_sys_open                   10
#define NACL_sys_close                  11
#define NACL_sys_read                   12
#define NACL_sys_write                  13
#define NACL_sys_lseek                  14
#define NACL_sys_ioctl                  15
#define NACL_sys_stat                   16
#define NACL_sys_fstat                  17
#define NACL_sys_chmod                  18
/* no fchmod emulation on windows */

#define NACL_sys_sysbrk                 20
#define NACL_sys_mmap                   21
#define NACL_sys_munmap                 22

#define NACL_sys_getdents               23

#define NACL_sys_exit                   30
#define NACL_sys_getpid                 31
#define NACL_sys_sched_yield            32
#define NACL_sys_sysconf                33

#define NACL_sys_gettimeofday           40
#define NACL_sys_clock                  41

#define NACL_sys_multimedia_init        50
#define NACL_sys_multimedia_shutdown    51
#define NACL_sys_video_init             52
#define NACL_sys_video_shutdown         53
#define NACL_sys_video_update           54
#define NACL_sys_video_poll_event       55
#define NACL_sys_audio_init             56
#define NACL_sys_audio_shutdown         57
#define NACL_sys_audio_stream           58

#define NACL_sys_imc_makeboundsock      60
#define NACL_sys_imc_accept             61
#define NACL_sys_imc_connect            62
#define NACL_sys_imc_sendmsg            63
#define NACL_sys_imc_recvmsg            64
#define NACL_sys_imc_mem_obj_create     65
#define NACL_sys_imc_socketpair         66

#define NACL_sys_mutex_create           70
#define NACL_sys_mutex_lock             71
#define NACL_sys_mutex_trylock          72
#define NACL_sys_mutex_unlock           73
#define NACL_sys_cond_create            74
#define NACL_sys_cond_wait              75
#define NACL_sys_cond_signal            76
#define NACL_sys_cond_broadcast         77
#define NACL_sys_cond_timed_wait_abs    79

#define NACL_sys_thread_create          80
#define NACL_sys_thread_exit            81
#define NACL_sys_tls_init               82

#define NACL_sys_srpc_get_fd            90

#define NACL_sys_sem_create             100
#define NACL_sys_sem_wait               101
#define NACL_sys_sem_post               102
#define NACL_sys_sem_get_value          103

//jjs204
#define NACL_sys_initdynlinkwrappers	113
#define NACL_sys_outofjail		114
#define NACL_sys_jniescape		115

//JNI Wrapper functions
//mes310 add to include all 229 JNI functions

#define NACL_sys_jni_call_getversion                    116
#define NACL_sys_jni_call_defineclass                   117
#define NACL_sys_jni_call_findclass                     118
#define NACL_sys_jni_call_fromreflectedmethod           119
#define NACL_sys_jni_call_fromreflectedfield            120
#define NACL_sys_jni_call_toreflectedmethod             121
#define NACL_sys_jni_call_getsuperclass                 122
#define NACL_sys_jni_call_isassignablefrom              123
#define NACL_sys_jni_call_toreflectedfield              124
#define NACL_sys_jni_call_throw                         125
#define NACL_sys_jni_call_thrownew                      126
#define NACL_sys_jni_call_exceptionoccurred             127
#define NACL_sys_jni_call_exceptiondescribe             128
#define NACL_sys_jni_call_exceptionclear                129
#define NACL_sys_jni_call_fatalerror                    130
#define NACL_sys_jni_call_pushlocalframe                131
#define NACL_sys_jni_call_poplocalframe                 132
#define NACL_sys_jni_call_newglobalref                  133
#define NACL_sys_jni_call_deleteglobalref               134
#define NACL_sys_jni_call_deletelocalref                135
#define NACL_sys_jni_call_issameobject                  136
#define NACL_sys_jni_call_newlocalref                   137
#define NACL_sys_jni_call_ensurelocalcapacity           138
#define NACL_sys_jni_call_allocobject                   139
#define NACL_sys_jni_call_newobject                     140
#define NACL_sys_jni_call_newobjectv                    141
#define NACL_sys_jni_call_newobjecta                    142
#define NACL_sys_jni_call_getobjectclass                143
#define NACL_sys_jni_call_isinstanceof                  144
#define NACL_sys_jni_call_getmethodid                   145
#define NACL_sys_jni_call_callobjectmethod              146
#define NACL_sys_jni_call_callobjectmethodv             147
#define NACL_sys_jni_call_callobjectmethoda             148
#define NACL_sys_jni_call_callbooleanmethod             149
#define NACL_sys_jni_call_callbooleanmethodv            150
#define NACL_sys_jni_call_callbooleanmethoda            151
#define NACL_sys_jni_call_callbytemethod                152
#define NACL_sys_jni_call_callbytemethodv               153
#define NACL_sys_jni_call_callbytemethoda               154
#define NACL_sys_jni_call_callcharmethod                155
#define NACL_sys_jni_call_callcharmethodv               156
#define NACL_sys_jni_call_callcharmethoda               157
#define NACL_sys_jni_call_callshortmethod               158
#define NACL_sys_jni_call_callshortmethodv              159
#define NACL_sys_jni_call_callshortmethoda              160
#define NACL_sys_jni_call_callintmethod                 161
#define NACL_sys_jni_call_callintmethodv                162
#define NACL_sys_jni_call_callintmethoda                163
#define NACL_sys_jni_call_calllongmethod                164
#define NACL_sys_jni_call_calllongmethodv               165
#define NACL_sys_jni_call_calllongmethoda               166
#define NACL_sys_jni_call_callfloatmethod               167
#define NACL_sys_jni_call_callfloatmethodv              168
#define NACL_sys_jni_call_callfloatmethoda              169
#define NACL_sys_jni_call_calldoublemethod              170
#define NACL_sys_jni_call_calldoublemethodv             171
#define NACL_sys_jni_call_calldoublemethoda             172
#define NACL_sys_jni_call_callvoidmethod                173
#define NACL_sys_jni_call_callvoidmethodv               174
#define NACL_sys_jni_call_callvoidmethoda               175
#define NACL_sys_jni_call_callnonvirtualobjectmethod    176
#define NACL_sys_jni_call_callnonvirtualobjectmethodv   177
#define NACL_sys_jni_call_callnonvirtualobjectmethoda   178
#define NACL_sys_jni_call_callnonvirtualbooleanmethod   179
#define NACL_sys_jni_call_callnonvirtualbooleanmethodv  180
#define NACL_sys_jni_call_callnonvirtualbooleanmethoda  181
#define NACL_sys_jni_call_callnonvirtualbytemethod      182
#define NACL_sys_jni_call_callnonvirtualbytemethodv     183
#define NACL_sys_jni_call_callnonvirtualbytemethoda     184
#define NACL_sys_jni_call_callnonvirtualcharmethod      185
#define NACL_sys_jni_call_callnonvirtualcharmethodv     186
#define NACL_sys_jni_call_callnonvirtualcharmethoda     187
#define NACL_sys_jni_call_callnonvirtualshortmethod     188
#define NACL_sys_jni_call_callnonvirtualshortmethodv    189
#define NACL_sys_jni_call_callnonvirtualshortmethoda    190
#define NACL_sys_jni_call_callnonvirtualintmethod       191
#define NACL_sys_jni_call_callnonvirtualintmethodv      192
#define NACL_sys_jni_call_callnonvirtualintmethoda      193
#define NACL_sys_jni_call_callnonvirtuallongmethod      194
#define NACL_sys_jni_call_callnonvirtuallongmethodv     195
#define NACL_sys_jni_call_callnonvirtuallongmethoda     196
#define NACL_sys_jni_call_callnonvirtualfloatmethod     197
#define NACL_sys_jni_call_callnonvirtualfloatmethodv    198
#define NACL_sys_jni_call_callnonvirtualfloatmethoda    199
#define NACL_sys_jni_call_callnonvirtualdoublemethod    200
#define NACL_sys_jni_call_callnonvirtualdoublemethodv   201
#define NACL_sys_jni_call_callnonvirtualdoublemethoda   202
#define NACL_sys_jni_call_callnonvirtualvoidmethod      203
#define NACL_sys_jni_call_callnonvirtualvoidmethodv     204
#define NACL_sys_jni_call_callnonvirtualvoidmethoda     205
#define NACL_sys_jni_call_getfieldid                    206
#define NACL_sys_jni_call_getobjectfield                207
#define NACL_sys_jni_call_getbooleanfield               208
#define NACL_sys_jni_call_getbytefield                  209
#define NACL_sys_jni_call_getcharfield                  210
#define NACL_sys_jni_call_getshortfield                 211
#define NACL_sys_jni_call_getintfield                   212
#define NACL_sys_jni_call_getlongfield                  213
#define NACL_sys_jni_call_getfloatfield                 214
#define NACL_sys_jni_call_getdoublefield                215
#define NACL_sys_jni_call_setobjectfield                216
#define NACL_sys_jni_call_setbooleanfield               217
#define NACL_sys_jni_call_setbytefield                  218
#define NACL_sys_jni_call_setcharfield                  219
#define NACL_sys_jni_call_setshortfield                 220
#define NACL_sys_jni_call_setintfield                   221
#define NACL_sys_jni_call_setlongfield                  222
#define NACL_sys_jni_call_setfloatfield                 223
#define NACL_sys_jni_call_setdoublefield                224
#define NACL_sys_jni_call_getstaticmethodid             225
#define NACL_sys_jni_call_callstaticobjectmethod        226
#define NACL_sys_jni_call_callstaticobjectmethodv       227
#define NACL_sys_jni_call_callstaticobjectmethoda       228
#define NACL_sys_jni_call_callstaticbooleanmethod       229
#define NACL_sys_jni_call_callstaticbooleanmethodv      230
#define NACL_sys_jni_call_callstaticbooleanmethoda      231
#define NACL_sys_jni_call_callstaticbytemethod          232
#define NACL_sys_jni_call_callstaticbytemethodv         233
#define NACL_sys_jni_call_callstaticbytemethoda         234
#define NACL_sys_jni_call_callstaticcharmethod          235
#define NACL_sys_jni_call_callstaticcharmethodv         236
#define NACL_sys_jni_call_callstaticcharmethoda         237
#define NACL_sys_jni_call_callstaticshortmethod         238
#define NACL_sys_jni_call_callstaticshortmethodv        239
#define NACL_sys_jni_call_callstaticshortmethoda        240
#define NACL_sys_jni_call_callstaticintmethod           241
#define NACL_sys_jni_call_callstaticintmethodv          242
#define NACL_sys_jni_call_callstaticintmethoda          243
#define NACL_sys_jni_call_callstaticlongmethod          244
#define NACL_sys_jni_call_callstaticlongmethodv         245
#define NACL_sys_jni_call_callstaticlongmethoda         246
#define NACL_sys_jni_call_callstaticfloatmethod         247
#define NACL_sys_jni_call_callstaticfloatmethodv        248
#define NACL_sys_jni_call_callstaticfloatmethoda        249
#define NACL_sys_jni_call_callstaticdoublemethod        250
#define NACL_sys_jni_call_callstaticdoublemethodv       251
#define NACL_sys_jni_call_callstaticdoublemethoda       252
#define NACL_sys_jni_call_callstaticvoidmethod          253
#define NACL_sys_jni_call_callstaticvoidmethodv         254
#define NACL_sys_jni_call_callstaticvoidmethoda         255
#define NACL_sys_jni_call_getstaticfieldid              256
#define NACL_sys_jni_call_getstaticobjectfield          257
#define NACL_sys_jni_call_getstaticbooleanfield         258
#define NACL_sys_jni_call_getstaticbytefield            259
#define NACL_sys_jni_call_getstaticcharfield            260
#define NACL_sys_jni_call_getstaticshortfield           261
#define NACL_sys_jni_call_getstaticintfield             262
#define NACL_sys_jni_call_getstaticlongfield            263
#define NACL_sys_jni_call_getstaticfloatfield           264
#define NACL_sys_jni_call_getstaticdoublefield          265
#define NACL_sys_jni_call_setstaticobjectfield          266
#define NACL_sys_jni_call_setstaticbooleanfield         267
#define NACL_sys_jni_call_setstaticbytefield            268
#define NACL_sys_jni_call_setstaticcharfield            269
#define NACL_sys_jni_call_setstaticshortfield           270
#define NACL_sys_jni_call_setstaticintfield             271
#define NACL_sys_jni_call_setstaticlongfield            272
#define NACL_sys_jni_call_setstaticfloatfield           273
#define NACL_sys_jni_call_setstaticdoublefield          274
#define NACL_sys_jni_call_newstring                     275
#define NACL_sys_jni_call_getstringlength               276
#define NACL_sys_jni_call_getstringchars                277
#define NACL_sys_jni_call_releasestringchars            278
#define NACL_sys_jni_call_newstringutf                  279
#define NACL_sys_jni_call_getstringutflength            280
#define NACL_sys_jni_call_getstringutfchars             281
#define NACL_sys_jni_call_releasestringutfchars         282
#define NACL_sys_jni_call_getarraylength                283
#define NACL_sys_jni_call_newobjectarray                284
#define NACL_sys_jni_call_getobjectarrayelement         285
#define NACL_sys_jni_call_setobjectarrayelement         286
#define NACL_sys_jni_call_newbooleanarray               287
#define NACL_sys_jni_call_newbytearray                  288
#define NACL_sys_jni_call_newchararray                  289
#define NACL_sys_jni_call_newshortarray                 290
#define NACL_sys_jni_call_newintarray                   291
#define NACL_sys_jni_call_newlongarray                  292
#define NACL_sys_jni_call_newfloatarray                 293
#define NACL_sys_jni_call_newdoublearray                294
#define NACL_sys_jni_call_getbooleanarrayelements       295
#define NACL_sys_jni_call_getbytearrayelements          296
#define NACL_sys_jni_call_getchararrayelements          297
#define NACL_sys_jni_call_getshortarrayelements         298
#define NACL_sys_jni_call_getintarrayelements           299
#define NACL_sys_jni_call_getlongarrayelements          300
#define NACL_sys_jni_call_getfloatarrayelements         301
#define NACL_sys_jni_call_getdoublearrayelements        302
#define NACL_sys_jni_call_releasebooleanarrayelements   303
#define NACL_sys_jni_call_releasebytearrayelements      304
#define NACL_sys_jni_call_releasechararrayelements      305
#define NACL_sys_jni_call_releaseshortarrayelements     306
#define NACL_sys_jni_call_releaseintarrayelements       307
#define NACL_sys_jni_call_releaselongarrayelements      308
#define NACL_sys_jni_call_releasefloatarrayelements     309
#define NACL_sys_jni_call_releasedoublearrayelements    310
#define NACL_sys_jni_call_getbooleanarrayregion         311
#define NACL_sys_jni_call_getbytearrayregion            312
#define NACL_sys_jni_call_getchararrayregion            313
#define NACL_sys_jni_call_getshortarrayregion           314
#define NACL_sys_jni_call_getintarrayregion             315
#define NACL_sys_jni_call_getlongarrayregion            316
#define NACL_sys_jni_call_getfloatarrayregion           317
#define NACL_sys_jni_call_getdoublearrayregion          318
#define NACL_sys_jni_call_setbooleanarrayregion         319
#define NACL_sys_jni_call_setbytearrayregion            320
#define NACL_sys_jni_call_setchararrayregion            321
#define NACL_sys_jni_call_setshortarrayregion           322
#define NACL_sys_jni_call_setintarrayregion             323
#define NACL_sys_jni_call_setlongarrayregion            324
#define NACL_sys_jni_call_setfloatarrayregion           325
#define NACL_sys_jni_call_setdoublearrayregion          326
#define NACL_sys_jni_call_registernatives               327
#define NACL_sys_jni_call_unregisternatives             328
#define NACL_sys_jni_call_monitorenter                  329
#define NACL_sys_jni_call_monitorexit                   330
#define NACL_sys_jni_call_getjavavm                     331
#define NACL_sys_jni_call_getstringregion               332
#define NACL_sys_jni_call_getstringutfregion            333
#define NACL_sys_jni_call_getprimitivearraycritical     334
#define NACL_sys_jni_call_releaseprimitivearraycritical 335
#define NACL_sys_jni_call_getstringcritical             336
#define NACL_sys_jni_call_releasestringcritical         337
#define NACL_sys_jni_call_newweakglobalref              338
#define NACL_sys_jni_call_deleteweakglobalref           339
#define NACL_sys_jni_call_exceptioncheck                340
#define NACL_sys_jni_call_newdirectbytebuffer           341
#define NACL_sys_jni_call_getdirectbufferaddress        342
#define NACL_sys_jni_call_getdirectbuffercapacity       343
#define NACL_sys_jni_call_getobjectreftype              344

// include one more for (*vm)->GetEnv
#define NACL_sys_vm_call_getenv				345

#define NACL_sys_robustasleep				346

#define NACL_sys_getglobaljnienv			347

//jjs204
//used to be 110
//mes310
//used to be 170 in Robusta 1.0

#define NACL_MAX_SYSCALLS				350

#endif /* NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_NACL_SYSCALLS_H_ */
