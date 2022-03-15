close all
%% 
% Usage:
% 1. Load the data struct into the Workspace
% 2. Run individual sections of the script to plot the desired data

%% Torque, Vehicle Speed, Current
figure

requested_torque = S.requested_torque;
commanded_torque = S.commanded_torque;
feedback_torque = S.torque_feedback;
busCurrent = S.dc_bus_current;
busVoltage = S.dc_bus_voltage;
motor_speed = S.motor_speed;
vehicle_speed_mph = motor_speed;
vehicle_speed_mph(:,2) = -motor_speed(:,2).*0.225.*0.000284091.*pi.*60;
hold on
plot(motor_speed(:,1),-motor_speed(:,2)./100);
plot(busCurrent(:,1),busCurrent(:,2)./2);
plot(commanded_torque(:,1),commanded_torque(:,2)./10);
plot(feedback_torque(:,1),feedback_torque(:,2)./10);
plot(requested_torque(:,1),requested_torque(:,2)./10);
legend({'Motor Speed (RPM)*0.01','Current (A)*0.5','Commanded Torque*0.1 (Nm)','Feedback Torque*0.1 (Nm)','Requested Torque*0.1 (Nm)'})
xlabel('Time (ms)')
title('Torque, Speed, Current')

%% Pedal Input Traces
figure

front_brakes_data = S.brake_transducer_1(:, 2);
front_brakes_time = S.brake_transducer_1(:, 1);

pedal_data = S.accelerator_pedal_1(:, 2);
pedal_time = S.accelerator_pedal_1(:, 1);

% Normalizing and cleaning pedal traces
front_brakes_data = front_brakes_data - mode(front_brakes_data);
front_brakes_data(front_brakes_data < 0) = 0;
front_brakes_data = front_brakes_data/max(front_brakes_data);

pedal_data = pedal_data - mode(pedal_data);
pedal_data(pedal_data < 0) = 0;
pedal_data = pedal_data/max(pedal_data);

hold on
plot(pedal_time, pedal_data, '.-');
plot(front_brakes_time, front_brakes_data, '.-');
grid on

xlabel('Time (ms)')
ylabel('Normalized Pedal Position and Brake Pressure')
title('Brake and Pedal Traces')
legend({'Accelerator Pedal Position','Brake Pressure'})

%% DC Bus Current, DC Bus Voltage, and Calculated DC Power Output
figure
voltage = S.dc_bus_voltage; 
current = S.dc_bus_current;

% Data uniqueness
for i = 1:length(voltage(:,1))
    voltage(i,1) = voltage(i,1) + i/100000000;
end
for i = 1:length(current(:,1))
    current(i,1) = current(i,1) + i/100000000;
end
    
time = 1:0.1:max(current(:,1)); %Seconds
current_adj = interp1(current(:,1),current(:,2),time);
voltage_adj = interp1(voltage(:,1),voltage(:,2),time);
power = current_adj.*voltage_adj./1000;

hold on
plot(S.dc_bus_current(:,1), S.dc_bus_current(:,2), '.-');
plot(S.dc_bus_voltage(:,1), S.dc_bus_voltage(:,2), '.-'); 
plot(time, power, '.-')
grid on

xlabel('Time (ms)')
ylabel('Voltage (V), Current (A), Power (kW)')
title('DC Bus Current, DC Bus Voltage, and Calculated DC Power Output')
legend({'Current','Voltage','Power'})

%% Cooling Loop: Motor and MCU Temperatures
figure

hold on
plot(S.gate_driver_board_temperature(:,1),S.gate_driver_board_temperature(:,2))
plot(S.control_board_temperature(:,1),S.control_board_temperature(:,2))
plot(S.module_a_temperature(:,1),S.module_a_temperature(:,2))
plot(S.module_b_temperature(:,1),S.module_b_temperature(:,2)) 
plot(S.module_c_temperature(:,1),S.module_c_temperature(:,2))
plot(S.motor_temperature(:,1),S.motor_temperature(:,2))
plot(S.dc_bus_current(:,1),S.dc_bus_current(:,2)./10) 
grid on

legend({'MCU Gate Driver Board Temperature','MCU Control Board Temperature','MCU Module A Temperature','MCU Module B Temperature','MCU Module C Temperature','Motor Temperature','Current/10 (A)'})
xlabel('Time (ms)')
ylabel('Temperature (C)')
title('Cooling Loop Temperature Plots')

%% Accumulator Cell Temperatures
figure
hold on
plot(S.dc_bus_current(:,1),S.dc_bus_current(:,2)./10)
plot(S.BMS_average_temperature(:,1),S.BMS_average_temperature(:,2),'.')
plot(S.BMS_high_temperature(:,1),S.BMS_high_temperature(:,2),'.')
plot(S.BMS_low_temperature(:,1),S.BMS_low_temperature(:,2),'.') 
ylim([0,50]) % only BMS_state, BMS_total_discharge, BMS_total_charge
ylabel('Temperature (C)')
xlabel('Time (ms)')
title('Accumulator Cell Temperatures')
legend({'Current/10 (A)','BMS Average Temperature','BMS High Temperature','BMS Low Temperature'})

