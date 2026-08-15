#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=diagnostics/diagnostics_x2cscope.c hal/adc.c hal/board_service.c hal/clock.c hal/cmp1.c hal/device_config.c hal/port_config.c hal/pwm.c hal/uart1.c src/longwin/s_logic_throttle.c src/longwin/s_logic_error_handler.c src/longwin/s_logic_convert.c src/longwin/s_logic_battery.c src/longwin/s_logic_motor.c src/longwin/s_logic_vr.c src/longwin/s_logic_temp_controller.c src/longwin/s_logic_temp_motor.c src/longwin/s_modbus_master.c src/longwin/s_hal_rs485.c src/longwin/s_modbus_decode.c src/longwin/codeSw.c src/longwin/s_logic_embraker.c mcc_generated_files/can1.c src/cn_configure.c src/meascurr.s src/overcurrent_enable.c src/q15sqrt.s src/readadc0.s src/sccp3_tmr.c src/SpeedCalc.s src/HallScan_V2_1.c main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o ${OBJECTDIR}/hal/adc.o ${OBJECTDIR}/hal/board_service.o ${OBJECTDIR}/hal/clock.o ${OBJECTDIR}/hal/cmp1.o ${OBJECTDIR}/hal/device_config.o ${OBJECTDIR}/hal/port_config.o ${OBJECTDIR}/hal/pwm.o ${OBJECTDIR}/hal/uart1.o ${OBJECTDIR}/src/longwin/s_logic_throttle.o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o ${OBJECTDIR}/src/longwin/s_logic_convert.o ${OBJECTDIR}/src/longwin/s_logic_battery.o ${OBJECTDIR}/src/longwin/s_logic_motor.o ${OBJECTDIR}/src/longwin/s_logic_vr.o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o ${OBJECTDIR}/src/longwin/s_modbus_master.o ${OBJECTDIR}/src/longwin/s_hal_rs485.o ${OBJECTDIR}/src/longwin/s_modbus_decode.o ${OBJECTDIR}/src/longwin/codeSw.o ${OBJECTDIR}/src/longwin/s_logic_embraker.o ${OBJECTDIR}/mcc_generated_files/can1.o ${OBJECTDIR}/src/cn_configure.o ${OBJECTDIR}/src/meascurr.o ${OBJECTDIR}/src/overcurrent_enable.o ${OBJECTDIR}/src/q15sqrt.o ${OBJECTDIR}/src/readadc0.o ${OBJECTDIR}/src/sccp3_tmr.o ${OBJECTDIR}/src/SpeedCalc.o ${OBJECTDIR}/src/HallScan_V2_1.o ${OBJECTDIR}/main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d ${OBJECTDIR}/hal/adc.o.d ${OBJECTDIR}/hal/board_service.o.d ${OBJECTDIR}/hal/clock.o.d ${OBJECTDIR}/hal/cmp1.o.d ${OBJECTDIR}/hal/device_config.o.d ${OBJECTDIR}/hal/port_config.o.d ${OBJECTDIR}/hal/pwm.o.d ${OBJECTDIR}/hal/uart1.o.d ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d ${OBJECTDIR}/src/longwin/s_logic_convert.o.d ${OBJECTDIR}/src/longwin/s_logic_battery.o.d ${OBJECTDIR}/src/longwin/s_logic_motor.o.d ${OBJECTDIR}/src/longwin/s_logic_vr.o.d ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d ${OBJECTDIR}/src/longwin/s_modbus_master.o.d ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d ${OBJECTDIR}/src/longwin/codeSw.o.d ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d ${OBJECTDIR}/mcc_generated_files/can1.o.d ${OBJECTDIR}/src/cn_configure.o.d ${OBJECTDIR}/src/meascurr.o.d ${OBJECTDIR}/src/overcurrent_enable.o.d ${OBJECTDIR}/src/q15sqrt.o.d ${OBJECTDIR}/src/readadc0.o.d ${OBJECTDIR}/src/sccp3_tmr.o.d ${OBJECTDIR}/src/SpeedCalc.o.d ${OBJECTDIR}/src/HallScan_V2_1.o.d ${OBJECTDIR}/main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o ${OBJECTDIR}/hal/adc.o ${OBJECTDIR}/hal/board_service.o ${OBJECTDIR}/hal/clock.o ${OBJECTDIR}/hal/cmp1.o ${OBJECTDIR}/hal/device_config.o ${OBJECTDIR}/hal/port_config.o ${OBJECTDIR}/hal/pwm.o ${OBJECTDIR}/hal/uart1.o ${OBJECTDIR}/src/longwin/s_logic_throttle.o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o ${OBJECTDIR}/src/longwin/s_logic_convert.o ${OBJECTDIR}/src/longwin/s_logic_battery.o ${OBJECTDIR}/src/longwin/s_logic_motor.o ${OBJECTDIR}/src/longwin/s_logic_vr.o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o ${OBJECTDIR}/src/longwin/s_modbus_master.o ${OBJECTDIR}/src/longwin/s_hal_rs485.o ${OBJECTDIR}/src/longwin/s_modbus_decode.o ${OBJECTDIR}/src/longwin/codeSw.o ${OBJECTDIR}/src/longwin/s_logic_embraker.o ${OBJECTDIR}/mcc_generated_files/can1.o ${OBJECTDIR}/src/cn_configure.o ${OBJECTDIR}/src/meascurr.o ${OBJECTDIR}/src/overcurrent_enable.o ${OBJECTDIR}/src/q15sqrt.o ${OBJECTDIR}/src/readadc0.o ${OBJECTDIR}/src/sccp3_tmr.o ${OBJECTDIR}/src/SpeedCalc.o ${OBJECTDIR}/src/HallScan_V2_1.o ${OBJECTDIR}/main.o

