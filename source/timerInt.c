#include <mkl25z4.h>
void init_LED();
void init_Timer();
#define RED_LED		(18)  	//B18
#define MASK(X)  (1<<X)

int main(){
	__disable_irq();
	init_LED();
	init_Timer();
    __enable_irq();
	while(1){}
}

void init_Timer(){
	// timer clk on
	SIM->SCGC6 |=SIM_SCGC6_TPM0_MASK;

	//TPMSRC = 0b01 --> select the MCG clock
	//remember: the MCG is set to the FLL which runs at 20.9 MHz
	SIM->SOPT2 |= 1 << 24;   //SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

	//Configure registers in TPM0

	//MOD register is the counter-compare value
	TPM0->MOD= 16300;

	//The Status&Control(SC) register needs to be configured
	TPM0->SC |= 1 << 7; 			//Clear TOF before start,
	TPM0->SC |= 0x7;				//optional pre-scale by 2^7 =128
	TPM0->SC |= 1 << 6; 			//Enable TOIE for interrupt
	TPM0->SC |= 1 << 3; 			//enable internal clock to run
/*
	// Alternatively, can use the MKL25z4.h MACROs for self-documenting code
	TPM0->SC |= TPM_SC_TOF_MASK; 	//Clear TOF before start,
	TPM0->SC |= TPM_SC_PS(7);		//optional pre-scale by 2^7 =128
	TPM0->SC |= TPM_SC_TOIE_MASK; 	//Enable TOIE for interrupt
	TPM0->SC |= TPM_SC_CMOD(1); 	//enable internal clock to run
*/


	NVIC->ICPR[0] = 1 << 17;		// clear pending interrupt 17 for TPM0
	NVIC->IP[4] = 0x3 << 8;			// set priority of TPM0 to "3"
	NVIC->ISER[0] = 1 << 17;		// enable interrupt 17 for TPM0
/*
	// Alternatively, can use the CMSIS API (pp. 107 of the Dean textbook)
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	NVIC_SetPriority(TPM0_IRQn, 3);
	NVIC_EnableIRQ(TPM0_IRQn);
*/
}

void TPM0_IRQHandler(){
	//interrupt frequency should be MCGCLK / TPM0 prescale / TPM0 MOD value
	//								20.9 MHz / 128 / 16300 = 10.0 Hz
	PTB->PTOR |= MASK(RED_LED);
	TPM0->SC |= TPM_SC_TOF_MASK ; //clear the interrupt
}


void init_LED(){
	SIM->SCGC5 |=SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[RED_LED ] &= ~PORT_PCR_MUX_MASK;	//Clear mux
	PORTB->PCR[RED_LED ] |= PORT_PCR_MUX(1);	//setup to be GPIO
	PTB->PDDR |= MASK(RED_LED ) ;
	PTB->PSOR =MASK(RED_LED);	//turn off RED
}
