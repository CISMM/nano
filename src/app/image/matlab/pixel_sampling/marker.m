function A = marker(C, color)

x = C(1);
y = C(2);
r = 15;

axes = gca;

xlims = xlim(axes);
xmax = xlims(2);

ylims = ylim(axes);
ymax = ylims(2);

A = zeros(1, 5);

A(1) = line([1 x-r], [y y], 'Color', color);
A(2) = line([x+r xmax], [y y], 'Color', color);
A(3) = line([x x], [1 y-r], 'Color', color);
A(4) = line([x x], [y+r ymax], 'Color', color);

A(5) = line([x x], [y y], 'Color', color);
