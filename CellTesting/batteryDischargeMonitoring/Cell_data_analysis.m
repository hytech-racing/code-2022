%% Import Variables
clear;
%clc;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Update Cell List With New Tests %%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Cell_List = ... %Cell ID, File/Test Number, Channel Number
    {...
    'c19076', 1, 1;...
    'c19077', 1, 2;...
    'c19078', 2, 1;...
    'c19079', 2, 2;...
    'c19082', 3, 1;...
    'c19083', 3, 2;...
    };
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

files = dir('*.txt');
load('Cell_Testing_Data.mat')

if ~exist('CellData','var')
    CellData = struct;
end

[rows, ~] = size(Cell_List);
f = waitbar(0, 'Importing Cell Testing Data...');
for c = 1:2 % IR expressed in ohms
    ID = Cell_List{c,1}; % Cell ID
    i =  Cell_List{c,2}; %file number
    j =  Cell_List{c,3}; %channel number
    waitbar(c/rows, f)
    if ~isfield(CellData,ID)
        tab_import(i).raw = tdfread(files(i).name,'\t');
        CellData.(ID) =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
                          tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR*1000, ...
                          tab_import(i).raw.Contactor, tab_import(i).raw.State];
        CellData.(ID) =  CellData.(ID)(tab_import(i).raw.Channel == j,:);
    end
end
for c = 3:rows % IR expressed in mOhms
    waitbar(c/rows, f)
    ID = Cell_List{c,1}; % Cell ID
    i =  Cell_List{c,2}; %file number
    j =  Cell_List{c,3}; %channel number
    if ~isfield(CellData,ID)
        tab_import(i).raw = tdfread(files(i).name,'\t');
        CellData.(ID) =  [tab_import(i).raw.Time/1000,tab_import(i).raw.Voltage,tab_import(i).raw.Current, ...
                          tab_import(i).raw.OCV_Predict,tab_import(i).raw.Calculated_IR_mOhm, ...
                          tab_import(i).raw.Contactor, tab_import(i).raw.State];
        CellData.(ID) =  CellData.(ID)(tab_import(i).raw.Channel == j,:);
    end    
end
close(f)
clear files i j tab_import ID ans f c
save('Cell_Testing_Data.mat')

%% Coulomb Counting
figure(1);clf
CellSummary.V_Ah_table = zeros(rows,3);
for c = 1:rows
    ID = Cell_List{c,1}; % Cell ID
    
    CellData.(ID)( (CellData.(ID)(:,6) == 0) ,3) = 0; % force zero current when contactor is open
    CellData.(ID)(:,8) = (1/ 3600) .* cumtrapz( CellData.(ID)(:,1) ,  CellData.(ID)(:,3) ); % Amp-Seconds to Amp-Hour Conversion
    
    CellSummary.(ID).StartV = mean(CellData.(ID)(1:100,2)); %average of first 100 measurements
    CellSummary.(ID).Ah = CellData.(ID)(end,8); %Last Cumulative Sum
    
    fprintf(['Cell ',ID(2:end),' Starting Voltage: %0.4f\t',...
             'Ah: %0.4f\n'],CellSummary.(ID).StartV,CellSummary.(ID).Ah)
    
    scatter(CellSummary.(ID).StartV,CellSummary.(ID).Ah,'Filled','DisplayName',ID); hold on; grid on;legend('Location','NorthWest');
    CellSummary.V_Ah_table(c,:) = [str2num(ID(2:end)),CellSummary.(ID).StartV,CellSummary.(ID).Ah];
end
xlabel('Starting Voltage')
ylabel('Capacity (Ah)')

clear c rows ID
save('Cell_Testing_Data.mat')

%% Plotting
figure(2);clf
subplot(1,1,1)
plot(CellData.c19076(:,1),CellData.c19076(:,2)); hold on
plot(CellData.c19077(:,1),CellData.c19077(:,2));
plot(CellData.c19078(:,1),CellData.c19078(:,2));
plot(CellData.c19079(:,1),CellData.c19079(:,2));
plot(CellData.c19082(:,1),CellData.c19082(:,2));
plot(CellData.c19083(:,1),CellData.c19083(:,2));
% subplot(2,1,2)
% plot(CellData.c19079(:,1),CellData.c19079(:,3))
% subplot(3,1,3)
% plot(c19079(:,1),c19079(:,6))