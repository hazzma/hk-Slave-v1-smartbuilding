#ifndef CONFIG_REGISTER_MAP_H
#define CONFIG_REGISTER_MAP_H

// Identity Registers (0x0000 - 0x0004)
#define REG_NODE_ADDRESS              0x0000
#define REG_FW_VERSION                0x0001
#define REG_MAC_0_1                   0x0002
#define REG_MAC_2_3                   0x0003
#define REG_MAC_4_5                   0x0004

// Capability Registers (0x0010 - 0x0017)
#define REG_TEMP_ASSIGNMENT           0x0010
#define REG_LUX_ASSIGNMENT            0x0011
#define REG_CO2_COUNT                 0x0012
#define REG_PRESENCE_ASSIGNMENT       0x0013
#define REG_RELAY_ASSIGNMENT          0x0014
#define REG_IR_PROJECTOR_ENABLE       0x0015
#define REG_IR_AC_1_ENABLE            0x0016
#define REG_IR_AC_2_ENABLE            0x0017

// Config / Recovery Registers (0x00F0 - 0x00F7)
#define REG_CONFIG_VERSION            0x00F0
#define REG_LAST_ERROR                0x00F1
#define REG_UPTIME_LOW                0x00F2
#define REG_UPTIME_HIGH               0x00F3
#define REG_RECOVERY_MAC_0_1          0x00F4
#define REG_RECOVERY_MAC_2_3          0x00F5
#define REG_RECOVERY_MAC_4_5          0x00F6
#define REG_RECOVERY_NODE_ADDRESS     0x00F7

// Sensor Registers (0x0100 - 0x010E)
#define REG_TEMP_1_X10                0x0100
#define REG_TEMP_2_X10                0x0101
#define REG_TEMP_3_X10                0x0102
#define REG_TEMP_4_X10                0x0103
#define REG_LUX_1_LX                  0x0104
#define REG_LUX_2_LX                  0x0105
#define REG_LUX_3_LX                  0x0106
#define REG_LUX_4_LX                  0x0107
#define REG_CO2_PPM                   0x0108
#define REG_PRESENCE_1_STATE          0x0109
#define REG_PRESENCE_2_STATE          0x010A
#define REG_PRESENCE_3_STATE          0x010B
#define REG_PRESENCE_4_STATE          0x010C
#define REG_RELAY_1_STATE             0x010D
#define REG_RELAY_2_STATE             0x010E

// Control Registers (0x0200 - 0x0212)
#define REG_AC_1_POWER                0x0200
#define REG_AC_1_SET_TEMP             0x0201
#define REG_AC_1_MODE                 0x0202
#define REG_AC_2_POWER                0x0203
#define REG_AC_2_SET_TEMP             0x0204
#define REG_AC_2_MODE                 0x0205
#define REG_AC_1_COMMAND_STATUS       0x0206
#define REG_AC_2_COMMAND_STATUS       0x0207
#define REG_AC_1_FAN_SPEED            0x0208
#define REG_AC_1_SWING_VERTICAL       0x0209
#define REG_AC_1_SWING_HORIZONTAL     0x020A
#define REG_AC_2_FAN_SPEED            0x020B
#define REG_AC_2_SWING_VERTICAL       0x020C
#define REG_AC_2_SWING_HORIZONTAL     0x020D
#define REG_PROJECTOR_POWER           0x0210
#define REG_PROJECTOR_INPUT           0x0211
#define REG_PROJECTOR_COMMAND_STATUS  0x0212

#endif // CONFIG_REGISTER_MAP_H