figure
subplot(2,2,1)
hold on
plot(S.IC_0_therm_0(:,1),S.IC_0_therm_0(:,2),'.')
plot(S.IC_0_therm_1(:,1),S.IC_0_therm_1(:,2),'.')
plot(S.IC_0_therm_2(:,1),S.IC_0_therm_2(:,2),'.')
plot(S.IC_1_therm_0(:,1),S.IC_1_therm_0(:,2),'.')
plot(S.IC_1_therm_1(:,1),S.IC_1_therm_1(:,2),'.')
plot(S.IC_1_therm_2(:,1),S.IC_1_therm_2(:,2),'.')
ylabel('Temperature (C)')
xlabel('Time (ms)')
title('Accumulator Cell Temperatures: Segment 1 Detailed View')
legend({'Temp1','Temp2','Temp3','Temp4','Temp5','Temp6'},'Location','southeast')
subplot(2,2,2)
hold on
plot(S.IC_2_therm_0(:,1),S.IC_2_therm_0(:,2),'.')
plot(S.IC_2_therm_1(:,1),S.IC_2_therm_1(:,2),'.')
plot(S.IC_2_therm_2(:,1),S.IC_2_therm_2(:,2),'.')
plot(S.IC_3_therm_0(:,1),S.IC_3_therm_0(:,2),'.')
plot(S.IC_3_therm_1(:,1),S.IC_3_therm_1(:,2),'.')
plot(S.IC_3_therm_2(:,1),S.IC_3_therm_2(:,2),'.')
ylabel('Temperature (C)')
xlabel('Time (ms)')
title('Accumulator Cell Temperatures: Segment 2 Detailed View')
legend({'Temp1','Temp2','Temp3','Temp4','Temp5','Temp6'},'Location','southeast')
subplot(2,2,3)
hold on
plot(S.IC_4_therm_0(:,1),S.IC_4_therm_0(:,2),'.')
plot(S.IC_4_therm_1(:,1),S.IC_4_therm_1(:,2),'.')
plot(S.IC_4_therm_2(:,1),S.IC_4_therm_2(:,2),'.')
plot(S.IC_5_therm_0(:,1),S.IC_5_therm_0(:,2),'.')
plot(S.IC_5_therm_1(:,1),S.IC_5_therm_1(:,2),'.')
plot(S.IC_5_therm_2(:,1),S.IC_5_therm_2(:,2),'.')
ylabel('Temperature (C)')
xlabel('Time (ms)')
title('Accumulator Cell Temperatures: Segment 3 Detailed View')
legend({'Temp1','Temp2','Temp3','Temp4','Temp5','Temp6'},'Location','southeast')
subplot(2,2,4)
hold on
plot(S.IC_6_therm_0(:,1),S.IC_6_therm_0(:,2),'.')
plot(S.IC_6_therm_1(:,1),S.IC_6_therm_1(:,2),'.')
plot(S.IC_6_therm_2(:,1),S.IC_6_therm_2(:,2),'.')
plot(S.IC_7_therm_0(:,1),S.IC_7_therm_0(:,2),'.')
plot(S.IC_7_therm_1(:,1),S.IC_7_therm_1(:,2),'.')
plot(S.IC_7_therm_2(:,1),S.IC_7_therm_2(:,2),'.')
ylabel('Temperature (C)')
xlabel('Time (ms)')
title('Accumulator Cell Temperatures: Segment 4 Detailed View')
legend({'Temp1','Temp2','Temp3','Temp4','Temp5','Temp6'},'Location','southeast')

%% Accumulator Capacity Analysis
current = S.dc_bus_current; %Amps
motorSpeed = S.motor_speed; %RPM
voltage = S.dc_bus_voltage; %Volts
motorSpeed(:,2) = motorSpeed(:,2)./60; %Rotations per second
consumption = cumtrapz(current(:,1),current(:,2));
consumption = [current(:,1),consumption./3600];
distance = -cumtrapz(motorSpeed(:,1),motorSpeed(:,2)); %Rotations
distance = [motorSpeed(:,1),(distance./4.44)*pi*0.4064./1000]; %Kilometers

% Data uniqueness
for i = 1:length(distance(:,1))
    distance(i,1) = distance(i,1) + i/100000000;
end
for i = 1:length(consumption(:,1))
    consumption(i,1) = consumption(i,1) + i/100000000;
end
for i = 1:length(voltage(:,1))
    voltage(i,1) = voltage(i,1) + i/100000000;
end
for i = 1:length(current(:,1))
    current(i,1) = current(i,1) + i/100000000;
end

time = 1:0.1:max(current(:,1)); %Seconds
adjDistance = interp1(distance(:,1),distance(:,2),time);
adjConsumption = interp1(consumption(:,1),consumption(:,2),time);
adjVoltage = interp1(voltage(:,1),voltage(:,2),time);
adjCurrent = interp1(current(:,1),current(:,2),time);
adjPower = adjVoltage.*adjCurrent; %Watts
adjPower(~isfinite(adjPower)) = 0;
adjEnergy = cumtrapz(time(2:end),adjPower(2:end))./3600; %Watt Hours
adjEnergy = adjEnergy./1000; %kWh
% Plotting
figure
subplot(2,1,1)
plot(adjDistance,adjConsumption)
ylabel('Charge (Ah)')
xlabel('Distance Traveled (km)')
title('Accumulator Capacity Usage vs Distance Traveled (No Slip Assumption)')
subplot(2,1,2)
plot(adjDistance(2:end),adjEnergy)
ylabel('Energy (kWh)')
xlabel('Distance Traveled (km)')
title('Accumulator Energy Expended vs Distance Traveled (No Slip Assumption)')

%% Accumulator Voltage Drop
figure

mask = adjCurrent>10 & adjVoltage > 245;
adjCurrent(~mask) = [];
adjVoltage(~mask) = [];
plot(adjCurrent,adjVoltage,'.')
xlabel('Current')
ylabel('Voltage')
title('Accumulator Voltage Drop Analysis')

