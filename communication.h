/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.3 at Sat Oct 08 12:55:05 2016. */

#ifndef PB_COMMUNICATION_PB_H_INCLUDED
#define PB_COMMUNICATION_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
/* Struct definitions */
typedef struct _CompressorInfo {
    int32_t compressorId;
    bool state_water_heating;
    bool state_room_heating;
    bool state_cooling;
    bool compressorRun;
    bool panicColdStopActive;
    bool panicHotStopActive;
    bool maintainceStop;
    bool has_pidOutput;
    float pidOutput;
} CompressorInfo;

typedef PB_BYTES_ARRAY_T(8) CompressorSettings_pidSensorAddr_t;
typedef PB_BYTES_ARRAY_T(8) CompressorSettings_hotWaterSensorAddr_t;
typedef PB_BYTES_ARRAY_T(8) CompressorSettings_coolingStartSensorAddr_t;
typedef PB_BYTES_ARRAY_T(8) CompressorSettings_coolingStopSensorAddr_t;
typedef struct _CompressorSettings {
    bool has_compressorId;
    int32_t compressorId;
    bool has_pidSetpoint;
    float pidSetpoint;
    bool has_pidP;
    float pidP;
    bool has_pidI;
    float pidI;
    bool has_pidD;
    float pidD;
    bool has_requestedHotWaterTemp;
    float requestedHotWaterTemp;
    bool has_heatingHysteresis;
    float heatingHysteresis;
    bool has_requestedCoolWaterTemp;
    float requestedCoolWaterTemp;
    bool has_coolingHysteresis;
    float coolingHysteresis;
    bool has_pidSensorAddr;
    CompressorSettings_pidSensorAddr_t pidSensorAddr;
    bool has_hotWaterSensorAddr;
    CompressorSettings_hotWaterSensorAddr_t hotWaterSensorAddr;
    bool has_coolingStartSensorAddr;
    CompressorSettings_coolingStartSensorAddr_t coolingStartSensorAddr;
    bool has_coolingStopSensorAddr;
    CompressorSettings_coolingStopSensorAddr_t coolingStopSensorAddr;
} CompressorSettings;

typedef struct _FlowInfo {
    bool has_sensorId;
    int32_t sensorId;
    bool has_LitersPerSecond;
    int32_t LitersPerSecond;
    bool has_absoluteValue;
    int32_t absoluteValue;
} FlowInfo;

typedef struct _PIDInfo {
    bool has_compressorId;
    int32_t compressorId;
    bool has_In;
    float In;
    bool has_Out;
    float Out;
    bool has_P;
    float P;
    bool has_I;
    float I;
    bool has_D;
    float D;
    bool has_dispP;
    float dispP;
    bool has_dispI;
    float dispI;
    bool has_dispD;
    float dispD;
} PIDInfo;

typedef PB_BYTES_ARRAY_T(8) TempInfo_SensorAddr_t;
typedef struct _TempInfo {
    bool has_SensorAddr;
    TempInfo_SensorAddr_t SensorAddr;
    bool has_Temp;
    float Temp;
} TempInfo;

/* Default values for struct fields */

/* Initializer values for message structs */
#define CompressorSettings_init_default          {false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, {0, {0}}, false, {0, {0}}, false, {0, {0}}, false, {0, {0}}}
#define CompressorInfo_init_default              {0, 0, 0, 0, 0, 0, 0, 0, false, 0}
#define TempInfo_init_default                    {false, {0, {0}}, false, 0}
#define PIDInfo_init_default                     {false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define FlowInfo_init_default                    {false, 0, false, 0, false, 0}
#define CompressorSettings_init_zero             {false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, {0, {0}}, false, {0, {0}}, false, {0, {0}}, false, {0, {0}}}
#define CompressorInfo_init_zero                 {0, 0, 0, 0, 0, 0, 0, 0, false, 0}
#define TempInfo_init_zero                       {false, {0, {0}}, false, 0}
#define PIDInfo_init_zero                        {false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define FlowInfo_init_zero                       {false, 0, false, 0, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define CompressorInfo_compressorId_tag          1
#define CompressorInfo_state_water_heating_tag   2
#define CompressorInfo_state_room_heating_tag    3
#define CompressorInfo_state_cooling_tag         4
#define CompressorInfo_compressorRun_tag         5
#define CompressorInfo_panicColdStopActive_tag   6
#define CompressorInfo_panicHotStopActive_tag    7
#define CompressorInfo_maintainceStop_tag        8
#define CompressorInfo_pidOutput_tag             9
#define CompressorSettings_compressorId_tag      1
#define CompressorSettings_pidSetpoint_tag       2
#define CompressorSettings_pidP_tag              3
#define CompressorSettings_pidI_tag              4
#define CompressorSettings_pidD_tag              5
#define CompressorSettings_requestedHotWaterTemp_tag 6
#define CompressorSettings_heatingHysteresis_tag 7
#define CompressorSettings_requestedCoolWaterTemp_tag 8
#define CompressorSettings_coolingHysteresis_tag 9
#define CompressorSettings_pidSensorAddr_tag     10
#define CompressorSettings_hotWaterSensorAddr_tag 11
#define CompressorSettings_coolingStartSensorAddr_tag 12
#define CompressorSettings_coolingStopSensorAddr_tag 13
#define FlowInfo_sensorId_tag                    1
#define FlowInfo_LitersPerSecond_tag             2
#define FlowInfo_absoluteValue_tag               3
#define PIDInfo_compressorId_tag                 1
#define PIDInfo_In_tag                           2
#define PIDInfo_Out_tag                          3
#define PIDInfo_P_tag                            4
#define PIDInfo_I_tag                            5
#define PIDInfo_D_tag                            6
#define PIDInfo_dispP_tag                        7
#define PIDInfo_dispI_tag                        8
#define PIDInfo_dispD_tag                        9
#define TempInfo_SensorAddr_tag                  1
#define TempInfo_Temp_tag                        2

/* Struct field encoding specification for nanopb */
extern const pb_field_t CompressorSettings_fields[14];
extern const pb_field_t CompressorInfo_fields[10];
extern const pb_field_t TempInfo_fields[3];
extern const pb_field_t PIDInfo_fields[10];
extern const pb_field_t FlowInfo_fields[4];

/* Maximum encoded size of messages (where known) */
#define CompressorSettings_size                  91
#define CompressorInfo_size                      30
#define TempInfo_size                            15
#define PIDInfo_size                             51
#define FlowInfo_size                            33

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define COMMUNICATION_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif