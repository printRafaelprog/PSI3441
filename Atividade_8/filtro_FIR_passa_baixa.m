Fs = 800;      % frequência de amostragem
Fc = 100;       % frequência de corte

N = 10;         % ordem do filtro

b = fir1(N, Fc/(Fs/2), 'low');

disp(b)