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
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/eb7f086f63e79330fd340598ca5933afa28b571 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/87a1d422932c731968951142be6b33d51fd1f543 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/a450855cd478fd86456e60bab04959ea16f0b4bc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/9e01df3db8ca1b0d07a315aa5fc18779c35ce2e7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/553b2937486d6eba1add1e917630e714f0c6430 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/d90a69ec1c12ba0c30fee4573f30d1abcfde3b6e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/8293cb32a06620b0518775552f6a93f36165638d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/c44a8c22ff36746025111610971f02b3fcf1bb83 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/acb45715afc212a9f83a65c4ed35340be1ce24e8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/d91bc9a85bb2ba0406060cbff935191c58ac06c9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/7a4a185f0f7564e11681286ba5f2e279c04f1e72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/9acf5b4345a61562036a815c84e00cfe7e98a03f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/bc9d41f5fb849c04ef6173d391e42d5a9d0f37d8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/5795a86035e9ce751f3463168d904dc6e11a7906 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/88575b4885496625b67a9ccec8e902e3383c1af .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/361174e000c738b02210f6cc86baee6fd456b7d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/ae426fac635b9ee9e1507b86b5064b0c6fd2a6e2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/a995be9abeecd293f92c3e6fd7eeceffd05ddc1a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/4c7213d4997765b7ed23c62069cb454e7347a23b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/9ae8aeb55630a42d1237c7764c20ba2ba0c1c669 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/9c51124de3ecb7e0ac4f267b845a3cf02f800544 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/ea9232651ace337fe004b73e4bde90ff45095390 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/b80e82c96c330e1698c4812076d08ffdc6c86eab .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/4674dbcbed4a4adccf507534796756e6d3082e5a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/3f66a69236ed3b013a71865833785433d90e3c69 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/dc15011e11a5cbe157e2b5395e0eec47524ffefc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/e82af777faab495b751c9bceee2e5352077463c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/9971d34b4a31b37ca20d70c4b1c3629de139fff .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o: diagnostics/diagnostics_x2cscope.c  .generated_files/flags/default/983f8f64380cfd78aabcecccf244965640ff655e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/diagnostics" 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d 
	@${RM} ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  diagnostics/diagnostics_x2cscope.c  -o ${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/diagnostics/diagnostics_x2cscope.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/adc.o: hal/adc.c  .generated_files/flags/default/c51c2423b56e11a24aaa54137d601c4ab32b76d6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/adc.o.d 
	@${RM} ${OBJECTDIR}/hal/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/adc.c  -o ${OBJECTDIR}/hal/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/adc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/board_service.o: hal/board_service.c  .generated_files/flags/default/669492667b4beb1d5356d5eb22aa111de9b30c05 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/board_service.o.d 
	@${RM} ${OBJECTDIR}/hal/board_service.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/board_service.c  -o ${OBJECTDIR}/hal/board_service.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/board_service.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/clock.o: hal/clock.c  .generated_files/flags/default/c7b2a27910c6871e2f14bf1840c88fb151d23d54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/clock.o.d 
	@${RM} ${OBJECTDIR}/hal/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/clock.c  -o ${OBJECTDIR}/hal/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/cmp1.o: hal/cmp1.c  .generated_files/flags/default/c38b9558ac0fe1e1717788df181d3f2681b8eb58 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/cmp1.o.d 
	@${RM} ${OBJECTDIR}/hal/cmp1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/cmp1.c  -o ${OBJECTDIR}/hal/cmp1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/cmp1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/device_config.o: hal/device_config.c  .generated_files/flags/default/464270a0528ba2dcabc47e426534a9f807480c05 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/device_config.o.d 
	@${RM} ${OBJECTDIR}/hal/device_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/device_config.c  -o ${OBJECTDIR}/hal/device_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/device_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/port_config.o: hal/port_config.c  .generated_files/flags/default/97f79f7842108092d4928bf6369bedfa42c5386c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/port_config.o.d 
	@${RM} ${OBJECTDIR}/hal/port_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/port_config.c  -o ${OBJECTDIR}/hal/port_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/port_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/pwm.o: hal/pwm.c  .generated_files/flags/default/a4e57c32c7ffbae9db9d7439630ccf375f0773e0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/pwm.o.d 
	@${RM} ${OBJECTDIR}/hal/pwm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/pwm.c  -o ${OBJECTDIR}/hal/pwm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/pwm.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hal/uart1.o: hal/uart1.c  .generated_files/flags/default/acbf2d856017e181b84b8bebe7dbfd62df74f3fd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/hal" 
	@${RM} ${OBJECTDIR}/hal/uart1.o.d 
	@${RM} ${OBJECTDIR}/hal/uart1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hal/uart1.c  -o ${OBJECTDIR}/hal/uart1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hal/uart1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_throttle.o: src/longwin/s_logic_throttle.c  .generated_files/flags/default/504fec3f079e3ca03b9adf2fe3b36e61c6ea9e6b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_throttle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_throttle.c  -o ${OBJECTDIR}/src/longwin/s_logic_throttle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_throttle.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_error_handler.o: src/longwin/s_logic_error_handler.c  .generated_files/flags/default/fd0d498b549f48a67da88c34bb9d0f8d10c0ed3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_error_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_error_handler.c  -o ${OBJECTDIR}/src/longwin/s_logic_error_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_error_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_convert.o: src/longwin/s_logic_convert.c  .generated_files/flags/default/c987c71d59193e5ef06ebb7ed3542ffac5627fc2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_convert.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_convert.c  -o ${OBJECTDIR}/src/longwin/s_logic_convert.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_convert.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_battery.o: src/longwin/s_logic_battery.c  .generated_files/flags/default/80976c9545313f9261e025cab095f259ab35fdac .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_battery.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_battery.c  -o ${OBJECTDIR}/src/longwin/s_logic_battery.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_battery.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_motor.o: src/longwin/s_logic_motor.c  .generated_files/flags/default/3fdd0ee49ab27ea679db02aedbd74d3c5e06acb2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_vr.o: src/longwin/s_logic_vr.c  .generated_files/flags/default/5b01cadfda09dbbbbc785b91caadd60024a3fe40 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_vr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_vr.c  -o ${OBJECTDIR}/src/longwin/s_logic_vr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_vr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_controller.o: src/longwin/s_logic_temp_controller.c  .generated_files/flags/default/b29a39cf8761f6722108e262ac86d199f2244543 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_controller.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_controller.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_controller.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_temp_motor.o: src/longwin/s_logic_temp_motor.c  .generated_files/flags/default/2fe8a98456730cb741432fb0de1f8c07ac855dcb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_temp_motor.c  -o ${OBJECTDIR}/src/longwin/s_logic_temp_motor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_temp_motor.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_master.o: src/longwin/s_modbus_master.c  .generated_files/flags/default/ebd121147b47e7b1ca4c8981c08fe5e5f2660edc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_master.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_master.c  -o ${OBJECTDIR}/src/longwin/s_modbus_master.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_master.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_hal_rs485.o: src/longwin/s_hal_rs485.c  .generated_files/flags/default/28730f6b353fdd370bd3c3ac512a164edb00dc67 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_hal_rs485.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_hal_rs485.c  -o ${OBJECTDIR}/src/longwin/s_hal_rs485.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_hal_rs485.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_modbus_decode.o: src/longwin/s_modbus_decode.c  .generated_files/flags/default/8d410a23c4cdde48cb9c11adbb56304121f6c67a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_modbus_decode.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_modbus_decode.c  -o ${OBJECTDIR}/src/longwin/s_modbus_decode.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_modbus_decode.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/codeSw.o: src/longwin/codeSw.c  .generated_files/flags/default/3eed699e38b9ec73378d0a2070918c6cc02eec27 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/codeSw.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/codeSw.c  -o ${OBJECTDIR}/src/longwin/codeSw.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/codeSw.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/longwin/s_logic_embraker.o: src/longwin/s_logic_embraker.c  .generated_files/flags/default/5dc4a9de45b5b29c78e728aadad7f53bd7026eaa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/longwin" 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o.d 
	@${RM} ${OBJECTDIR}/src/longwin/s_logic_embraker.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/longwin/s_logic_embraker.c  -o ${OBJECTDIR}/src/longwin/s_logic_embraker.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/longwin/s_logic_embraker.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/mcc_generated_files/can1.o: mcc_generated_files/can1.c  .generated_files/flags/default/91b44f3a1431611215245b5f69cee44373400f05 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/can1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  mcc_generated_files/can1.c  -o ${OBJECTDIR}/mcc_generated_files/can1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/mcc_generated_files/can1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/cn_configure.o: src/cn_configure.c  .generated_files/flags/default/9d6c16df42547d0b1a487f346ded72a27487c7cb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/cn_configure.o.d 
	@${RM} ${OBJECTDIR}/src/cn_configure.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/cn_configure.c  -o ${OBJECTDIR}/src/cn_configure.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/cn_configure.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/overcurrent_enable.o: src/overcurrent_enable.c  .generated_files/flags/default/aaf67548dd03e0a81261aa4fabecf9f25d83f936 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o.d 
	@${RM} ${OBJECTDIR}/src/overcurrent_enable.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/overcurrent_enable.c  -o ${OBJECTDIR}/src/overcurrent_enable.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/overcurrent_enable.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/sccp3_tmr.o: src/sccp3_tmr.c  .generated_files/flags/default/cf25c50a2e493d77ebd9b440571b9575d803ab19 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o.d 
	@${RM} ${OBJECTDIR}/src/sccp3_tmr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/sccp3_tmr.c  -o ${OBJECTDIR}/src/sccp3_tmr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/sccp3_tmr.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/HallScan_V2_1.o: src/HallScan_V2_1.c  .generated_files/flags/default/14f5a9a58df3425b5fe7d60d2610e768d80cac66 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o.d 
	@${RM} ${OBJECTDIR}/src/HallScan_V2_1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/HallScan_V2_1.c  -o ${OBJECTDIR}/src/HallScan_V2_1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/HallScan_V2_1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/b4c7a7f2c2f0ccde10c99ce823716f31c8f4989b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -I"../" -I"../hal" -I"../lib" -I"../lib/motor_control" -I"../diagnostics" -DMCLV2 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/3363396748db0ad623572179f9366dfddeb72907 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/1be6ca2c1261b09ae5f09558299609c0e28ee92 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/c16209efed9b65a9399d2cf8a32b9460e8babb26 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/cb22a6d5f0ef6b009141b3a700d24f0ac1caa909 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o.d 
	@${RM} ${OBJECTDIR}/src/SpeedCalc.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/SpeedCalc.s  -o ${OBJECTDIR}/src/SpeedCalc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/SpeedCalc.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/src/meascurr.o: src/meascurr.s  .generated_files/flags/default/d9794ea2a830f6326fa3b2a2eddab65954b45a6b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/meascurr.o.d 
	@${RM} ${OBJECTDIR}/src/meascurr.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/meascurr.s  -o ${OBJECTDIR}/src/meascurr.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/meascurr.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/q15sqrt.o: src/q15sqrt.s  .generated_files/flags/default/e22110c6e68ec16d0cb8e13f8747960e4a728d87 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o.d 
	@${RM} ${OBJECTDIR}/src/q15sqrt.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/q15sqrt.s  -o ${OBJECTDIR}/src/q15sqrt.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/q15sqrt.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/readadc0.o: src/readadc0.s  .generated_files/flags/default/3475c6f574595e05c9fd35542d0c935c3c874f9d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/readadc0.o.d 
	@${RM} ${OBJECTDIR}/src/readadc0.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  src/readadc0.s  -o ${OBJECTDIR}/src/readadc0.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DXPRJ_default=$(CND_CONF)    -I"lib" -I"../" -I"lib/motor_control" -I"src" -Wa,-MD,"${OBJECTDIR}/src/readadc0.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/SpeedCalc.o: src/SpeedCalc.s  .generated_files/flags/default/a3e6d8fdcb3f1e7a7c0175ed66f6fb82cd6608d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    diagnostics\libx2cscope_33ck.a lib\motor_control\libmotor_control_dspic-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_PK5=1  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x1000:0x101B -mreserve=data@0x101C:0x101D -mreserve=data@0x101E:0x101F -mreserve=data@0x1020:0x1021 -mreserve=data@0x1022:0x1023 -mreserve=data@0x1024:0x1027 -mreserve=data@0x1028:0x104F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_PK5=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library=q,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  diagnostics/libx2cscope_33ck.a lib/motor_control/libmotor_control_dspic-elf.a 
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    diagnostics\libx2cscope_33ck.a lib\motor_control\libmotor_control_dspic-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library=q,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}\\xc16-bin2hex ${DISTDIR}/LWFDSPC.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
