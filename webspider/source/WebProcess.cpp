#include "cstringext.h"
#include "sys/path.h"
#include "sysprocess.h"
#include "WebProcess.h"
#include <string>

int web_process_create(const char* appname, const char* arg1, const char* arg2, process_t *pid)
{
	process_create_param_t param;
	memset(&param, 0, sizeof(param));

#if defined(OS_WINDOWS)
	char cmdLine[MAX_PATH] = {0};
	snprintf(cmdLine, sizeof(cmdLine)-1, "%s %s %s", appname, arg1, arg2?arg2:"");
	param.lpCommandLine = cmdLine;

	char workDir[MAX_PATH] = {0};
	path_getcwd(workDir, sizeof(workDir));
	param.lpCurrentDirectory = workDir;

#if defined(DEBUG) || defined(_DEBUG)
	param.dwCreationFlags |= CREATE_NEW_CONSOLE;
#endif
#if defined(_UNICODE) || defined(UNICODE)
	param.dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

#else
	char* argv[] = { (char*)appname, (char*)arg1, (char*)arg2, NULL };
	param.argv = argv;
	param.envp = envp;
#endif

	return process_createve(appname, &param, pid);
}

struct Result
{
	process_t  *pid;
	const char *name;
	const char *module;
	int result;
};

static void OnListModule(void* param, const char* name)
{
	Result* result = (Result*)param;
	if(streq(path_basename(result->module), path_basename(name)))
	{
		result->result = 0; // find it
	}
}

static int OnListProcess(void* param, const char* name, process_t pid)
{
	Result* result = (Result*)param;
	if(streq(path_basename(name), path_basename(result->name)))
	{
		process_getmodules(pid, OnListModule, param);
		if(0 == result->result)
		{
			*result->pid = pid;
			return 1;
		}
	}
	return 0;
}

int web_process_find(const char* appname, const char* module, process_t *pid)
{
	Result result;
	result.name = appname;
	result.module = module;
	result.result = -1;
	result.pid = pid;

	process_list(OnListProcess, &result);
	return result.result;
}