# Source Files
SOURCEFILES=diagnostics/diagnostics_x2cscope.c hal/adc.c hal/board_service.c hal/clock.c hal/cmp1.c hal/device_config.c hal/port_config.c hal/pwm.c hal/uart1.c src/longwin/s_logic_throttle.c src/longwin/s_logic_error_handler.c src/longwin/s_logic_convert.c src/longwin/s_logic_battery.c src/longwin/s_logic_motor.c src/longwin/s_logic_vr.c src/longwin/s_logic_temp_controller.c src/longwin/s_logic_temp_motor.c src/longwin/s_modbus_master.c src/longwin/s_hal_rs485.c src/longwin/s_modbus_decode.c src/longwin/codeSw.c src/longwin/s_logic_embraker.c mcc_generated_files/can1.c src/cn_configure.c src/meascurr.s src/overcurrent_enable.c src/q15sqrt.s src/readadc0.s src/sccp3_tmr.c src/SpeedCalc.s src/HallScan_V2_1.c main.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33CK256MP506
MP_LINKER_FILE_OPTION=,--script=p33CK256MP506.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/9a463fc5fc36d8045ab8d5c86ea8e96c2b05a4b5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/6574c9ad30b3a233dc308ee9b293b663292e5ad1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/2ec736bc09714ac999aa1a7141bd4e97aabd78f2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/f8f9e327d15e18269fe822ec3dd1b42dce37204a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/51869e34a68a46538ce850027833afaaf910c948 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/6778fbf10d7ec614f07ac335cb346987e1672b2f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/9dc71b4b48d2d1f2d972e7dc77fa6b7b6073bd3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/4b6a929893200c23b19d75f766893a8e468f12f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/36706316697e03e60ead8b18cdb5de21dba680da .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/b8b38fb2f0e2c2c4fc219b0373ab07068e62838 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/1b5f3c832c6950c2df20e80f67f37b9ce49cc39d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/1946f95b9f69b09d3c6ace1c5fd708f2d05b248c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/51f0fd709f863ebf3ecff6f2b7bc6bec7b12ad50 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/fa3d43360ef83bd93ee137bf77447e92a74dc462 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/b4c8d15a1d1eaadb51f265880db8027b2979e59c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/6f03166a6b0f072cc10704d2915ce41c9708a1ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/e5090171f38407be518426cb9aa7941e1048cb25 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/31cf0a2aa25edd5cc9062c421558a9e098befbae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/8caabafc0d11e8bccb75f79eb1dbeb00e9823006 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/9d54431d3b8a5566d6892dd8494decaa0c0faab5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/37160ec3567843d338c9c59ae224589b665ec74b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/d0d55aeff2b33d828bc59e619093a6fcefdaf052 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/ce4becb4702248e8369452cb4252c620a114bd53 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/40e1fb16178b6e6173f272f185a3b2d0e0fcfd0b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/cc04774cf6ea156901959f7bcb7ec42ce4c1e182 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/a347fa4549c0e40ec6072317fb0accf9b58137c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/461f12f06cb27ab7ee7f2e2099492b7bfd5e9e31 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/1a9c9af71b4a099e2b7042039c7d5052c8a2cbc7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/7e36a4d0c73ae8cbb8e6f2578edadd0c7c4f9153 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/e84ed1e601c5120145d25f6ee4b02d937993cf4c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/f9466b9af1e6b80986f0f13cf8dc6566427b934c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/77763a85d320a4d9c16aa79554ce15785228a6ae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/5ce7c6df88e23ade3bed5e697e95a352dcfc798c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/c5a63c7f3b942aee959b9ecb1dec1a90e61714d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/68e660f118e14ebfa76bcb82684c81a979187281 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/7add4fb0487c1882a7fff9c9abf9759a7bcbf70d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/f535e963ca0271a4879052312faf519551c3dbbf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/e606caece450e783aba96e8d5acfb019bcdee8d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/8858e9105facbad6f29cabc31317b80c617bfe71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/70bced898304e470b2ca2ca15b891328d0c3ad33 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/fe5ca6fcc9cc0689c3ed2b679d9f64f54a46eb9e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/2fabbe39fc374a8efd307eaf5a113aa22693be34 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/afc7ef9c8043ca89fbbe64afeaa35ec20db53aaf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/299f12ab974545fb93e3dd34c057a262fa4b89f4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/f4a4aeb03434a3c37eea0b67529ff9eaae0232be .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/c11610855728680c41981d81f072502bd3602afd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/115ee7597caf442e06af44a33efc022a15d6e0fd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/18f55079c89199968e0ef22e9ef644075db46266 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/644a59087924a97c9c4c2700a738ad8de76ea9e9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/1215bc96f87a48c67d9f7adaf40e4a534e1be431 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/d2c8a4dca37c6bd48203bb2db3c406da43667ea2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/b6ce2954503988ab310160aa6e1838005ede8930 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/d95a4bf367b46ebe4abc00d13db8f411a7b330b1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/f8c26ca44cb5a55c4737e6288e867e2f05ff0d2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/3f35b4681caa456d401c0296e5ee814c8049d917 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/e83b78a151cbeebf46e5b7448553ab8b12e877f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/90969423c983c92dd70735ecf4ac6944eae87af4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/4e2647eb411b491728eaf29924fa8fa99a62bd71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/cdacf15074e23cfd33205b263ec160db5bd705e9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/c5025d8ea8fd9235ca51d907ae18047b34fcd967 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o.d 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/SpeedCalc.s  -o ${OBJECTDIR}/src/SpeedCalc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/SpeedCalc.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/b8d4d1d97f4db13c3ac0c0daac6b2c14d2ffc8a2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/1692f6698bf212c8b77a1941243ff508c85d7580 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/6e0e6a015aaad4cbf038fc8c882382b7a291e2ed .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/60d56f94bbbdb43ebd0332791f3c3239b523c13f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o.d 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/SpeedCalc.s  -o ${OBJECTDIR}/src/SpeedCalc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/SpeedCalc.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  diagnostics/libx2cscope_33ck.a lib/motor_control/libmotor_control_dspic-elf.a  
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    diagnostics\libx2cscope_33ck.a lib\motor_control\libmotor_control_dspic-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x1000:0x101B -mreserve=data@0x101C:0x101D -mreserve=data@0x101E:0x101F -mreserve=data@0x1020:0x1021 -mreserve=data@0x1022:0x1023 -mreserve=data@0x1024:0x1027 -mreserve=data@0x1028:0x104F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_PK5=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--no-gc-sections,--fill-upper=0,--stackguard=16,--library=q,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  diagnostics/libx2cscope_33ck.a lib/motor_control/libmotor_control_dspic-elf.a 
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    diagnostics\libx2cscope_33ck.a lib\motor_control\libmotor_control_dspic-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--no-gc-sections,--fill-upper=0,--stackguard=16,--library=q,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}\\xc-dsc-bin2hex ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
