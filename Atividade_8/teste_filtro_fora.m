%% Ler arquivo
dados =load('acelerometro.txt');

ax = dados(:,1);
ay = dados(:,2);
az = dados(:,3);

%% Projeto do filtro FIR
Fs = 800;      % frequência de amostragem
Fc = 100;      % frequência de corte
N  = 50;       % ordem

b = fir1(N, Fc/(Fs/2), 'low');

%% Filtragem
ax_f = filtfilt(b,1,ax);
ay_f = filtfilt(b,1,ay);
az_f = filtfilt(b,1,az);

%% Vetor de tempo
t = (0:length(ax)-1)/Fs;

%% Plot X
figure
plot(t, ax, 'b')
hold on
plot(t, ax_f, 'r', 'LineWidth', 2)
grid on
legend('Original','Filtrado')
title('Eixo X')
xlabel('Tempo (s)')
ylabel('Aceleração')

%% Plot Y
figure
plot(t, ay, 'b')
hold on
plot(t, ay_f, 'r', 'LineWidth', 2)
grid on
legend('Original','Filtrado')
title('Eixo Y')

%% Plot Z
figure
plot(t, az, 'b')
hold on
plot(t, az_f, 'r', 'LineWidth', 2)
grid on
legend('Original','Filtrado')
title('Eixo Z')