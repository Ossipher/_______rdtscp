#include <Windows.h>
#include <intrin.h>
#include <stdio.h>
#include <winnt.h>


#pragma intrinsic(__rdtscp) // grabs internal clock cycles + but with stable without accidently miss predicting


// globally separated variables to obscure data dependencies
static unsigned __int64 g_TscMarkerA = 0;
static unsigned __int64 g_DynamicObfuscatonKey = 0x5A5A5A5A;

#define MSVC_FENCE() _ReadWriteBarrier() // tell MSVC to  avoid reordering reads or writes across that point during optimization

void ApplyTimingCheckpoint(void) { // entry point 

	unsigned int aux = 0;
	MSVC_FENCE();
	g_TscMarkerA = __rdtscp(&aux); // mix current hardware timestamp with the Obf key
	g_DynamicObfuscatonKey ^= (g_TscMarkerA & 0xFFFF); // avoid prediction with shifting the key
	MSVC_FENCE();
}


void EvaluateTimingIntregreity(void) {
	
	unsigned int aux = 0;
	MSVC_FENCE();
	unsigned __int64 current_time = __rdtscp(&aux);

	unsigned __int64 delta = current_time - g_TscMarkerA; // calculate actual elapsed clock cycles
	// constantly scramble the global key to avoid hardcoding so to be randomized every func run
	unsigned __int64 localized_envelope = 350000 + (g_DynamicObfuscatonKey & 0x7FFF);

	// if delta > enevelope -- the math stes compromised to non-zero value 
	// avoid exit();
	volatile unsigned __int64 compromised = (delta > localized_envelope) ? 0xBAD1 : 0x0;

	if (compromised != 0x0) {
		// FAST FAIL instantly terminates entire process :p
		__fastfail(FAST_FAIL_FATAL_APP_EXIT);
	}

}



void ExecuteDemo__(void) {

	// drop entry point
	ApplyTimingCheckpoint();

	int a = 10;
	int random[12] = { 0 };
	for (int i = 0; i < 10; i++) {
		
		random[i] = i << 5; //random operations
		printf("> i now is [%d] \n", i);
		printf("> left shift is now [%d] \n", random[i]);
	}

	EvaluateTimingIntregreity();

}


int main() {

	ExecuteDemo__();
	return 0;

}


