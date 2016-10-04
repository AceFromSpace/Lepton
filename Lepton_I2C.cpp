#include "Lepton_I2C.h"


#include "leptonSDKEmb32PUB/LEPTON_SDK.h"
#include "leptonSDKEmb32PUB/LEPTON_SYS.h"
#include "leptonSDKEmb32PUB/LEPTON_Types.h"
#include "leptonSDKEmb32PUB/LEPTON_AGC.h"
#include <iostream>
bool _connected;

LEP_CAMERA_PORT_DESC_T _port;
LEP_AGC_ENABLE_E _eneb;
LEP_SYS_FPA_TEMPERATURE_KELVIN_T fpa_temp_kelvin;
LEP_SYS_AUX_TEMPERATURE_KELVIN_T aux_temp_kelvin;
LEP_RESULT result;

int lepton_connect() {
	LEP_OpenPort(1, LEP_CCI_TWI, 400, &_port);
	_connected = true;
	return 0;
}

double lepton_temperature_fpa()
{
	if(!_connected) {
		lepton_connect();
	}
	

result =((LEP_GetSysFpaTemperatureKelvin(&_port,&fpa_temp_kelvin)));

return (fpa_temp_kelvin-27315);

}

double lepton_temperature_aux()
{
	if(!_connected) {
		lepton_connect();
	}
	

result =((LEP_GetSysAuxTemperatureKelvin(&_port,&aux_temp_kelvin)));

return (aux_temp_kelvin-27315);

}

void lepton_perform_ffc() {
	if(!_connected) {
		lepton_connect();
	}
	LEP_RunSysFFCNormalization(&_port);
}

void lepton_enable_agc() 
{
	if(!_connected) {
		lepton_connect();}
	
	result = LEP_GetAgcEnableState(&_port,&_eneb);
	if(result!=LEP_OK)std::cout<<"not ok"<<std::endl;
	result=LEP_SetAgcEnableState(&_port,LEP_AGC_ENABLE);
}

void lepton_disable_agc()
{
		if(!_connected) {
		lepton_connect();
	}
		
	result = LEP_GetAgcEnableState(&_port,&_eneb);
	if(result!=LEP_OK)std::cout<<"not ok"<<std::endl;
	result=LEP_SetAgcEnableState(&_port,LEP_AGC_DISABLE);
}

//presumably more commands could go here if desired
