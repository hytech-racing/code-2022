# enable_testing()
include_directories(${INC}/gtest)

add_subdirectory(Unit)

add_subdirectory(BMS_Control_Unit)
add_subdirectory(Charger_Control_Unit)
add_subdirectory(Main_Control_Unit)
add_subdirectory(Telemetry_Control_Unit)
add_subdirectory(Wheel_Speed_Unit)