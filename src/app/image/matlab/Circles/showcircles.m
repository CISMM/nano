function showcircles(A, im)

% showcircles(A, im)
%
%   Actually shows circles of the appropriate centers and radii as
%   given by A.

iptsetpref('TruesizeWarning','off');
image(im);
colormap(gray(256));
truesize;

for n = 1:size(A, 1)
	circle(A(n, 1:3), [0 0 1]);
end
