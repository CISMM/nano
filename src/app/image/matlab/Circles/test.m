function B = test

B = 1;
incr = 'B = B+1'

for n = 1:10
	yes = uicontrol('Style','pushbutton','Units','normalized',...
   	'Position',[.3 0 .1 .05],'String','Yes');

	no = uicontrol('Style','pushbutton','Units','normalized',...
      'Position',[.6 0 .1 .05],'String','No');
   
   set(yes,'Callback',incr);
end
