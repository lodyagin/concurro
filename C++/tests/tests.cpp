#include <stdio.h>
#include <string.h>
#include "Basic.h"
#include <map>
#include <string>

extern CU_TestInfo RMutexTests[];
int RMutexCUInit(void);
int RMutexCUClean(void);

CU_SuiteInfo suites[] = {	
  { "RMutex", RMutexCUInit, RMutexCUClean, 0, 0,
	  RMutexTests },
	CU_SUITE_INFO_NULL
};

using namespace std;

int main(int argc, char** argv, char** env)
{
	map<string, CU_pSuiteInfo> tests;
	
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	if(argc <= 1){
		if(CUE_SUCCESS != CU_register_suites(suites))
			return CU_get_error();
	} else{

		int i = 0;
		while(suites[i].pName){
			tests.insert(pair<string,CU_SuiteInfo*>(suites[i].pName, &suites[i]));
			i++;
		}
		
		for(int i = 1; i < argc; i++){
			if(tests.count(argv[i])){
				CU_pSuiteInfo pSuitInfo = tests[argv[i]];
				CU_pSuite pSuite = CU_add_suite(pSuitInfo->pName, pSuitInfo->pInitFunc, pSuitInfo->pCleanupFunc);
				if (NULL == pSuite) {
					printf("ERROR!!!");
					CU_cleanup_registry();
					return CU_get_error();
				}
				
				int t = 0;
				while(pSuitInfo->pTests[t].pName){
					//printf(" suite %s not found\n");
					CU_add_test(pSuite, pSuitInfo->pTests[t].pName, pSuitInfo->pTests[t].pTestFunc);
					t++;
				}
			} else{
				printf("ERROR: test suite %s not found\n", argv[i]);
				return CU_get_error();
			}
		}
		
		
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	
	printf("\n");
	CU_basic_show_failures(CU_get_failure_list());
	printf("\n\n");

	CU_cleanup_registry();
 	return CU_get_error();
}