% files = dir('*.xlsx');
% 
% data(1).raw = xlsread(files(1).name,'19076');
% data(2).raw = xlsread(files(1).name,'19077');
% 
% data(3).raw = xlsread(files(2).name,'19078');
% data(4).raw = xlsread(files(2).name,'19079');

%clear;clc;

files = dir('*.txt');

for i = 1:length(files)
 %   tab_import(i).raw = tdfread(files(i).name,'\t');
end

i = 1; j = 1;
c19076 =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
           tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR*1000, ...
           tab_import(i).raw.Contactor, tab_import(i).raw.State];
c19076 =  c19076(tab_import(i).raw.Channel == j,:); 

i = 1; j = 2;
c19077 =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
           tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR*1000, ...
           tab_import(i).raw.Contactor, tab_import(i).raw.State];
c19077 =  c19077(tab_import(i).raw.Channel == j,:); 

i = 2; j = 1;
c19078 =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
           tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR_mOhm, ...
           tab_import(i).raw.Contactor, tab_import(i).raw.State];
c19078 =  c19078(tab_import(i).raw.Channel == j,:); 

i = 2; j = 2;
c19079 =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
           tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR_mOhm, ...
           tab_import(i).raw.Contactor, tab_import(i).raw.State];
c19079 =  c19079(tab_import(i).raw.Channel == j,:);

figure(1)
subplot(2,1,1)
plot(c19079(:,1),c19079(:,2))
subplot(2,1,2)
plot(c19079(:,1),c19079(:,3))
% subplot(3,1,3)
% plot(c19079(:,1),c19079(:,6))