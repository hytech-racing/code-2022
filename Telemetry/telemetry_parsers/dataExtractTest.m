close all
%% 
% Usage:
% 1. Load the data struct into the Workspace
% 2. Run individual sections of the script to plot the desired data

%% IMU
figure

lat_accel = S.lat_accel;
long_accel = S.long_accel;
vert_accel = S.vert_accel;
yaw = S.yaw;
pitch = S.pitch;
roll = S.roll;
hold on
plot(lat_accel(:,1),lat_accel(:,2));
plot(long_accel(:,1),long_accel(:,2));
plot(vert_accel(:,1),vert_accel(:,2));
plot(yaw(:,1),yaw(:,2));
plot(pitch(:,1),pitch(:,2));
plot(roll(:,1),roll(:,2));
legend({'Lateral Acceleration (m/s^2)','Longitudinal Acceleration (m/s^2)','Vertical Acceleration (m/s^2)','Yaw (deg/s)','Pitch (deg/s)', 'Roll (deg/s)'})
xlabel('Time (ms)')
title('IMU Accelerometer and Gyroscope')

%% SAB
figure

amb_air_hum = S.amb_air_hum;
amb_air_temp = S.amb_air_temp;
fl_susp_lin_pot = S.fl_susp_lin_pot;
fr_susp_lin_pot = S.fr_susp_lin_pot;
bl_susp_lin_pot = S.bl_susp_lin_pot;
br_susp_lin_pot = S.br_susp_lin_pot;
hold on
plot(amb_air_hum(:,1),amb_air_hum(:,2));
plot(amb_air_temp(:,1),amb_air_temp(:,2));
plot(fl_susp_lin_pot(:,1),fl_susp_lin_pot(:,2));
plot(fr_susp_lin_pot(:,1),fr_susp_lin_pot(:,2));
plot(bl_susp_lin_pot(:,1),bl_susp_lin_pot(:,2));
plot(br_susp_lin_pot(:,1),br_susp_lin_pot(:,2));
legend({'Ambient Air Humidity (%)','Ambient Air Temperature (C)','Front-Left Suspension Linear Potentiometer (mm)','Front-Right Suspension Linear Potentiometer (mm)','Back-Left Suspension Linear Potentiometer (mm)', 'Back-Right Suspension Linear Potentiometer (mm)'})
xlabel('Time (ms)')
title('Sensor Acquisition Board All Sensors')
