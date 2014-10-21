function tsucmatlabplot(file, plottitle)

   tsuc2gpData = load(file);
   xsecs = tsuc2gpData(:,3) + 60*(tsuc2gpData(:,2) + 60*tsuc2gpData(:,1));
   xpct = linspace(1, 100, size(xsecs,1));
   xscale = xsecs;
   lw = 2;

   figure(1), clf, hold on, grid on, title(plottitle);
   plot(xscale, tsuc2gpData(:,6), 'g--', 'LineWidth', lw); % current rate
   plot(xscale, tsuc2gpData(:,5), 'r-.', 'LineWidth', lw); % error rate
   plot(xscale, tsuc2gpData(:,10), 'b-', 'LineWidth', lw); % cumul rate
   axisPeak = max([max(tsuc2gpData(:,6)) max(tsuc2gpData(:,5)) max(tsuc2gpData(:,10))]);
   axis([min(xscale) max(xscale)*1.05 0 axisPeak*1.05]);

   % -- Suomeksi
   % legend('hetkellinen siirtonopeus', 'pakettihäviönopeus', 'kumulatiivinen siirtonopeus', 'Location','SouthEast');
   % xlabel('aika [s]'), ylabel('nopeus [Mbit/s]');

   % -- English
   legend('Instant rate', 'Packet loss rate', 'Cumulative rate', 'Location','SouthEast');
   xlabel('Time [s]'), ylabel('Rate [Mbit/s]');
   set(gca, 'FontSize', 16);

   h = get(gca, 'xlabel'); set(h, 'FontSize', 16);
   h = get(gca, 'ylabel'); set(h, 'FontSize', 16);
   h = get(gca, 'title'); set(h, 'FontSize', 16);

