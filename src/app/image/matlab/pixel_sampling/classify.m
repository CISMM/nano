function [cov, cp] = classify(imfile, numpts)

% [cov, cp] = classify(imfile, numpts)
%
%   imfile:   A string containing a filename.
%   numpts:   The number of points to classify.
%   cp:       Has rows [x y intensity classification].
%   cov:      Percent covered by tube.
%
%   Tube -- type 'f' or 't'; stored as 1
%   Substrate -- type 'b', 's', or 'j'; stored as 0

color = [1 0 0];

imrad = 128;
mp = imrad+1;

im = imread(imfile);
cp = [];

rand('state', sum(100*clock));

bigim = zeros(size(im) + [2*imrad 2*imrad]);
image_size = fliplr(size(im));
xdim = image_size(1);
ydim = image_size(2);
bigim(imrad+1:imrad+ydim, imrad+1:imrad+xdim) = im;
image_to_classify = double(im);

iptsetpref('TruesizeWarning','off');
imshow(im, 'truesize');
big_figure_handle = gcf;
figure_handle = figure;

minim = zeros(2*imrad+1, 2*imrad+1);
image(minim);
colormap(gray(256));
truesize(gcf, [4*imrad, 4*imrad]);

for i = 1:numpts
   pixel = unidrnd(image_size);
   x = pixel(1);
   y = pixel(2);
   minim = bigim(y:y+2*imrad, x:x+2*imrad);
   image(minim);
   marker_handles = marker([mp mp], color);
   dot_is_on = 1;
   intensity = image_to_classify(y, x);
   k = 0;
   keypressed = '0';
   while k == 0 | all(keypressed ~= ['f' 't' 'b' 's' 'j'])
      k = waitforbuttonpress;
      keypressed = get(figure_handle, 'CurrentCharacter');
      if keypressed == ' '
         if dot_is_on == 1
   			delete(marker_handles(end));
			   dot_is_on = 0;
			else
			   marker_handles(end) = line([mp mp], [mp mp], 'Color', color);
			   dot_is_on = 1;
         end
      end
   end
   classification = any(keypressed == ['f' 't']);
   cp = [cp; x y intensity classification];
   delete_marker(marker_handles, dot_is_on);
end

delete(figure_handle);
delete(big_figure_handle);

cov = coverage(cp);
