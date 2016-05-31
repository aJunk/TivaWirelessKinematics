################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
functions.obj: ../functions.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --include_path="J:/Studium/Semester_4/EMB/CCS_workspace/git/connected_launchpad_driverlib" -g --gcc --define=ccs="ccs" --define=PART_TM4C1294NCPDT --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="functions.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --include_path="J:/Studium/Semester_4/EMB/CCS_workspace/git/connected_launchpad_driverlib" -g --gcc --define=ccs="ccs" --define=PART_TM4C1294NCPDT --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

motors.obj: ../motors.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --include_path="J:/Studium/Semester_4/EMB/CCS_workspace/git/connected_launchpad_driverlib" -g --gcc --define=ccs="ccs" --define=PART_TM4C1294NCPDT --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="motors.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

tm4c1294ncpdt_startup_ccs.obj: ../tm4c1294ncpdt_startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="C:/Users/Johannes/ccs/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --include_path="J:/Studium/Semester_4/EMB/CCS_workspace/git/connected_launchpad_driverlib" -g --gcc --define=ccs="ccs" --define=PART_TM4C1294NCPDT --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="tm4c1294ncpdt_startup_ccs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


