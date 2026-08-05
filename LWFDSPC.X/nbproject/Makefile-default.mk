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
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/56ab0896c1fa9e02d21236dd621745f28f6c93e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/f4925748d0e01fc4a3f83e338257181fde7ce70c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/19e7660061ec9bb84f3a2529fbb891404c9d0463 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/350e70abc5c0346946760662c07fd43ca9ed0f16 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/7173f7d701a521be01ebcb793daf7c4bbd4329b1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/7204ef9d4c53e6bdae984b1a7d39a96e5afeeb42 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/4cf39f761028f783ccef0cf527eb9a534d979236 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/6ad3e7e1fc92f1c7c05566cfec962f80b199c324 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/6905a1ce1b531b52495d978c7699da0fb4e8a79e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/5d86b8d874c027c3cbf1ca329c1908832e64b440 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/4e03911a79bbe32cd6f592732ecb78a48a904749 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/a38dfae78af7a5ee3a865521f3e091cda71327f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/7e29be20911ff3cf3479d2fc033db0f3fe9325d1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/c9cef4baf53458bc50b91fc4b7f1f73a01d5dec0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/48fc35915127e4140812111d7383d10a96d915d0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/22ccaf726209202524f4e65dcbd5719b8e507140 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/da39503e84aaa1be5b423ee01654e7778351475d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/3448028ba86c34279c3503a06417cf438f1ef472 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/92897e5ce769577b92ecab5c9993f187b4d6e4d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/cc9f95135126f666d6faa9ee6a19590a46bc8c0d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/d5bfd963d7b59ad506be48de98c2b9b255451736 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/832fcc409db59b03f783200b18fde14c9a8344d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/e746af85bcf76d4eef4fef10cef8e1e0ae496b72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/79552c652c48654596520fd51e64dce7bff5347b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/d80daa8728d95fd38343c4a8001732e93dfa50f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/9ddca75671e69d10af8bdaf3bbaeb9e2d1b9cc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/6ce8d60a17d968846ac39bb4ace1b3c750d52df7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/d8cd7ab121bf922c9ab02ed3e0e169ded88fcf73 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/fbba6e2ffbe44694dda8e1bb2eb0d64d7f672d00 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/702ff6aeb770ab43f4570d5378e7d8feaa95ad00 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/1f0c19ab95a0df14c4b1dc7b154de9743cd6ee2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/1fc57a11f95614b080dd419c5d9d7532feccdb93 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/99c96ccbcc96341bd28d0b44da1b213208f570a6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/94efa13f4c192b04706c22f75f1d8836937547f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/6502d8d39b3fe5f5842450f5ca3bdf9b728fe26b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/f6c57d09e27cef0182265a0191f506ff53f71a1b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/93db4cff3cf7bb880464d10793324799e1f1309d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/51cba60138256da48ac649238b32666c50b94bb8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/4fe4cb8d8a32896ef4100bc8979505b9831fdc9b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/26d2cd68f9df5462d1484998c510c62ff81c01f5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/2fe027d0e9bb4d1c8a30edf14ab2b3f7515fd8af .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/a502541d237649a99caea81872806868eacef381 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/3f58ac2bfade96c83b8b2785b2b5e56cc9acdaf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/28f3f50b0a2e5e7eff86a10bc558a28d034b573b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/40b8218b98583639a6273f7aecd79b67c4149d98 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/abf83bb4bd3a3ddef611b258b67fccd213b7a662 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/b9d7b34344763e401969cad37978030bce8216ff .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/98dfbf3786096b237ca8d0adc19d11579d75c8c3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/1b51db1653809ba75e202739ddd9026f4d8956c0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/133f8c97911d1fd04cd0ad322a00d8fbba1836d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/dd133d699af2c25c063081763b6b33078682510b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/7ca80cbb552e37a8d4c399e19489da8b525cb20d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/b41f5d3a6da69c412734634a98643530e3686e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/71cec4d264476ed624345288bc931bf1e23cfc86 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/76248bf9e7c5f93ae639fd102648fb261816643f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/3192f0fa29d40e5317bf5cdde503b66cff82269f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/1a32aa93ab8b3627798a454f2df16d80b5bc9636 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/14b2db6b2ad34b4dd1dc1e320d564578f7d58d38 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/48637db62aa59e4ebc2cf9ef04ac10960344fa7b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/de5e050552e985f6294adc458147373aeea156 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o.d 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/SpeedCalc.s  -o ${OBJECTDIR}/src/SpeedCalc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/SpeedCalc.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/868648243ba869adf27a011230b84d46f50a2820 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/d5048d49ed0b32b0224fafb53a625d69004714a9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/608a2c61c606221cae8d0f7706e81726df941bf4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/180c936403c660ef0d437596e941f884d57b34f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
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
