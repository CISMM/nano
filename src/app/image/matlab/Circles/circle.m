function circle(C,color)

nsegs = 25;
for n = 0:2*nsegs-1
   x1 = C(1) + C(3) * cos(pi * n / nsegs);
   x2 = C(1) + C(3) * cos(pi * (n+1) / nsegs);
   y1 = C(2) + C(3) * sin(pi * n / nsegs);
   y2 = C(2) + C(3) * sin(pi * (n+1) / nsegs);
   line([x1 x2], [y1 y2], 'Color', color);
end
