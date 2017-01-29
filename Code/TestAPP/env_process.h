#ifndef __ENV_PROCESS_H
#define __ENV_PROCESS_H


enum eContrMode{
  cmHeating,
  cmCooling,
  cmThermostatic
};


typedef struct sTHW_par{
  uint8_t ContrMode:2,
        HeatingMode:1,
        
  
}THW_par;



#endif
