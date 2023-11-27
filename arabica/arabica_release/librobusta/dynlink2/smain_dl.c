/*
 * Copyright 2013, SoS Laboratory, Lehigh University
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
 *     * Neither the name of SoS Laboratory, Lehigh University nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
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

#include "native_client/src/include/portability.h"
#include "native_client/src/include/portability_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

#include "native_client/src/trusted/platform/nacl_log.h"
#include "native_client/src/trusted/platform/nacl_sync.h"
#include "native_client/src/trusted/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/platform_qualify/nacl_os_qualify.h"

#include "native_client/src/trusted/service_runtime/env_cleanser.h"
#include "native_client/src/trusted/service_runtime/expiration.h"
#include "native_client/src/trusted/service_runtime/gio.h"
#include "native_client/src/trusted/service_runtime/nacl_app.h"
#include "native_client/src/trusted/service_runtime/nacl_all_modules.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"

#include "sel_ldr_dl.h"

int main()
{
	char *nacl_file = 0;
	int log_desc = -1;

	NaClAllModulesInit()

	//debug mode for now...
	NaClInsecurelyBypassAllAclChecks();
        NaClIgnoreValidatorResult();

	//hard code nacl file for now...
	nacl_file="";

	if (0 == GioMemoryFileSnapshotCtor(&gf, nacl_file)) 
	{
		perror("sel_main");
   		fprintf(stderr, "Cannot open \"%s\".\n", nacl_file);
    		return 1;
	}

	if (!NaClAppCtor(&state)) 
	{
		fprintf(stderr, "Error while constructing app state\n");
		goto done_file_dtor;
	}

	nap = &state;
	errcode = LOAD_OK;

	if (LOAD_OK == errcode) 
	{
		errcode = NaClAppLoadDynFile((struct Gio *) &gf, nap);
		if (LOAD_OK != errcode) 
		{
			nap->module_load_status = errcode;
			fprintf(stderr, "Error while loading \"%s\": %s\n",
				nacl_file,
				NaClErrorString(errcode));
		}
	}

	if (LOAD_OK == errcode) 
	{
		//always print to stdout & stderror instead of a log file
		//(for now)
		errcode = NaClAppPrepareToLaunch(nap,0,1,2);
	}

	NaClLog(1, "Jumping into the sandbox...\n");

	fflush((FILE *) NULL);

	NaClXMutexLock(&nap->mu);
	nap->module_load_status = LOAD_OK;
	NaClXCondVarBroadcast(&nap->cv);
	NaClXMutexUnlock(&nap->mu);

	//dl stuff
	int argc;
	char **argv;
	char *gv[2];
	char *p;

	argc = 2;
	gv[0] = "NaClMain";
	gv[1] = "tests/dynlink";
	argv = gv;

	if (!NaClCreateMainThread(nap,
                            argc,
                            argv,
                            NaClEnvCleanserEnvironment(&filtered_env))) 
	{
		fprintf(stderr, "creating main thread failed\n");
		NaClEnvCleanserDtor(&filtered_env);
		goto done;
	}

	ret_code = NaClWaitForMainThreadToExit(nap);

	_exit(ret_code);

	 done:
	fflush(stdout);

	if (verbosity) 
	{
		gprintf((struct Gio *) &gout, "exiting -- printing NaClApp details\n");
		NaClAppPrintDetails(nap, (struct Gio *) &gout);

		printf("Dumping vmmap.\n"); fflush(stdout);
		PrintVmmap(nap);
		fflush(stdout);

		printf("appdtor\n");
		fflush(stdout);
	}

	NaClAppDtor(&state);

	done_file_dtor:
	if ((*((struct Gio *) &gf)->vtbl->Close)((struct Gio *) &gf) == -1) 
	{
		fprintf(stderr, "Error while closing \"%s\".\n", av[optind]);
	}

	(*((struct Gio *) &gf)->vtbl->Dtor)((struct Gio *) &gf);

	if (verbosity > 0)
	{
		printf("Done.\n");
	}
	fflush(stdout);

	NaClAllModulesFini();

	WINDOWS_EXCEPTION_CATCH;

	_exit(ret_code);
}
