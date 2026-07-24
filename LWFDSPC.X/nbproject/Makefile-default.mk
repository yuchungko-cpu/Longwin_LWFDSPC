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
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/ea043959654b15eaaf437f2a4e7f2897130ff89e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/a662a87f98b99dec7281ff4f58af3d16ab758e05 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/b74eb8de3d99b4510532fa00c1dea4ebef65667b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/b60544894f4d4c5550f06a3824969e1799a9eb3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/54357dd56823a9a6e4e0b5b955b523d1214819f4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/55ecce6cd04629b72db87286c9b04a23bc0ac512 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/e1820f4a92fffb53f427b4334ac3e16eda57f2e3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/da7ed03e53abcd4190201014d9f9dcedada99ab6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/259315c1ed05270772ccb08013dcb30fe8c1c06e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/b926246fa2d43860cbcc21d3d19d6e8b81ca52cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/8245691130263da1253c6986cadc5213d9ddaf52 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/75d55a15968a9450d5a18e5c661e99dd04410c84 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/b7cb33acbd9fe43531aeacb7be4bac1ead5956f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/fad0f9896239963f7145c5929edddcfa750d137c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/f3b109f382cf5b05aa6ff2af57a6390c9f8e316a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/64c1ee327d84ca5921e733715a8d652d26721dfa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/4dda94aedea888eb10de82531416491756877937 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/d54793d5559c4a066ec6614ae26ac3624a6dcb9e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/1a44594eb3f9e929ae15bdbe3d1abfce498c00f2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/be34dd755d884233d56d87a0c9f5ea6548b13002 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/550ad05eb5c59bf363f450a24e5df8401a2c71d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/c090daa9b65981b98f7a2e2d2dcec47de8a0e307 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/255a3ea1bb4883b57faad6c63bb39e4963ba571 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/eb9d886f96b7c4ef6cb7a821cc1605577922fae9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/cc6e6e7de3eca06cc3c226983582299bcf2ccd45 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/fc2bb355147491020029c6c4c03067a7e2e5984f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/497a461f5944468fa6ceed348b66c7ea2af16a87 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/6e1f9c7969548cc982c144f3b2dc5e4f26b2732 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/16e17e036bace9ded2af86d0601114ffb4d3ff88 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/488a4e6b3d4a2e0571a6d24d8e973e120f8ed9ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/41f5fb02d2d70e88ed142f8a7a8bb2479c489f6c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/b79b5a87481dd6950cb8273c86495c564c02c62b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/ae7d7917faebb86bded8a59641d2fd6063a60b02 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/5338b479872cfcac8c0f4e6d66961398a8eb441d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/bc4975ea26343789f74e3190b6ed0f7113f87249 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/70e1f51932dd6618184c5b4d423e628391025cbb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/26ffdfaeeb48c740222fdd3a2b7809a6ea1a24b2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/a5c961cd7582da360d35da3dba682b076c259a65 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/81be6751d10d49b25a25e4580b9cc62ea4d31d32 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/f2869226af5f5fdd9f63712fa3a3044b7b03686 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/6c0c39e3de3faff63705ef30823daf7288d53ba8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/ae1fef5bb22c7d37de11915099f0e24974dc6033 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/2ee2a6de9d261c1b5fe9727c7ba9cbca515e34c0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/13830449201363904f0166d08547fc5d1171b3f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/1087be1abfc8cb6ab6e5bfaf691b6bc3e2e5970a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/60a1bef5a76dd9625b4283b8542ecce02c43e947 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/7a1a73ed7147f8992ec040e76565f8539a5ffec3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/3a57c6ec1a178f7c6164e24d17528b847efce41 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/3ebacff2278391fbbf24ff86235665479140d90a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/37b43a2c2c69244ece4a9191a7a76dc24e97b4c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/29cb991dbc64a0e23f8ec6fe9196ac4408f51ef1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/c718753f2d7c6ebff33140891871eded17488538 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/ddc41362c85bc2c5e3c9b75cb71bfbe758b3209 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/6e4eda3194da0c44a3479de123046cb5058ce66b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/2e1ed60fdfe8834ac47905af19b4863c9320b0e8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/b93d9bb7a5fa729379b35cd92d3fdc82fc9cb289 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/474a8c54e352881ac129aa5e96865b8a5b7accf8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/9b6a9844a61d2f57fa20f3581b7d408e3125fc5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/7cf9ac38046d581d7bc09dff1eb5eabba7d0c54f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/54af2173ff334c49cb2d198d6611e29986a57080 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o.d 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/SpeedCalc.s  -o ${OBJECTDIR}/src/SpeedCalc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/SpeedCalc.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/55a100958592ebc90c0e082816a64393c8cb199e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/7b850334d6c4b3c2707aa5a8fe3e4a9aa36eff15 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/525c2d753bb232d956f27b462baa91c56afabfa5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/2ea5c0f87c150f27f7f73b62ae085cb524293f7d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
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
