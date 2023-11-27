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
 * NaCl service library.  a primitive rpc library
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifndef __native_client__
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/desc/nacl_desc_imc.h"
#endif  /* __native_client__ */
#include "nacl_srpc.h"
#include "nacl_srpc_internal.h"


/*
 * A utility function for copying strings needed for ServerCtor
 */
static char* CopyStringLength(const char* string, size_t length) {
  /* TODO(sehr): use strndup. */
  char* copy = (char*) malloc(length + 1);
  if (NULL == copy) {
    return NULL;
  }
  strncpy(copy, string, length);
  copy[length] = '\0';
  return copy;
}

/*
 * Service discovery is used to build an interface description that
 * is searched for rpc dispatches.
 */
static int NaClSrpcBuildInterfaceDesc(NaClSrpcChannel  *channel) {
  int errcode;
  NaClSrpcArg*  ins[] = { NULL };
  NaClSrpcArg   out_carray;
  NaClSrpcArg*  outs[2];

  /*
   * We initialize the service descriptors to have service discovery
   * and other default services (as they are added).
   */
  NaClSrpcDesc  basic_services[] = {
    { "service_discovery", "", "C", NULL },
  };

  outs[0] = &out_carray;
  outs[1] = NULL;

  channel->rpc_descr = basic_services;
  channel->rpc_count = sizeof(basic_services) / sizeof(basic_services[0]);

  /* Build the argument value for invoking service discovery */
  out_carray.tag = NACL_SRPC_ARG_TYPE_CHAR_ARRAY;
  out_carray.u.caval.count = NACL_SRPC_MAX_SERVICE_DISCOVERY_CHARS;
  out_carray.u.caval.carr = calloc(NACL_SRPC_MAX_SERVICE_DISCOVERY_CHARS, 1);
  if (NULL == out_carray.u.caval.carr) {
    fprintf(stderr, "service_discovery could not allocate memory\n");
    return 0;
  }
  /* Invoke service discovery, getting description string */
  errcode = NaClSrpcInvokeV(channel, 0, ins, outs);
  if (NACL_SRPC_RESULT_OK != errcode) {
    fprintf(stderr, "service_discovery call failed(%d): %s\n", errcode,
            NaClSrpcErrorString(errcode));
    return 0;
  }
  /* Build the real rpc description from the resulting string. */
  channel->rpc_descr = __NaClSrpcBuildSrpcDesc(outs[0]->u.caval.carr,
                                               &channel->rpc_count);
  /* Free the service string */
  free(out_carray.u.caval.carr);
  /* Return success */
  return 1;
}

/*
 * The constructors and destructor.
 */

/*
 * Set up the buffering structures for a channel.
 */
int NaClSrpcClientCtor(NaClSrpcChannel* channel, NaClSrpcImcDescType handle) {
  channel->imc_handle = handle;

#ifndef __native_client__
  if (channel->imc_handle == NULL) {
    return 0;
  }
  if (!NaClNrdXferEffectorCtor(&channel->eff, channel->imc_handle)) {
    return 0;
  }
#endif
  /* Construct the buffers. */
  __NaClSrpcImcBufferCtor(&channel->send_buf, 1);
  __NaClSrpcImcBufferCtor(&channel->receive_buf, 0);
  /* Disable timing and initialize the timing counters. */
  channel->timing_enabled = 0;
  channel->send_usec = 0.0;
  channel->receive_usec = 0.0;
  channel->imc_read_usec = 0.0;
  channel->imc_write_usec = 0.0;
  /* Do service discovery to speed method invocation. */
  if (NaClSrpcBuildInterfaceDesc(channel) == 0) {
    return 0;
  }
  /* Return success. */
  return 1;
}

int NaClSrpcServerCtor(NaClSrpcChannel* channel,
                       NaClSrpcImcDescType handle,
                       const NaClSrpcHandlerDesc* handlers,
                       void* server_instance_data) {
  uint32_t handler_count;
  uint32_t i;

  channel->imc_handle = handle;
#ifndef __native_client__
  if (channel->imc_handle == NULL) {
    return 0;
  }
  if (!NaClNrdXferEffectorCtor(&channel->eff, channel->imc_handle)) {
    return 0;
  }
#endif
  /* Construct the buffers. */
  __NaClSrpcImcBufferCtor(&channel->send_buf, 1);
  __NaClSrpcImcBufferCtor(&channel->receive_buf, 0);
  /* Count the number of methods. */
  handler_count = 0;
  while (NULL != handlers[handler_count].entry_fmt)
    ++handler_count;
  /*
   * This is a server connection, build the descriptors by parsing the
   * handler descriptors passed in.
   */
  channel->rpc_descr =
      (NaClSrpcDesc*) malloc(handler_count * sizeof(*channel->rpc_descr));
  if (NULL == channel->rpc_descr) {
    return 0;
  }
  channel->rpc_count = handler_count;
  for (i = 0; i < handler_count; ++i) {
    const char* p;
    const char* nextp;

    /* entry_fmt should look like "name:inargs:outargs" */
    p = handlers[i].entry_fmt;
    /* Get name. */
    nextp = strchr(p, ':');
    if (p == NULL) {
      return 0;
    }
    channel->rpc_descr[i].rpc_name = CopyStringLength(p, nextp - p);
    p = nextp + 1;
    /* Get inargs. */
    nextp = strchr(p, ':');
    if (p == NULL) {
      return 0;
    }
    channel->rpc_descr[i].in_args = CopyStringLength(p, nextp - p);
    p = nextp + 1;
    /* Get outargs. */
    nextp = strchr(p, '\0');
    if (p == NULL) {
      return 0;
    }
    channel->rpc_descr[i].out_args = CopyStringLength(p, nextp - p);
    /* Add the handler pointer to the descriptor.  */
    channel->rpc_descr[i].handler = handlers[i].handler;
  }
  /* Disable timing and initialize the timing counters. */
  channel->timing_enabled = 0;
  channel->send_usec = 0.0;
  channel->receive_usec = 0.0;
  channel->imc_read_usec = 0.0;
  channel->imc_write_usec = 0.0;
  channel->server_instance_data = server_instance_data;
  /* Return success. */
  return 1;
}

void NaClSrpcDtor(NaClSrpcChannel *channel) {
#ifndef __native_client__
  struct NaClDescEffector* effp = (struct NaClDescEffector*) &channel->eff;
  effp->vtbl->Dtor(effp);
  NaClDescUnref(channel->imc_handle);
#endif
  if (channel->rpc_descr)
    free(channel->rpc_descr);
}
