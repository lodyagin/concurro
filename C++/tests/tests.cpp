#include <stdio.h>
#include <string.h>
#include "Basic.h"
#include <map>
#include <string>

extern CU_TestInfo ClassWithStatesTests[];
extern CU_TestInfo ExistentTests[];
extern CU_TestInfo RepositoryTests[];
extern CU_TestInfo EventTests[];
extern CU_TestInfo RStateTests[];
extern CU_TestInfo RThreadTests[];
extern CU_TestInfo RMutexTests[];
extern CU_TestInfo REventTests[];
extern CU_TestInfo RHolderTests[];
extern CU_TestInfo RBufferTests[];
extern CU_TestInfo RWindowTests[];
extern CU_TestInfo RSignalTests[];
extern CU_TestInfo RSocketTests[];
extern CU_TestInfo RConnectionTests[];
extern CU_TestInfo SCommonTests[];
extern CU_TestInfo SSingletonTests[];

int ClassWithStatesCUInit(void);
int ClassWithStatesCUClean(void);
int ExistentCUInit(void);
int ExistentCUClean(void);
int RepositoryCUInit(void);
int RepositoryCUClean(void);
int EventCUInit(void);
int EventCUClean(void);
int RStateCUInit(void);
int RStateCUClean(void);
int RThreadCUInit(void);
int RThreadCUClean(void);
int RMutexCUInit(void);
int RMutexCUClean(void);
int REventCUInit(void);
int REventCUClean(void);
int RBufferCUInit(void);
int RBufferCUClean(void);
int RWindowCUInit(void);
int RWindowCUClean(void);
int RSignalCUInit(void);
int RSignalCUClean(void);
int RSocketCUInit(void);
int RSocketCUClean(void);
int RConnectionCUInit(void);
int RConnectionCUClean(void);
int SCommonCUInit(void);
int SCommonCUClean(void);
int SSingletonCUInit(void);
int SSingletonCUClean(void);
int RHolderCUInit(void);
int RHolderCUClean(void);

CU_SuiteInfo suites[] = {	
  { "Event", EventCUInit, EventCUClean, 0, 0,
	  EventTests },
  { "RState", RStateCUInit, RStateCUClean, 0, 0,
	  RStateTests },
  { "REvent", REventCUInit, REventCUClean, 0, 0,
	  REventTests },
#if 0
  { "ClassWithStates", ClassWithStatesCUInit, 
    ClassWithStatesCUClean, 0, 0, 
          ClassWithStatesTests},
#endif
  { "Existent", ExistentCUInit, ExistentCUClean, 0, 0,
	  ExistentTests },
  { "Repository", RepositoryCUInit, RepositoryCUClean, 0, 0,
	  RepositoryTests },
  { "Common", SCommonCUInit, SCommonCUClean, 0, 0,
      SCommonTests },
  { "SSingleton", SSingletonCUInit, SSingletonCUClean, 0, 0,
	  SSingletonTests },
  { "RHolder", RHolderCUInit, RHolderCUClean, 0, 0,
	  RHolderTests },
  { "RThread", RThreadCUInit, RThreadCUClean, 0, 0,
	  RThreadTests },
  { "RMutex", RMutexCUInit, RMutexCUClean, 0, 0,
	  RMutexTests },
  { "RBuffer", RBufferCUInit, RBufferCUClean, 0, 0,
	  RBufferTests },
  { "RWindow", RWindowCUInit, RWindowCUClean, 0, 0,
	  RWindowTests },
//  { "RSignal", RSignalCUInit, RSignalCUClean, 0, 0,
//	  RSignalTests },
  { "RSocket", RSocketCUInit, RSocketCUClean, 0, 0,
	  RSocketTests },
  { "RConnection", RConnectionCUInit, RConnectionCUClean, 0, 0,
	  RConnectionTests },
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
